import sys
import warnings

REQUIRED_VERSION_HEX = 0x02070000


if sys.hexversion < REQUIRED_VERSION_HEX:
    raise Exception("ERT GUI Python requires at least version 2.7 of Python")
    
