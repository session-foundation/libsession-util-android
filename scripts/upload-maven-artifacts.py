#!/usr/bin/env python3
import dataclasses
import hashlib
import os
import re
import subprocess
import time
import typing
from tempfile import NamedTemporaryFile
from functools import reduce
from pathlib import Path
from xml.etree.ElementTree import Element, ElementTree


@dataclasses.dataclass
class Environment:
    # The SSH key to use to connect to the remote server
    remote_ssh_key: str
    remote_host: str
    # The name of the git repository
    repo: str
    # The path to the maven repo's metadata XML file
    maven_metadata_path: Path


@dataclasses.dataclass
class MavenVersioning:
    latest: str
    release: str
    versions: typing.List[str]
    last_updated: int

    @staticmethod
    def from_xml(el: Element):
        return MavenVersioning(
            latest=el.find('latest').text,
            release=el.find('release').text,
            versions=[v.text for v in el.find('versions')],
            last_updated=int(el.find('lastUpdated').text)
        )

    def to_xml(self) -> str:
        return (f'<versioning>'
                f'<latest>{self.latest}</latest>'
                f'<release>{self.release}</release>'
                f'<versions>'
                f'{"".join(f"<version>{v}</version>" for v in self.versions)}'
                f'</versions>'
                f'<lastUpdated>{self.last_updated}</lastUpdated>'
                f'</versioning>')


@dataclasses.dataclass
class MavenRepositoryMetadata:
    group_id: str
    artifact_id: str
    versioning: MavenVersioning

    @staticmethod
    def from_xml(el: Element) -> 'MavenRepositoryMetadata':
        return MavenRepositoryMetadata(
            group_id=el.find('groupId').text,
            artifact_id=el.find('artifactId').text,
            versioning=MavenVersioning.from_xml(el.find('versioning'))
        )

    @staticmethod
    def parse_from_file(file: Path) -> 'MavenRepositoryMetadata':
        return MavenRepositoryMetadata.from_xml(ElementTree(file=file).getroot())

    def merge(self, other: 'MavenRepositoryMetadata') -> 'MavenRepositoryMetadata':
        if self.group_id != other.group_id or self.artifact_id != other.artifact_id:
            raise ValueError('Cannot merge metadata for different artifacts')

        return MavenRepositoryMetadata(
            group_id=self.group_id,
            artifact_id=self.artifact_id,
            versioning=MavenVersioning(
                latest=max(self.versioning.latest, other.versioning.latest),
                release=max(self.versioning.release, other.versioning.release),
                versions=sorted(set(self.versioning.versions + other.versioning.versions)),
                last_updated=max(self.versioning.last_updated, other.versioning.last_updated)
            )
        )

    def to_xml(self) -> str:
        return (f'<metadata>'
                f'<groupId>{self.group_id}</groupId>'
                f'<artifactId>{self.artifact_id}</artifactId>'
                f'{self.versioning.to_xml()}'
                f'</metadata>')


_MAVEN_REPO_METADATA_HASHES = [
    {
        'ext': 'md5',
        'hasher': hashlib.md5,
    },
    {
        'ext': 'sha1',
        'hasher': hashlib.sha1,
    },
    {
        'ext': 'sha256',
        'hasher': hashlib.sha256,
    },
    {
        'ext': 'sha512',
        'hasher': hashlib.sha512,
    },
]


def create_and_write_temp_file(content: str) -> NamedTemporaryFile:
    tmp_file = NamedTemporaryFile('wt')
    tmp_file.write(content)
    tmp_file.flush()
    return tmp_file


