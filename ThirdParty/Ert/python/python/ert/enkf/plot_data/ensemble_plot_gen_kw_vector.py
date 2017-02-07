# Copyright (C) 2014  Statoil ASA, Norway.
#
# The file 'ensemble_plot_gen_kw_vector.py' is part of ERT - Ensemble based Reservoir Tool.
#
# ERT is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ERT is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
# for more details.

from cwrap import BaseCClass
from ert.enkf import EnkfPrototype

class EnsemblePlotGenKWVector(BaseCClass):
    TYPE_NAME = "ensemble_plot_gen_kw_vector"

    _size      = EnkfPrototype("int    enkf_plot_gen_kw_vector_get_size(ensemble_plot_gen_kw_vector)")
    _get_value = EnkfPrototype("double enkf_plot_gen_kw_vector_iget(ensemble_plot_gen_kw_vector, int)")


    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __len__(self):
        """ @rtype: int """
        return self._size()

    def getValue(self, index):
        """ @rtype: float """
        return self[index]

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1

    def __getitem__(self, index):
        """ @rtype: float """
        return self._get_value(index)

    def __repr__(self):
        return 'EnsemblePlotGenKWVector(size = %d) %s' % (len(self), self._ad_str())
