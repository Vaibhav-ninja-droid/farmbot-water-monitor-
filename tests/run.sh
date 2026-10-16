#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
output_dir=$(mktemp -d)
trap 'rm -rf "$output_dir"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I arduino/FarmbotWaterMonitor/src arduino/FarmbotWaterMonitor/src/monitor.c tests/test_monitor.c -o "$output_dir/test_monitor"
"$output_dir/test_monitor"
${CC:-cc} -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I arduino/FarmbotWaterMonitor/src arduino/FarmbotWaterMonitor/src/monitor.c arduino/FarmbotWaterMonitor/src/hrxl_serial.c tests/test_hrxl_serial.c -o "$output_dir/test_serial"
"$output_dir/test_serial"
python3 -m unittest discover -s tests -p 'test_*.py'