def upload(environment: Environment):
    local_maven_repo_metadata = MavenRepositoryMetadata.parse_from_file(environment.maven_metadata_path)
    print(f'Local maven info: {local_maven_repo_metadata}')

    tmp_files = []

    try:
        # Write the SSH key to a temporary file
        ssh_key_file = create_and_write_temp_file(environment.remote_ssh_key)
        tmp_files.append(ssh_key_file)
        os.chmod(ssh_key_file.name, 0o600)  # Make sure the file is only readable by the owner

        sftp_commands = []

        # Point to the root of this artifact and create the directory
        remote_path = reduce(lambda path, c: path / c,
                             local_maven_repo_metadata.group_id.split('.'),
                             Path(f'oxen.rocks/{environment.repo}/maven')
                             )
        sftp_commands.extend(_mkdirs_sftp_commands(remote_path))

        # Grab the remote maven metadata if it exists
        try:
            # Find the latest version of the repo metadata
            pattern = re.compile(r'^maven-metadata-(\d+)\.xml$')
            remote_metadata_files = [(file, pattern.match(file).group(1)) for file in
                                     _ls_remote_dir(ssh_key_file.name, environment.remote_host, remote_path) if
                                     pattern.match(file)]
            if remote_metadata_files:
                remote_metadata_files.sort(key=lambda x: x[1], reverse=True)
                latest_remote_metadata_file = remote_metadata_files[0][0]
                remote_maven_repo_metadata = _get_remote_maven_metadata(ssh_key_file.name, environment.remote_host,
                                                                        remote_path / latest_remote_metadata_file)
            else:
                remote_maven_repo_metadata = None
        except subprocess.CalledProcessError:
            remote_maven_repo_metadata = None

        print(f'Remote maven info: {remote_maven_repo_metadata}')

        merged_metadata = local_maven_repo_metadata.merge(
            remote_maven_repo_metadata) if remote_maven_repo_metadata else local_maven_repo_metadata
        print(f'Merged maven info: {merged_metadata}')

        # For each version in the local maven, upload them to the remote server
        sftp_commands.append(f'cd "{remote_path}"')
        for version in local_maven_repo_metadata.versioning.versions:
            artifact_dir = environment.maven_metadata_path.parent / version
            sftp_commands.append(f'put -r "{artifact_dir}"')

        # Create a postfixed metadata file and its hash files
        merged_metadata_filename = f'maven-metadata-{int(time.time() * 1000)}.xml'
        merged_contents = merged_metadata.to_xml()
        merged_metadata_file = create_and_write_temp_file(merged_contents)
        tmp_files.append(merged_metadata_file)
        sftp_commands.append(f'put "{merged_metadata_file.name}" {merged_metadata_filename}')

        # Create hashes for the metadata file
        merged_contents_bytes = merged_contents.encode('utf-8')
        for hash_info in _MAVEN_REPO_METADATA_HASHES:
            hash_file = create_and_write_temp_file(hash_info['hasher'](merged_contents_bytes).hexdigest())
            tmp_files.append(hash_file)
            sftp_commands.append(f'put "{hash_file.name}" {merged_metadata_filename}.{hash_info["ext"]}')

        # Now run all the sftp commands
        _run_sftp_commands(ssh_key_file.name, environment.remote_host, sftp_commands)

    finally:
        for tmp_file in tmp_files:
            tmp_file.close()


def _ls_remote_dir(ssh_key_path: str, host: str, remote_dir: Path) -> typing.List[str]:
    with NamedTemporaryFile('wt') as command_file:
        command_file.write(f'cd "{remote_dir}"\n')
        command_file.write(f'ls -1\n')
        command_file.flush()

        try:
            lines = subprocess.check_output(
                ['sftp', '-i', ssh_key_path, '-o', 'StrictHostKeyChecking=off', '-b', command_file.name, host]).splitlines()

            return [line.decode('utf-8') for line in lines[3:]]
        except Exception as ec:
            print(ec)
            return []


def _get_remote_maven_metadata(ssh_key_path: str, host: str, remote_meta_path: Path) -> MavenRepositoryMetadata:
    with NamedTemporaryFile() as xml_file:
        _run_sftp_commands(ssh_key_path, host, [f'get "{remote_meta_path}" "{xml_file.name}"'])
        return MavenRepositoryMetadata.parse_from_file(Path(xml_file.name))


def _run_sftp_commands(ssh_key_path: str, host: str, commands: typing.List[str]):
    with NamedTemporaryFile('wt') as command_file:
        for command in commands:
            command_file.write(command)
            command_file.write('\n')

        command_file.flush()

        subprocess.check_call(
            ['sftp', '-i', ssh_key_path, '-o', 'StrictHostKeyChecking=off', '-b', command_file.name, host])


def _require_env_var(name: str) -> str:
    value = os.environ.get(name)
    if value is None:
        raise ValueError(f"Environment variable {name} is required but not set")
    return value


# Generate sftp commands to create a directory (and all its parents)
def _mkdirs_sftp_commands(path: Path) -> typing.List[str]:
    commands = [f'-mkdir {p}' for p in path.parents if str(p) not in ('.', '/')]
    commands.reverse()
    commands.append(f'-mkdir {path}')
    return commands


if __name__ == '__main__':
    upload(
        environment=Environment(
            remote_ssh_key=_require_env_var('SSH_KEY'),
            remote_host='drone@oxen.rocks',
            repo=_require_env_var('DRONE_REPO').replace(' ', '_'),
            maven_metadata_path=Path(__file__).parent / 'library/build/repo/org/sessionfoundation/libsession-util-android/maven-metadata.xml'
        )
    )
