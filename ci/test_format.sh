#!/bin/bash
echo "clang-format - checking Code Formatting..."

if [[ "${TRAVIS_OS_NAME}" == "linux" ]] && \
   [[ "${TEST_CLANG_FORMAT}" == "yes" ]]; then

    RETURN=0
    CLANG_FORMAT="clang-format-3.9"

    which clang-format-3.9

    if [ ! -f ".clang-format" ]; then
        echo ".clang-format file not found!"
        exit 1
    fi

    FILES=`git diff master --name-only | grep -E "\.(cc|cpp|h)$"`

    for FILE in $FILES; do

        $CLANG_FORMAT $FILE | cmp  $FILE >/dev/null

        if [ $? -ne 0 ]; then
            echo "[!] Clang-Format Found INCORRECT FORMATTING. Please re-format and re-submit.  The following file failed: $FILE" >&2
            RETURN=1
        fi

    done

    exit $RETURN
fi

exit 0
