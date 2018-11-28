# The structure of the python packages have been reorganized several times,
# unfortunately we have not managed to deprecate properly and guide users over
# to the new layout. We therefor now have two(?) varietes of old layout which
# must(?) be supported for a while.
#
# from ert.ecl import EclEgrid
# from ecl.ecl import EclGrid
#
# Both of these legacy forms are controlled by the cmake switch
# INSTALL_ERT_LEGACY.

from ert.ecl import *
