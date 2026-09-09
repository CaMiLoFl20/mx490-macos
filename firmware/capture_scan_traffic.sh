#!/bin/sh
set -eu

printer_host="${1:-B8389B000000.local.}"
capture_file="${2:-/tmp/mx490-scan.pcap}"

printer_ip="$(/usr/bin/dscacheutil -q host -a name "$printer_host" | /usr/bin/awk '/^ip_address: / && $2 !~ /:/ {print $2; exit}')"
if [ -z "$printer_ip" ]; then
    echo "Could not resolve $printer_host to an IPv4 address." >&2
    exit 1
fi

echo "Capturing unicast traffic between this Mac and $printer_host."
echo "Start one small platen scan, then press Ctrl-C here. Output: $capture_file"

sudo /usr/sbin/tcpdump -i en0 -nn -s 0 -w "$capture_file" \
    "host $printer_ip and not multicast and not igmp"

sudo /usr/sbin/chown "$(/usr/bin/id -u):$(/usr/bin/id -g)" "$capture_file"
echo "Capture complete: $capture_file"
