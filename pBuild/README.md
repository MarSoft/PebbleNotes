# pBuild for Travis-CI
PebbleBuild is a tool for building pebble projects in Travis CI.
It is really useful when using the Pebble SDK.

## Setup
1. To install pBuild, copy .travis.yml, before_install.sh, and build.sh to your pebble app's github repository.  Make sure the two .sh files are marked executable, using `git update-index --chmod=+x` if necessary.
2. Then go to travis, go into your repoditory (in Travis) and go to Settings -> Settings and into Enviromental Variables. 
3. Add new enviromental variables: `PEBBLE_SDK=pebble-sdk-4.1.1-linux64` (this is the pebble sdk version to use for compilation) and check "Display value in build logs", and `PEBBLE_PROJECT_PATH=[RepoOwnerUsername]/[RepoName]/[PebbleProjectPathinRepo]` (Only add the PebbleProjectPathinRepo if needed). Example: `PEBBLE_PROJECT_PATH=sGerli/pBuild` (this one is case sensitive) and check "Display value in build logs"

## Settings
* Pebble Analytics are on for default. If you want to disable it in before_install.sh modify the 9th line and change `touch ~/.pebble-sdk/ENABLE_ANALYTICS` to `touch ~/.pebble-sdk/NO_TRACKING`

## Troubleshooting
1. If a resource is not found, check its path because it's case sensitive.
2. If you get any other build script error add an issue.
3. If you get an abnormal error, send a email to support@igerli.com.

## Contributors
* Stefano Gerli (@sGerli)
* Ben Combee (@unwiredben)
* Josh Austin (@nonproftechie)

## Credits
pBuild is based on [Sawyer Pangborn's travis-ci-pebble](https://github.com/spangborn/travis-ci-pebble)
