#!/usr/bin/env sh
# Regression checks for semantic IR properties that are not covered by merely
# checking whether a source file parses and produces a non-empty .ir file.
set -eu

[ "$#" -eq 1 ] || { echo "Usage: $0 EXECUTABLE" >&2; exit 2; }
executable=$1
[ -x "$executable" ] || { echo "Error: executable '$executable' not found or not executable." >&2; exit 2; }

temporary_dir=$(mktemp -d)
trap 'rm -rf "$temporary_dir"' EXIT

generate() {
    source_file=$1
    output_file="$temporary_dir/$(basename "$source_file").ir"
    "$executable" --ir "$output_file" "$source_file" >/dev/null
    printf '%s\n' "$output_file"
}

require() {
    pattern=$1
    file=$2
    description=$3
    if ! grep -Fq -- "$pattern" "$file"; then
        echo "FAIL: $description (missing '$pattern')" >&2
        exit 1
    fi
}

loops=$(generate test/loops.c)
require 's = 0' "$loops" 'for-loop initializer'
require 'i = 0' "$loops" 'comma-expression loop initializer'

arrays=$(generate test/compiler_stress_tests/07_arrays_multidimensional.c)
if grep -Fq '<InitializerList>' "$arrays"; then
    echo 'FAIL: nested initializer was emitted as a placeholder' >&2
    exit 1
fi
require '[0][0]' "$arrays" 'multidimensional array initialization'

references=$(generate test/compiler_stress_tests/13_references_new_delete.cpp.c)
require 'alias ' "$references" 'reference alias lowering'
require 'int*' "$references" 'pointer type preservation'

overloads=$(generate test/ir_cases/12_overloading_multilevel_pointers.c)
require 'func blend$1$int:' "$overloads" 'single-argument overload symbol'
require 'func blend$2$int$int:' "$overloads" 'two-argument overload symbol'
require 'call blend$1$int, 1' "$overloads" 'single-argument overload call'
require 'call blend$2$int$int, 2' "$overloads" 'two-argument overload call'

classes=$(generate test/compiler_stress_tests/11_classes_access_this_methods.cpp.c)
require 'param this : ' "$classes" 'explicit object receiver'
require '::' "$classes" 'qualified class member symbol'

enums=$(generate test/compiler_stress_tests/10_enum_typedef_static_sizeof.c)
require 'const ' "$enums" 'enum constant lowering'
require 'static ' "$enums" 'static storage annotation'

if "$executable" --ir "$temporary_dir/ambiguous.ir" \
    test/compiler_stress_tests/34_error_overload_ambiguous.cpp.c >/dev/null 2>&1; then
    echo 'FAIL: ambiguous overload was accepted' >&2
    exit 1
fi

echo 'IR semantic regression checks passed.'
