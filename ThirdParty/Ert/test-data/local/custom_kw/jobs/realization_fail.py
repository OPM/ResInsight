#!/usr/bin/env python
import sys

iens = None
if len(sys.argv) > 1:
    iens = int(sys.argv[1])

if iens is None or not iens in [2, 3, 5, 7]:
    with open("realization.ok", "w") as f:
        f.write("OK")

