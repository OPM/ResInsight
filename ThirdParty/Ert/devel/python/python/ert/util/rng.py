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
    def __init__(self, alg_type = RngAlgTypeEnum.MZRAN , init_mode = RngInitModeEnum.INIT_CLOCK):
        assert isinstance(alg_type, RngAlgTypeEnum)
        assert isinstance(init_mode, RngInitModeEnum)

        c_ptr = RandomNumberGenerator.cNamespace().rng_alloc(alg_type, init_mode)
        super(RandomNumberGenerator, self).__init__(c_ptr)


    def stateSize(self):
        return RandomNumberGenerator.cNamespace().state_size(self)


    def setState(self , seed_string):
        state_size = self.stateSize()
        if len(seed_string) < state_size:
            raise ValueError("The seed string must be at least %d characters long" % self.stateSize())
        RandomNumberGenerator.cNamespace().set_state(self , seed_string)


    def getDouble(self):
        """ @rtype: float """
        return RandomNumberGenerator.cNamespace().get_double(self)


    def getInt(self, max=None):
        """ @rtype: float """
        if max is None:
            max = RandomNumberGenerator.cNamespace().get_max_int(self)

        return RandomNumberGenerator.cNamespace().get_int(self, max)


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
RandomNumberGenerator.cNamespace().get_int = cwrapper.prototype("int rng_get_int(rng, int)")
RandomNumberGenerator.cNamespace().get_max_int = cwrapper.prototype("uint rng_get_max_int(rng)")
RandomNumberGenerator.cNamespace().state_size = cwrapper.prototype("int rng_state_size(rng)")
RandomNumberGenerator.cNamespace().set_state = cwrapper.prototype("void rng_set_state(rng , char*)")
