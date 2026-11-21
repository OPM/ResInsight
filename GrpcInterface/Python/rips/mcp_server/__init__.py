"""MCP server for ResInsight.

Lets an MCP client (an AI assistant, an IDE) look at the project in a running
ResInsight through a small set of hand-picked, read-only tools. Install with
``pip install rips[mcp]`` and start with ``python -m rips.mcp_server``.

Tools return small JSON documents. Lists and summary vectors come back capped,
never as a full array.
"""
