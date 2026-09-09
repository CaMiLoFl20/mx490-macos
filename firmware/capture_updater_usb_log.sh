#!/bin/zsh
set -euo pipefail

out=${1:-/tmp/mx490-updater-usb.log}
echo "Recording macOS USB and Canon updater logs to $out"
echo "Launch the Canon updater and stop after it displays the current version."
echo "Press Ctrl-C here before clicking Start."
exec log stream --style compact --level debug \
  --predicate '(subsystem CONTAINS[c] "USB" OR process CONTAINS[c] "Printer Update" OR process CONTAINS[c] "CIJ")' \
  > "$out"
