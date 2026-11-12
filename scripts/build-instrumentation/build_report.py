"""Build time report based on the CMake instrumentation API (CMake 4.5+).

Usage:
  python build_report.py install <build-dir>   Write the instrumentation query for <build-dir>
  python build_report.py <index-file>          Callback invoked by CMake after each 'cmake --build'

Configuring with -DRESINSIGHT_ENABLE_BUILD_INSTRUMENTATION=ON registers the same callback, without 'install'.

Set RI_BUILD_LABEL to tag a run (e.g. the measurement scenario). Reports are written to
<build-dir>/instrumentation-reports/<timestamp>[-<label>]/ and one summary row per run is appended to
<build-dir>/instrumentation-reports/history.csv.

https://cmake.org/cmake/help/latest/manual/cmake-instrumentation.7.html
"""

import collections
import csv
import datetime
import json
import os
import re
import shutil
import sys
from pathlib import Path

QUERY_OPTIONS = [
    "staticSystemInformation",
    "dynamicSystemInformation",
    "processMetrics",
    "trace",
]
TOP_COUNT = 50


def install(build_dir):
    query_dir = Path(build_dir) / ".cmake" / "instrumentation" / "v1" / "query"
    query_dir.mkdir(parents=True, exist_ok=True)
    script = Path(__file__).resolve().as_posix()
    python = Path(sys.executable).resolve().as_posix()
    query = {
        "version": {"major": 1, "minor": 2},
        "callbacks": [" ".join(f'"{p}"' if " " in p else p for p in (python, script))],
        "hooks": ["postCMakeBuild"],
        "options": QUERY_OPTIONS,
    }
    query_file = query_dir / "build_report.json"
    query_file.write_text(json.dumps(query, indent=2) + "\n")
    print(f"Wrote {query_file}. Re-run configure to activate instrumentation.")


def custom_kind(snippet):
    command = snippet.get("command", "")
    if "cmake_autogen" in command:
        return "moc/uic"
    if "cmake_autorcc" in command:
        return "rcc"
    if re.search(r"protoc(\.exe)?\b", command):
        return "protoc"
    return "other"


def custom_name(snippet):
    if snippet.get("target"):
        return snippet["target"]
    match = re.search(r"([\w.-]+)_autogen", snippet.get("command", ""))
    if match:
        return match.group(1)
    outputs = snippet.get("outputs") or []
    return Path(outputs[0]).name if outputs else snippet.get("command", "")[:80]


def metrics(snippet):
    pm = snippet.get("processMetrics") or {}
    rss_mb = (pm.get("maxRSS") or 0) / 1024
    cpu_s = ((pm.get("userTime") or 0) + (pm.get("systemTime") or 0)) / 1e6
    return rss_mb, cpu_s


