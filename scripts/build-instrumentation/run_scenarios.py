"""Run the build time measurement scenarios on an instrumented build tree.

Usage:
  python run_scenarios.py <build-dir> [--cmake <cmake>] [--runs N] [--jobs N] [--scenarios ABCDE] [--session NAME]

Each build is labelled <session>-<scenario>-<run> through RI_BUILD_LABEL, so build_report.py writes one report per
build. The median wall time per scenario is printed at the end. Run 'build_report.py install' first.
"""

import argparse
import csv
import datetime
import os
import statistics
import subprocess
import sys
from pathlib import Path

SOURCE_DIR = Path(__file__).resolve().parents[2]

# Files are touched (mtime only) to trigger an incremental build
SCENARIOS = {
    "A": ("Clean build", None),
    "B": ("Leaf .cpp", "ApplicationLibCode/ProjectDataModel/RimProject.cpp"),
    "C": ("Rim header", "ApplicationLibCode/ProjectDataModel/RimProject.h"),
    "D": ("Framework header", "Fwk/AppFwk/cafProjectDataModel/cafPdmCore/cafPdmObjectHandle.h"),
    "E": ("No-op build", None),
}


def build(args, label):
    env = dict(os.environ, RI_BUILD_LABEL=label)
    cmd = [args.cmake, "--build", args.build_dir, "-j", str(args.jobs)]
    print(f"[{datetime.datetime.now():%H:%M:%S}] {label}: {' '.join(cmd)}", flush=True)
    result = subprocess.run(cmd, env=env, stdout=subprocess.DEVNULL if args.quiet else None)
    if result.returncode != 0:
        sys.exit(f"Build failed for {label}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("build_dir")
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--runs", type=int, default=3)
    parser.add_argument("--jobs", type=int, default=30)
    parser.add_argument("--scenarios", default="ABCDE")
    parser.add_argument("--session", default=datetime.datetime.now().strftime("s%Y%m%d%H%M%S"))
    parser.add_argument("--quiet", action="store_true", help="Hide build output")
    args = parser.parse_args()

    history = Path(args.build_dir) / "instrumentation-reports" / "history.csv"

    def read_history():
        if not history.exists():
            return []
        with open(history, newline="") as f:
            return list(csv.DictReader(f))

    # Only aggregate rows appended by this invocation, the session name may have been used before
    history_start = len(read_history())

    # Bring the tree up to date so incremental scenarios start from a clean state
    build(args, f"{args.session}-prep")

    for scenario in args.scenarios:
        _, touch = SCENARIOS[scenario]
        for run in range(1, args.runs + 1):
            if scenario == "A":
                subprocess.run([args.cmake, "--build", args.build_dir, "--target", "clean"], check=True,
                               stdout=subprocess.DEVNULL)
            if touch:
                (SOURCE_DIR / touch).touch()
            build(args, f"{args.session}-{scenario}-{run}")

    rows = [r for r in read_history()[history_start:] if r["label"].startswith(args.session + "-")]
    print(f"\nSession {args.session}, median of {args.runs} runs, -j{args.jobs}")
    print("| Scenario | wall s | compile s | link s | custom s | steps | parallelism |")
    print("|---|---:|---:|---:|---:|---:|---:|")
    for scenario in args.scenarios:
        runs = [r for r in rows if r["label"].rsplit("-", 2)[1] == scenario]
        if not runs:
            print(f"| {scenario}. {SCENARIOS[scenario][0]} | no report | | | | | |")
            continue
        med = lambda key: statistics.median(float(r[key]) for r in runs)
        steps = statistics.median(int(r["compile_count"]) + int(r["link_count"]) + int(r["custom_count"]) for r in runs)
        print(f"| {scenario}. {SCENARIOS[scenario][0]} | {med('wall_s'):.1f} | {med('compile_s'):.1f} | "
              f"{med('link_s'):.1f} | {med('custom_s'):.1f} | {steps:.0f} | {med('parallelism'):.1f} |")


if __name__ == "__main__":
    main()
