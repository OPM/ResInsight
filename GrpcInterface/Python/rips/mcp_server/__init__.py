"""MCP server for ResInsight.

Lets an MCP client (an AI assistant, an IDE) look at the project in a running
ResInsight through a small set of hand-picked tools. Most tools only read; the
change tools set the time step or cell result of a view, open a project or load
a case, and nothing is saved or deleted. Install with ``pip install rips[mcp]``
and start with ``python -m rips.mcp_server``.

Tools return small JSON documents. Lists and summary vectors come back capped,
never as a full array.
"""
