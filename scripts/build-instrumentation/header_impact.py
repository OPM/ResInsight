"""Rank headers by how much compile time a change to them triggers.

Usage:
  python header_impact.py <build-dir> <compiles.csv> [--top N] [--filter REGEX] [--csv OUT]

Uses the Ninja dependency log ('ninja -t deps', recorded from /showIncludes or depfiles) to find the TUs that
include each header, and compiles.csv from build_report.py (a clean build) for the compile time of each TU.

  rebuild_s  sum of compile times of all TUs including the header, i.e. the cost of touching it
  tus        number of TUs including the header
"""

import argparse
import collections
import csv
import json
import os
import re
import subprocess
from pathlib import Path


def norm(path):
    return os.path.normcase(os.path.normpath(path))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("build_dir")
    parser.add_argument("compiles_csv")
    parser.add_argument("--top", type=int, default=40)
    parser.add_argument("--filter", default="", help="Only report headers whose path matches this regex")
    parser.add_argument("--ninja", default="ninja")
    parser.add_argument("--csv", help="Write the full ranking to this file")
    args = parser.parse_args()

    build_dir = Path(args.build_dir).resolve()
    source_dir = Path(__file__).resolve().parents[2]

    # object file -> source file
    obj_to_source = {}
    for entry in json.load(open(build_dir / "compile_commands.json")):
        output = entry.get("output")
        if output:
            obj = output if os.path.isabs(output) else os.path.join(entry["directory"], output)
            obj_to_source[norm(obj)] = norm(entry["file"])

    # source file -> compile seconds
    source_seconds = {}
    with open(args.compiles_csv) as f:
        for row in csv.DictReader(f):
            source = row["source"]
            if not os.path.isabs(source):
                source = source_dir / source
            source_seconds[norm(str(source))] = float(row["seconds"])

    deps = subprocess.run([args.ninja, "-C", str(build_dir), "-t", "deps"], capture_output=True, text=True,
                          check=True).stdout

    header_tus = collections.Counter()
    header_seconds = collections.Counter()
    total_seconds = 0.0
    seconds = None
    for line in deps.splitlines():
        if not line.startswith(" "):
            match = re.match(r"(.+?): #deps", line)
            seconds = None
            if match:
                obj = match.group(1)
                obj = obj if os.path.isabs(obj) else str(build_dir / obj)
                seconds = source_seconds.get(obj_to_source.get(norm(obj), ""))
                if seconds is not None:
                    total_seconds += seconds
        elif seconds is not None:
            header = norm(line.strip())
            header_tus[header] += 1
            header_seconds[header] += seconds

    pattern = re.compile(args.filter, re.I) if args.filter else None
    ranked = [h for h, _ in header_seconds.most_common() if not pattern or pattern.search(h)]

    def display(path):
        try:
            return Path(path).relative_to(norm(str(source_dir))).as_posix()
        except ValueError:
            return Path(path).as_posix()

    if args.csv:
        with open(args.csv, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["rebuild_s", "tus", "header"])
            for h in ranked:
                writer.writerow([f"{header_seconds[h]:.1f}", header_tus[h], display(h)])

    print(f"Total compile time of matched TUs: {total_seconds:.0f} s")
    print("| rebuild s | % of total | TUs | header |")
    print("|---:|---:|---:|---|")
    for h in ranked[: args.top]:
        print(f"| {header_seconds[h]:.0f} | {100 * header_seconds[h] / total_seconds:.0f}% | {header_tus[h]} | "
              f"{display(h)} |")


if __name__ == "__main__":
    main()
