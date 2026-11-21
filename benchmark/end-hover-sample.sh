#!/usr/bin/env bash

root="$(cd "$(dirname "$0")/.." && pwd)"

cd "$root" && perl -e 'alarm 240; exec @ARGV' nvim --headless -l test/hover.lua test/config/end-hover.json > "$(dirname "$1")/end-hover-sample.stdout" 2>&1
echo "exit $?" >> "$(dirname "$1")/end-hover-sample.stdout"

: > "$(dirname "$1")/end-hover-sample-samples.md"
exit 0
