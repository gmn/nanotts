#!/bin/bash
git log -1 | head -1 | sed -e 's/commit /\#define GIT_TIP_VERSION "/g' -e 's/$/"\r/g' > git_version.h
git log -1 | grep Date | sed -E -e 's/Date:[ ]*/\#define GIT_TIP_DATE "/g' -e 's/$/"\r/g' >> git_version.h
if git status | grep 'not staged' >/dev/null; then echo '* With unknown local changes'; else echo 'Caught up' ; fi | sed -e 's/^/\#define GIT_TIP_MSG "/g' -e 's/$/"\r/g' >> git_version.h

NUM=`cat release_version`
if grep -q 'With unknown' git_version.h ; then
    let NUM="$NUM+1"
    echo $NUM > release_version
fi
echo '#define RELEASE_VERSION "'$NUM'"' > release_version.h

