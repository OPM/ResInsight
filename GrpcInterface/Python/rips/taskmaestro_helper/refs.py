"""ResInsight object references in taskmaestro input files.

The Workflow UI writes a `{__resinsight_ref__: <type>, ...id...}` map into
input.yaml for fields that refer to a ResInsight object. `run` resolves these
maps back to live `rips` objects. The C++ side maps Python types to the
`<type>` labels in RimWorkflowDescribeTools.
"""

from __future__ import annotations

from typing import Any

# Marker used inside input.yaml to denote a ResInsight object reference.
REF_MARKER = "__resinsight_ref__"


def _is_ref(value: Any) -> bool:
    return isinstance(value, dict) and REF_MARKER in value


def _holds_ref(value: Any) -> bool:
    """A reference, or a list with a reference among its items."""
    return _is_ref(value) or (
        isinstance(value, list) and any(_holds_ref(item) for item in value)
    )


def strip_refs(value: Any) -> Any:
    """Remove ResInsight object references, which are only valid for one run.

    A list that holds a reference is removed as a whole: dropping only the
    reference would shorten the list and change what the remaining items mean.
    """
    if isinstance(value, dict):
        return {
            key: strip_refs(item) for key, item in value.items() if not _holds_ref(item)
        }
    if isinstance(value, list):
        return [strip_refs(item) for item in value if not _holds_ref(item)]
    return value
