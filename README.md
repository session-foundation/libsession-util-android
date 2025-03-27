### libsession-util Android bridging code

This repository contains the bridging code allowing the Java/Kotlin client to access to the [libsession-util](https://github.com/session-foundation/libsession-util/) library.

#### Quick start

To start using this library, add this into your top level `build.gradle`'s repository settings

```gradle
maven {
    url uri("https://oxen.rocks/session-foundation/libsession-util-android/maven")
    content {
        includeGroup('org.sessionfoundation')
    }
}
```

Then in your app's `build.gradle', you can add this as dependency:

```gradle
implementation 'org.sessionfoundation:libsession-util-android:LATEST_VERSION_NUMBER'
```

All tags pushed to this repository will be built and you can find out the latest by visiting https://github.com/session-foundation/libsession-util-android/tags

#### Versioning

This library's versioning rules are:
1. If there is a tag related to this commit, the tag will be used as the version name
2. Otherwise, the version name will look like "lastGitTag-numberOfChangesSinceLastTag-shortCommitHash", for example "1.0.0-5-abcd12345". "-dirty" will be appended if the git workspace contains uncommitted changes.

Normally you don't have to worry about versioning as it's fully automatic and determinstic. If changes warrant a new release, then Session devs should push a tag and we'll have a formal release version built.

#### Development & deployment

Everyone's welcome to open a Pull Request. However, in order to push any build artifact to `oxen.rocks`, the changes have to come from either the merge of a PR, or the PRs to be opened directly in this repo, as per permission setup of our CI.
