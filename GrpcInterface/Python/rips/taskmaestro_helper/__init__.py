"""Bridge between ResInsight and the taskmaestro workflow library.

ResInsight invokes this package as a subprocess to execute workflows against
a running ResInsight. Workflow schemas are read with the taskmaestro CLI
(`python -m taskmaestro workflow describe`).
"""
