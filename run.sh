#!/usr/bin/env sh
set -u

[ "$#" -eq 1 ] || {
    echo "Usage: $0 EXECUTABLE" >&2
    exit 2
}

executable=$1

[ -x "$executable" ] || {
    echo "Error: executable '$executable' not found or not executable." >&2
    exit 1
}

output_dir=test/ir_output
mkdir -p "$output_dir"
failures=0
case_list=/tmp/sa-cases.$$
find test -type f -name '*.c' | sort >"$case_list"

run_case() {
    f=$1
    printf '\n=== %s ===\n' "$f"

    ir_file="$output_dir/$(printf '%s' "$f" | tr '/' '_').ir"

    expected_failure=false
    case "$f" in
        test/negative.c|*error_*|*invalid*) expected_failure=true ;;
    esac
    if "$executable" --ir "$ir_file" "$f" >/tmp/sa-out.$$ 2>/tmp/sa-err.$$; then
        echo "Exit status: 0"
        if [ "$expected_failure" = true ]; then
            echo "FAIL: this case was expected to be rejected." >&2
            failures=$((failures + 1))
        fi
    else
        status=$?
        echo "Exit status: $status"
        if [ "$expected_failure" = false ]; then
            echo "FAIL: IR generation failed for a valid test case." >&2
            failures=$((failures + 1))
        fi
    fi

    if [ -s /tmp/sa-out.$$ ]; then
        cat /tmp/sa-out.$$
    fi

    if [ -s /tmp/sa-err.$$ ]; then
        cat /tmp/sa-err.$$ >&2
    fi
}

while IFS= read -r f; do
    run_case "$f"
done <"$case_list"

rm -f /tmp/sa-out.$$ /tmp/sa-err.$$ "$case_list"

if [ "$failures" -ne 0 ]; then
    exit 1
fi
