#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_fs.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.enums import RealizationStateEnum


class StateMap(BaseCClass):
    def __init__(self):
        c_ptr = StateMap.cNamespace().alloc()
        super(StateMap, self).__init__(c_ptr)

    def __len__(self):
        """ @rtype: int """
        return StateMap.cNamespace().size(self)

    def __iter__(self):
        index = 0
        size = len(self)

        while index < size:
            yield self[index]
            index += 1

    def __getitem__(self, index):
        """ @rtype: RealizationStateEnum """
        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if index >= size:
            raise IndexError("Index out of range: %d < %d" % (index, size))

        return StateMap.cNamespace().iget(self, index)


    def __setitem__(self, index, value):
        if self.isReadOnly():
            raise UserWarning("This State Map is read only!")

        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        if not isinstance(value, RealizationStateEnum):
            raise TypeError("Expected a RealizationStateEnum")

        if index < 0:
            raise IndexError("Index out of range: %d < 0" % index)

        StateMap.cNamespace().iset(self, index, value)

    @staticmethod
    def isLegalTransition(realization_state1, realization_state2):
        """ @rtype: bool """

        if not isinstance(realization_state1, RealizationStateEnum) or not isinstance(realization_state2, RealizationStateEnum):
            raise TypeError("Expected a RealizationStateEnum")

        return StateMap.cNamespace().is_legal_transition(realization_state1, realization_state2)


    def isReadOnly(self):
        """ @rtype: bool """
        return StateMap.cNamespace().is_read_only(self)


    def free(self):
        StateMap.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("state_map", StateMap)
cwrapper.registerType("state_map_obj", StateMap.createPythonObject)
cwrapper.registerType("state_map_ref", StateMap.createCReference)

StateMap.cNamespace().alloc = cwrapper.prototype("c_void_p state_map_alloc()")
StateMap.cNamespace().free = cwrapper.prototype("void state_map_free(state_map)")
StateMap.cNamespace().size = cwrapper.prototype("int state_map_get_size(state_map)")
StateMap.cNamespace().iget = cwrapper.prototype("realisation_state_enum state_map_iget(state_map, int)")
StateMap.cNamespace().iset = cwrapper.prototype("void state_map_iset(state_map, int, realisation_state_enum)")
StateMap.cNamespace().is_read_only = cwrapper.prototype("bool state_map_is_readonly(state_map)")
StateMap.cNamespace().is_legal_transition = cwrapper.prototype("bool state_map_legal_transition(realisation_state_enum, realisation_state_enum)")
