"""Check that the generated protobuf code can be loaded by every supported runtime.

The protobuf runtime refuses to load gencode newer than itself, so the gencode
version emitted by grpcio-tools must not exceed the lower bound declared for
protobuf in pyproject.toml. Nothing ties those two together: a grpcio-tools
upgrade can move the gencode a full major version without any edit here, and
the resulting package still installs cleanly and only fails at import time.
"""

import re
import sys
import tomllib
from pathlib import Path

from packaging.requirements import Requirement
from packaging.version import Version

HERE = Path(__file__).parent
GENERATED = HERE / "rips" / "generated"
GENCODE_PATTERN = re.compile(r"^# Protobuf Python Version: (\S+)", re.MULTILINE)


def declared_protobuf_floor() -> Version:
    """Lowest protobuf runtime a user of this package can end up with."""
    with (HERE / "pyproject.toml").open("rb") as f:
        dependencies = tomllib.load(f)["project"]["dependencies"]

    requirements = [Requirement(d) for d in dependencies]
    protobuf = next((r for r in requirements if r.name == "protobuf"), None)
    if protobuf is None:
        sys.exit("protobuf is not declared in [project].dependencies")

    floors = [
        Version(s.version) for s in protobuf.specifier if s.operator in (">=", "==")
    ]
    if not floors:
        sys.exit(
            f"protobuf dependency '{protobuf}' has no lower bound to check against"
        )

    return max(floors)


def gencode_versions() -> dict[str, Version]:
    versions = {}
    for path in sorted(GENERATED.glob("*_pb2.py")):
        match = GENCODE_PATTERN.search(path.read_text(encoding="utf-8"))
        if match is None:
            sys.exit(f"{path.name} has no gencode version header")
        versions[path.name] = Version(match.group(1))
    return versions


def main() -> None:
    floor = declared_protobuf_floor()
    versions = gencode_versions()
    if not versions:
        sys.exit(f"no generated *_pb2.py files in {GENERATED}, nothing was verified")

    too_new = {name: v for name, v in versions.items() if v > floor}
    if too_new:
        listing = "\n".join(f"  {name}: gencode {v}" for name, v in too_new.items())
        sys.exit(
            f"Generated code is newer than the oldest supported protobuf runtime ({floor}).\n"
            f"{listing}\n"
            "Pin grpcio-tools to a release emitting older gencode, or raise the protobuf "
            "lower bound in pyproject.toml. Note that raising it drops support for runtimes "
            "below the new bound."
        )

    print(
        f"{len(versions)} generated files, gencode {max(versions.values())}, runtime floor {floor}: OK"
    )


if __name__ == "__main__":
    main()
