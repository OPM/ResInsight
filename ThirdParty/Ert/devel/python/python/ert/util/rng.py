#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'rng.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.


from ert.util import UTIL_LIB
from ert.cwrap import CWrapper, BaseCClass
from ert.util.enums import RngInitModeEnum
from ert.util.enums import RngAlgTypeEnum



class RandomNumberGenerator(BaseCClass):
    def __init__(self, alg_type, init_mode):
        assert isinstance(alg_type, RngAlgTypeEnum)
        assert isinstance(init_mode, RngInitModeEnum)

        c_ptr = RandomNumberGenerator.cNamespace().rng_alloc(alg_type, init_mode)
        super(RandomNumberGenerator, self).__init__(c_ptr)

    def getDouble(self):
        """ @rtype: float """
        return RandomNumberGenerator.cNamespace().get_double(self)

    def free(self):
        RandomNumberGenerator.cNamespace().free(self)


#################################################################

cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerType("rng", RandomNumberGenerator)
CWrapper.registerType("rng_obj", RandomNumberGenerator.createPythonObject)
CWrapper.registerType("rng_ref", RandomNumberGenerator.createCReference)

RandomNumberGenerator.cNamespace().rng_alloc = cwrapper.prototype("c_void_p rng_alloc(rng_alg_type_enum, rng_init_mode_enum)")
RandomNumberGenerator.cNamespace().free = cwrapper.prototype("void rng_free(rng)")
RandomNumberGenerator.cNamespace().get_double = cwrapper.prototype("double rng_get_double(rng)")
