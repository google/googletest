#!/bin/bash
# Based on https://raw.githubusercontent.com/dev-id/Magic-Spoiler/8111a06ab6682e020169991d5e2aa4fa503d787f/preflight.sh
set -e

CHANGED_FILES=`git diff --name-only master...${TRAVIS_COMMIT}`
ONLY_READMES=True
MD=".md"

for CHANGED_FILE in $CHANGED_FILES; do
  if ! [[ $CHANGED_FILE =~ $MD ]]; then
    ONLY_READMES=False
    break
  fi
done

if [[ $ONLY_READMES == True ]]; then
  echo "Only .md files found, exiting."
  exit 0
