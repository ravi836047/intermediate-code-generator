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
EXPECTED_FAIL_COUNT=0

cat <<'WARNING'
WARNING: Blessing overwrites golden master IR files. Verify that your compiler
is currently producing correct IR before running this script.
WARNING

while IFS= read -r -d '' SOURCE_FILE; do
    EXPECTED_IR="${SOURCE_FILE}.expected.ir"
    rm -f -- "$EXPECTED_IR"
    if "$EXECUTABLE" --ir "$EXPECTED_IR" "$SOURCE_FILE"; then
        if [[ -f $EXPECTED_IR ]]; then
            printf 'BLESSED %s -> %s\n' "$SOURCE_FILE" "$EXPECTED_IR"
            ((PASS_COUNT += 1))
        else
            printf 'FAIL %s (compiler succeeded but did not create IR output)\n' "$SOURCE_FILE"
            ((FAIL_COUNT += 1))
        fi
    else
        STATUS=$?
        printf 'Failed (Expected): %s (compiler status %d)\n' "$SOURCE_FILE" "$STATUS"
        rm -f -- "$EXPECTED_IR"
        ((EXPECTED_FAIL_COUNT += 1))
    fi
done < <(find "$SCRIPT_DIR" -type f -name '*.c' -print0 | sort -z)

printf '\nBlessing summary: %d snapshots written, %d expected failures\n' \
    "$PASS_COUNT" "$EXPECTED_FAIL_COUNT"
