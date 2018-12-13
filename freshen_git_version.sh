#!/bin/bash
git log -1 | head -1 | sed -e 's/commit /\#define GIT_TIP_VERSION "/g' -e 's/$/"\r/g' > git_version.h
git log -1 | grep Date | sed -E -e 's/Date:[ ]*/\#define GIT_TIP_DATE "/g' -e 's/$/"\r/g' >> git_version.h
if git status | grep 'not staged' >/dev/null; then echo '* With unknown local changes'; else echo 'Caught up' ; fi | sed -e 's/^/\#define GIT_TIP_MSG "/g' -e 's/$/"\r/g' >> git_version.h

NUM=`cat release_version | awk '{print $1}'`
VERSION=`git log -1 | head -1 | sed -e 's/commit //g'`
OLD_VERSION=`cat release_version | awk '{print $2}'`

if [ "$VERSION" != "$OLD_VERSION" ]; then
    # release version increments every commit
    let NUM="$NUM+1"
    echo "$NUM $VERSION" > release_version
fi
echo '#define RELEASE_VERSION "'$NUM'"' > release_version.h

