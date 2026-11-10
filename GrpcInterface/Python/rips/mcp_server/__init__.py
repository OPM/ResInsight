"""MCP server for ResInsight.

Lets an MCP client (an AI assistant, an IDE) look at the project in a running
ResInsight through a small set of hand-picked, read-only tools. Install with
``pip install rips[mcp]`` and start with ``python -m rips.mcp_server``.

Tools return small JSON documents. Anything per cell or per time step comes
back as statistics or as a capped number of points, never as a full array.
"""
