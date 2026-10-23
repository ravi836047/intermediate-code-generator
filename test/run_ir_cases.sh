#!/usr/bin/env sh

set -u

[ "$#" -eq 1 ] || {
    echo "Usage: $0 EXECUTABLE" >&2
    exit 2
}

executable=$1

[ -x "$executable" ] || {
    echo "Error: executable '$executable' not found or executable." >&2
    exit 2
}

temporary_dir=$(mktemp -d)
trap 'rm -rf "$temporary_dir"' EXIT

failures=0

run_case() {
    source_file=$1
    expected_status=$2

    ir_file="$temporary_dir/$(basename "$source_file").ir"
    if "$executable" --ir "$ir_file" "$source_file" >"$temporary_dir/output" 2>"$temporary_dir/error"; then
        actual_status=0
    else
        actual_status=$?
    fi

    if [ "$actual_status" -eq "$expected_status" ] && \
       { [ "$expected_status" -ne 0 ] || [ -s "$ir_file" ]; }; then
        printf 'PASS  %s (exit %s)\n' "$source_file" "$actual_status"
    else
        printf 'FAIL  %s (expected exit %s and generated IR; got exit %s)\n' \
            "$source_file" "$expected_status" "$actual_status" >&2
        sed -n '1,6p' "$temporary_dir/error" >&2
        failures=$((failures + 1))
    fi
}

for source_file in test/ir_cases/[0-9][0-9]_*.c; do
    case "$source_file" in
        *10_invalid_syntax_and_lexing.c)
            run_case "$source_file" 1
            ;;
        *)
            run_case "$source_file" 0
            ;;
    esac
done

if [ "$failures" -ne 0 ]; then
    exit 1
fi

echo "All 12 IR parser/AST cases passed."
