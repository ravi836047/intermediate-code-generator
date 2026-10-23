#!/usr/bin/env bash

set -u

if [[ $# -lt 1 || -z ${1:-} ]]; then
    echo "Usage: $0 <compiler-executable>" >&2
    exit 2
fi

EXECUTABLE=$1
if [[ ! -x $EXECUTABLE ]]; then
    echo "Error: compiler is not executable: $EXECUTABLE" >&2
    exit 2
fi

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
PASS_COUNT=0
FAIL_COUNT=0
TEMP_DIR=$(mktemp -d)
trap 'rm -rf -- "$TEMP_DIR"' EXIT

while IFS= read -r -d '' SOURCE_FILE; do
    EXPECTED_IR="${SOURCE_FILE}.expected.ir"
    ACTUAL_IR="$TEMP_DIR/actual.ir"
    if [[ -f $EXPECTED_IR ]]; then
        EXPECTED_STATUS=0
    else
        EXPECTED_STATUS=1
    fi

    rm -f -- "$ACTUAL_IR"
    if "$EXECUTABLE" --ir "$ACTUAL_IR" "$SOURCE_FILE"; then
        STATUS=0
    else
        STATUS=$?
    fi

    if (( (EXPECTED_STATUS == 0) != (STATUS == 0) )); then
        printf 'FAIL %s (expected status %d, compiler status %d)\n' \
            "$SOURCE_FILE" "$EXPECTED_STATUS" "$STATUS"
        ((FAIL_COUNT += 1))
        continue
    fi

    if (( EXPECTED_STATUS != 0 )); then
        printf 'PASS %s (failed as expected; no snapshot)\n' "$SOURCE_FILE"
        ((PASS_COUNT += 1))
        continue
    fi

    if [[ ! -f $EXPECTED_IR ]]; then
        printf 'FAIL %s (missing golden master: %s)\n' "$SOURCE_FILE" "$EXPECTED_IR"
        ((FAIL_COUNT += 1))
        continue
    fi

    if [[ ! -f $ACTUAL_IR ]]; then
        printf 'FAIL %s (compiler succeeded but did not create IR output)\n' "$SOURCE_FILE"
        ((FAIL_COUNT += 1))
        continue
    fi

    if diff -u -- "$EXPECTED_IR" "$ACTUAL_IR"; then
        printf 'PASS %s\n' "$SOURCE_FILE"
        ((PASS_COUNT += 1))
    else
        printf 'FAIL %s (IR differs from %s)\n' "$SOURCE_FILE" "$EXPECTED_IR"
        ((FAIL_COUNT += 1))
    fi
done < <(find "$SCRIPT_DIR" -type f \( -name '*.c' -o -name '*.cpp.c' \) -print0 | sort -z)

printf '\nTest summary: %d passed, %d failed\n' "$PASS_COUNT" "$FAIL_COUNT"
(( FAIL_COUNT == 0 ))
