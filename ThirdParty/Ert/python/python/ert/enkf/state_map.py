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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf.enums import RealizationStateEnum
from ert.util import BoolVector


class StateMap(BaseCClass):
    TYPE_NAME = "state_map"

    _alloc               = EnkfPrototype("void* state_map_alloc()", bind = False)
    _fread               = EnkfPrototype("bool  state_map_fread(state_map , char*)")
    _fwrite              = EnkfPrototype("void  state_map_fwrite(state_map , char*)")
    _equal               = EnkfPrototype("bool  state_map_equal(state_map , state_map)")
    _free                = EnkfPrototype("void  state_map_free(state_map)")
    _size                = EnkfPrototype("int   state_map_get_size(state_map)")
    _iget                = EnkfPrototype("realisation_state_enum state_map_iget(state_map, int)")
    _iset                = EnkfPrototype("void  state_map_iset(state_map, int, realisation_state_enum)")
    _select_matching     = EnkfPrototype("void  state_map_select_matching(state_map, bool_vector, realisation_state_enum)")
    _is_read_only        = EnkfPrototype("bool  state_map_is_readonly(state_map)")
    _is_legal_transition = EnkfPrototype("bool  state_map_legal_transition(realisation_state_enum, realisation_state_enum)", bind = False)



    def __init__(self , filename = None):
        c_ptr = self._alloc()
        super(StateMap, self).__init__(c_ptr)
        if filename:
            self.load(filename)


    def __len__(self):
        """ @rtype: int """
        return self._size()

    def __iter__(self):
        index = 0
        size = len(self)

        while index < size:
            yield self[index]
            index += 1

    def __eq__(self , other):
        return self._equal(other)


    def __getitem__(self, index):
        """ @rtype: RealizationStateEnum """
        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if index < 0:
            index += size
        if 0 <= index < size:
            return self._iget(index)
        raise IndexError("Invalid index.  Valid range: [0, %d)" % size)


    def __setitem__(self, index, value):
        if self.isReadOnly():
            raise UserWarning("This State Map is read only!")

        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        if not isinstance(value, RealizationStateEnum):
            raise TypeError("Expected a RealizationStateEnum")

        if index < 0:
            index += len(self)
        if index < 0:
            raise IndexError("Index out of range: %d < 0" % index)

        self._iset(index, value)

    @classmethod
    def isLegalTransition(cls, realization_state1, realization_state2):
        """ @rtype: bool """

        if not isinstance(realization_state1, RealizationStateEnum) or not isinstance(realization_state2, RealizationStateEnum):
            raise TypeError("Expected a RealizationStateEnum")

        return cls._is_legal_transition(realization_state1, realization_state2)


    def isReadOnly(self):
        """ @rtype: bool """
        return self._is_read_only()

    def selectMatching(self, select_target, select_mask):
        """
        @type select_target: BoolVector
        @type select_mask: RealizationStateEnum
        """
        assert isinstance(select_target, BoolVector)
        assert isinstance(select_mask, RealizationStateEnum)

        self._select_matching(select_target, select_mask)


    def realizationList(self , state_value):
        """
        Will create a list of all realisations with state equal to state_value.

        @type state_value: RealizationStateEnum
        @rtype: ert.util.IntVector
        """
        mask = BoolVector(False, len(self))
        self.selectMatching(mask, state_value)
        return BoolVector.createActiveList(mask)



    def free(self):
        self._free()

    def __repr__(self):
        ro = 'read only' if self.isReadOnly() else 'read/write'
        return 'StateMap(size = %d, %s) %s' % (len(self), ro, self._ad_str())

    def load(self,filename):
        if not self._fread(filename):
            raise IOError("Failed to load state map from:%s" % filename)


    def save(self, filename):
        self._fwrite(filename)
