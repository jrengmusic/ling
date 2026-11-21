#!/usr/bin/env bash

root="$(cd "$(dirname "$0")/.." && pwd)"

"$root/Builds/Schema/ling_artefacts/Release/ling" "$1" | diff "$root/test/baseline/stdio-client-macos.txt" -
exitCode=$?

: > "$(dirname "$1")/stdio-client-samples.md"
exit $exitCode
