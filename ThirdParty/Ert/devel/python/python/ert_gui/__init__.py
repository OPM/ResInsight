import sys
import warnings

ABSOLUTE_REQUIRED_VERSION_HEX = 0x02060000
WARNING_REQUIRED_VERSION_HEX  = 0x02070000


if sys.hexversion < ABSOLUTE_REQUIRED_VERSION_HEX:
    raise Exception("ERT GUI Python requires at least version 2.6 of Python")
    
if sys.hexversion < WARNING_REQUIRED_VERSION_HEX:
    warnings.warn("To get optimal graphical performance you should use Python 2.7")
