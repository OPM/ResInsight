"""ResInsight object references in taskmaestro input files.

The Workflow UI writes a `{__resinsight_ref__: <type>, ...id...}` map into
input.yaml for fields that refer to a ResInsight object. `run` resolves these
maps back to live `rips` objects. The C++ side maps Python types to the
`<type>` labels in RimWorkflowDescribeTools.
"""

from __future__ import annotations

# Marker used inside input.yaml to denote a ResInsight object reference.
REF_MARKER = "__resinsight_ref__"