def write_csv(path, header, rows):
    with open(path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(rows)


def report(index_file):
    index = json.loads(Path(index_file).read_text())
    build_dir = Path(index["buildDir"])
    data_dir = Path(index["dataDir"])

    snippets = []
    failed_snippets = []
    for name in index.get("snippets", []):
        try:
            snippets.append(json.loads((data_dir / name).read_text()))
        except (OSError, ValueError) as e:
            failed_snippets.append(name)
            print(f"build_report: failed to read snippet {name}: {e}", file=sys.stderr)

    by_role = collections.defaultdict(list)
    for s in snippets:
        by_role[s.get("role")].append(s)

    steps = by_role["compile"] + by_role["link"] + by_role["custom"]
    cmake_builds = by_role["cmakeBuild"]
    if not steps and not cmake_builds:
        return

    label = os.environ.get("RI_BUILD_LABEL", "")
    stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    reports_dir = build_dir / "instrumentation-reports"
    base_name = f"{stamp}-{label}" if label else stamp
    out_dir = reports_dir / base_name
    suffix = 1
    while out_dir.exists():
        suffix += 1
        out_dir = reports_dir / f"{base_name}-{suffix}"
    out_dir.mkdir(parents=True)

    sec = lambda s: s["duration"] / 1000.0
    if cmake_builds:
        wall_s = max(sec(s) for s in cmake_builds)
    else:
        start = min(s["timeStart"] for s in steps)
        end = max(s["timeStart"] + s["duration"] for s in steps)
        wall_s = (end - start) / 1000.0

    compiles = sorted(by_role["compile"], key=sec, reverse=True)
    links = sorted(by_role["link"], key=sec, reverse=True)
    customs = sorted(by_role["custom"], key=sec, reverse=True)
    compile_s = sum(map(sec, compiles))
    link_s = sum(map(sec, links))
    custom_s = sum(map(sec, customs))
    step_s = compile_s + link_s + custom_s
    parallelism = step_s / wall_s if wall_s > 0 else 0.0

    def rel(path):
        path = str(path or "").replace("\\", "/")
        for root in (str(build_dir).replace("\\", "/"), os.getcwd().replace("\\", "/")):
            if path.lower().startswith(root.lower() + "/"):
                return path[len(root) + 1 :]
        return path

    write_csv(
        out_dir / "compiles.csv",
        ["seconds", "maxRSS_MB", "cpu_s", "target", "source"],
        [
            # Absolute source path, header_impact.py matches it against compile_commands.json
            [f"{sec(s):.2f}", f"{metrics(s)[0]:.0f}", f"{metrics(s)[1]:.2f}", s.get("target", ""),
             str(s.get("source") or "").replace("\\", "/")]
            for s in compiles
        ],
    )
    write_csv(
        out_dir / "links.csv",
        ["seconds", "maxRSS_MB", "target", "output"],
        [
            [f"{sec(s):.2f}", f"{metrics(s)[0]:.0f}", s.get("target", ""), rel((s.get("outputs") or [""])[0])]
            for s in links
        ],
    )
    write_csv(
        out_dir / "custom.csv",
        ["seconds", "kind", "name"],
        [[f"{sec(s):.2f}", custom_kind(s), custom_name(s)] for s in customs],
    )

    per_target = collections.defaultdict(lambda: [0.0, 0, 0.0])
    for s in compiles:
        entry = per_target[s.get("target", "")]
        entry[0] += sec(s)
        entry[1] += 1
    for s in links:
        per_target[s.get("target", "")][2] += sec(s)
    targets = sorted(per_target.items(), key=lambda kv: kv[1][0] + kv[1][2], reverse=True)
    write_csv(
        out_dir / "targets.csv",
        ["target", "compile_s", "compile_count", "link_s"],
        [[t, f"{c:.1f}", n, f"{l:.1f}"] for t, (c, n, l) in targets],
    )

    per_kind = collections.Counter()
    for s in customs:
        per_kind[custom_kind(s)] += sec(s)

    if index.get("trace"):
        trace = data_dir / index["trace"]
        if trace.exists():
            shutil.copy(trace, out_dir / "trace.json")

    system = index.get("staticSystemInformation") or {}
    lines = [
        f"# Build report {stamp} {label}".rstrip(),
        "",
    ]
    if failed_snippets:
        lines += [
            f"**INCOMPLETE: {len(failed_snippets)} snippet(s) could not be read, totals are too low and the run is "
            "not added to history.csv**",
            "",
        ]
    lines += [
        f"- Host: {system.get('hostname', '?')}, {system.get('processorName', '?')}, "
        f"{system.get('numberOfLogicalCPU', '?')} logical CPUs",
        f"- Wall time: {wall_s:.1f} s",
        f"- Sum of step durations: {step_s:.1f} s (average parallelism {parallelism:.1f})",
        f"- Compile: {len(compiles)} steps, {compile_s:.1f} s",
        f"- Link: {len(links)} steps, {link_s:.1f} s",
        f"- Custom: {len(customs)} steps, {custom_s:.1f} s ("
        + ", ".join(f"{k} {v:.1f} s" for k, v in per_kind.most_common())
        + ")",
        "",
        f"## Top {TOP_COUNT} compiles",
        "",
        "| s | MB | target | source |",
        "|---:|---:|---|---|",
    ]
    lines += [
        f"| {sec(s):.1f} | {metrics(s)[0]:.0f} | {s.get('target', '')} | {rel(s.get('source'))} |"
        for s in compiles[:TOP_COUNT]
    ]
    lines += ["", "## Links", "", "| s | MB | target |", "|---:|---:|---|"]
    lines += [f"| {sec(s):.1f} | {metrics(s)[0]:.0f} | {s.get('target', '')} |" for s in links]
    lines += ["", "## Top 20 targets by compile + link time", "", "| target | compile s | TUs | link s |", "|---|---:|---:|---:|"]
    lines += [f"| {t} | {c:.1f} | {n} | {l:.1f} |" for t, (c, n, l) in targets[:20]]
    (out_dir / "summary.md").write_text("\n".join(lines) + "\n")

    if failed_snippets:
        print(f"Build report: {out_dir / 'summary.md'} is INCOMPLETE ({len(failed_snippets)} unreadable snippets), "
              "not added to history.csv", file=sys.stderr)
        return

    history = reports_dir / "history.csv"
    new_file = not history.exists()
    with open(history, "a", newline="") as f:
        writer = csv.writer(f)
        if new_file:
            writer.writerow(
                ["timestamp", "label", "wall_s", "compile_s", "link_s", "custom_s",
                 "compile_count", "link_count", "custom_count", "parallelism", "report"]
            )
        writer.writerow(
            [stamp, label, f"{wall_s:.1f}", f"{compile_s:.1f}", f"{link_s:.1f}", f"{custom_s:.1f}",
             len(compiles), len(links), len(customs), f"{parallelism:.1f}", out_dir.name]
        )

    print(f"Build report: {out_dir / 'summary.md'} (wall {wall_s:.1f} s)")


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "install":
        install(sys.argv[2])
    elif len(sys.argv) == 2:
        report(sys.argv[1])
    else:
        print(__doc__)
        sys.exit(1)
