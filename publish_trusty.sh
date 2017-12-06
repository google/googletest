#!/bin/bash
#
#
# This is NOT a good way to do things. It's just a place holder.
#


# Adding updated package to our aptly repo
SNAPSHOTID=`date +%Y%m%d-%H%M`
REPO='3rdparty-14.04'

if [ `aptly repo list | grep 3rdparty | wc -l` == 0 ]; then
    aptly repo create 3rdparty-14.04
else
    aptly repo remove 3rdparty-14.04 googletest
fi

BUILD_DIR=build

echo "Build dir is:"
ls -la ${BUILD_DIR}
echo "HERE is:"
pwd
ls -la .
echo "Aptly Publishish:"
for x in echo ' '; do
	echo "STAGE $x"
	$x aptly -config=./aptly-conf repo add ${REPO} ${BUILD_DIR}/*.deb
	$x aptly -config=./aptly-conf snapshot create $REPO-$SNAPSHOTID from repo $REPO
	$x aptly -config=./aptly-conf publish drop trusty s3:${REPO}:
	$x aptly -config=./aptly-conf publish snapshot -force-overwrite -passphrase=waldorf-fantastic-optimization-caviar -batch=true --distribution=trusty $REPO-$SNAPSHOTID s3:${REPO}:
done
