"""Bridge between ResInsight and the taskmaestro workflow library.

ResInsight invokes this package as a subprocess to introspect workflow
schemas and to execute workflows. The Python side exists so ResInsight
itself does not need to parse YAML or know about Pydantic.
"""
