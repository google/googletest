#!/bin/bash
# Publishes packages to server.
# TODO(frankie): This is NOT a good way to do things. It's just a place holder.
#
# Copyright: 2017 Ditto Technologies. All Rights Reserved.
# Author: Frankie Li, Daran He


# Adding updated package to our s3 repo.
REPO=$1
DISTRI=$2

KEY_ID=`gpg --list-keys | grep "pub " | grep -oP "(?<=\/)[A-Z0-9]+(?=\s)"`
BUILD_DIR=build

# Show folder content.
echo "Build dir is:"
ls -lLa ${BUILD_DIR}
echo "HERE is:"
pwd
ls -la

# Publish.
echo "Publishing:"
for x in echo ' '; do
    echo "STAGE $x"

    $x deb-s3 upload \
        --sign $KEY_ID \
        --gpg-options "\-\-batch \-\-passphrase waldorf\-fantastic\-optimization\-caviar" \
        --codename $DISTRI --bucket $REPO --lock \
        $BUILD_DIR/*.deb
done
