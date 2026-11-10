#!/usr/bin/env python3
"""Capture firmware CSV from a serial port (pyserial) or saved console text."""

import argparse
import csv
from datetime import datetime, timezone
from pathlib import Path

FIELDS = "uptime_ms,sequence,sample_event,pulse_us,raw_mm,filtered_mm,depth_mm,depth_percent,sample_age_ms,validity,permission,reason,capture_overruns".split(
    ","
)


def parse_row(line):
    values = next(csv.reader([line.strip()]))
    if len(values) != len(FIELDS) or not values[0].isdigit():
        return None
    row = dict(zip(FIELDS, values))
    for name in FIELDS:
        if name not in ("validity", "reason"):
            int(row[name])
    if row["permission"] not in ("0", "1") or row["sample_event"] not in ("0", "1"):
        raise ValueError("invalid Boolean field")
    return row


def main():
    p = argparse.ArgumentParser(description=__doc__)
    source = p.add_mutually_exclusive_group(required=True)
    source.add_argument("--port")
    source.add_argument("--input", type=Path)
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--output", required=True, type=Path)
    args = p.parse_args()
    if args.port:
        try:
            import serial
        except ImportError:
            p.error("serial capture requires pyserial: python3 -m pip install pyserial")
        stream = serial.Serial(args.port, args.baud, timeout=1)
    else:
        stream = args.input.open("rb")
    # Exclusive creation prevents accidental overwrite of experiment evidence.
    with stream, args.output.open("x", newline="") as out, Path(
        str(args.output) + ".console.txt"
    ).open("x") as raw:
        writer = csv.DictWriter(
            out, fieldnames=["host_receive_time_utc", "session"] + FIELDS
        )
        writer.writeheader()
        session = 0
        try:
            while True:
                data = stream.readline()
                if not data:
                    if args.input:
                        break
                    continue
                line = data.decode("utf-8", errors="replace")
                raw.write(line)
                raw.flush()
                if line.startswith("# water-monitor"):
                    session += 1
                try:
                    row = parse_row(line)
                except ValueError:
                    continue
                if row:
                    writer.writerow(
                        dict(
                            host_receive_time_utc=datetime.now(
                                timezone.utc
                            ).isoformat(),
                            session=session,
                            **row
                        )
                    )
                    out.flush()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
