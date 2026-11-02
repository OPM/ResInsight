"""Bridge between ResInsight and the taskmaestro workflow library.

ResInsight invokes this package as a subprocess (``python -m
rips.taskmaestro_helper <subcommand>``) to:

- list installed tasks and workflows (``catalog``),
- convert workflow folders and registered workflows to editable workflow
  definitions (``load``, ``export-registered``) and back to YAML (``save``),
- describe nested workflows for read-only display (``describe-nested``),
- execute workflows against a running ResInsight (``run``).
"""
