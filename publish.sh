#!/bin/bash
#
#
# This is NOT a good way to do things. It's just a place holder.
#


# Adding updated package to our aptly repo
REPO=$1
DISTRI=$2

BUILD_DIR=build
SNAPSHOTID=`date +%Y%m%d-%H%M`
PASSPHRASE=waldorf-fantastic-optimization-caviar

if [ `aptly repo list | grep ${REPO} | wc -l` == 0 ]; then
    aptly repo create ${REPO}
else
    aptly repo remove ${REPO} googletest
fi

echo "Build dir is:"
ls -la ${BUILD_DIR}
echo "HERE is:"
pwd
ls -la .
echo "Aptly Publishish:"
for x in echo ' '; do
    echo "STAGE $x"
    
    $x aptly -config=./aptly-conf repo add $REPO $BUILD_DIR/*.deb
    $x aptly -config=./aptly-conf snapshot create $REPO-$SNAPSHOTID from repo $REPO
    $x aptly -config=./aptly-conf publish drop $DISTRI s3:$REPO:
    $x aptly -config=./aptly-conf publish snapshot -force-overwrite -passphrase=${PASSPHRASE} -batch=true --distribution=$DISTRI $REPO-$SNAPSHOTID s3:${REPO}:
done
