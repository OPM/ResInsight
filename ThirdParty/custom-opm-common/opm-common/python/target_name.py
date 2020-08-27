from __future__ import print_function

try:
    from importlib.machinery import EXTENSION_SUFFIXES
    suffix = EXTENSION_SUFFIXES[0]
except ImportError:
    suffix = ".so"

print("libopmcommon_python{}".format(suffix), end="")
