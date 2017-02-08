#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'active_list.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf import ActiveMode

class ActiveList(BaseCClass):
    TYPE_NAME = "active_list"

    _alloc     = EnkfPrototype("void* active_list_alloc()", bind = False)
    _free      = EnkfPrototype("void  active_list_free(active_list)")
    _add_index = EnkfPrototype("void  active_list_add_index(active_list , int)")
    _asize     = EnkfPrototype("int   active_list_get_active_size(active_list, int)")
    _get_mode  = EnkfPrototype("active_mode_enum active_list_get_mode(active_list)")

    def __init__(self):
        c_ptr = self._alloc()
        super(ActiveList, self).__init__(c_ptr)

    def getMode(self):
        return self._get_mode()

    def addActiveIndex(self, index):
        self._add_index(index)

    def getActiveSize(self, default_value):
        """In mode PARTLY_ACTIVE, we return the size of the active set; In mode
        INACTIVE 0 is returned and if the mode is ALL_ACTIVE, the input
        default_value is returned.
        """
        mode = self.getMode()
        if mode == ActiveMode.PARTLY_ACTIVE:
            return self._asize(0)
        if mode == ActiveMode.INACTIVE:
            return 0
        return default_value

    def free(self):
        self._free()

    def __repr__(self):
        size = ''
        if self.getMode() == ActiveMode.PARTLY_ACTIVE:
            size = ', active_size = %d' % self._asize(0)
        cnt = 'mode = %s%s' % (self.getMode(), size)
        return self._create_repr(cnt)
