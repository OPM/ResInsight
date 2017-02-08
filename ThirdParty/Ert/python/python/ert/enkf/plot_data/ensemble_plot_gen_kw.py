# Copyright (C) 2014 Statoil ASA, Norway.
#
#  The file 'ensemble_plot_gen_kw.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf.config import EnkfConfigNode
from ert.enkf.enkf_fs import EnkfFs
from ert.enkf.enums.ert_impl_type_enum import ErtImplType
from ert.util import BoolVector
from ert.enkf.plot_data import EnsemblePlotGenKWVector


class EnsemblePlotGenKW(BaseCClass):
    TYPE_NAME = "ensemble_plot_gen_kw"

    _alloc                = EnkfPrototype("void* enkf_plot_gen_kw_alloc(enkf_config_node)", bind = False)
    _size                 = EnkfPrototype("int   enkf_plot_gen_kw_get_size(ensemble_plot_gen_kw)")
    _load                 = EnkfPrototype("void  enkf_plot_gen_kw_load(ensemble_plot_gen_kw, enkf_fs, bool, int, bool_vector)")
    _get                  = EnkfPrototype("ensemble_plot_gen_kw_vector_ref enkf_plot_gen_kw_iget(ensemble_plot_gen_kw, int)")
    _iget_key             = EnkfPrototype("char* enkf_plot_gen_kw_iget_key(ensemble_plot_gen_kw, int)")
    _get_keyword_count    = EnkfPrototype("int   enkf_plot_gen_kw_get_keyword_count(ensemble_plot_gen_kw)")
    _should_use_log_scale = EnkfPrototype("bool  enkf_plot_gen_kw_should_use_log_scale(ensemble_plot_gen_kw, int)")
    _free                 = EnkfPrototype("void  enkf_plot_gen_kw_free(ensemble_plot_gen_kw)")

    def __init__(self, ensemble_config_node, file_system, input_mask=None):
        assert isinstance(ensemble_config_node, EnkfConfigNode)
        assert ensemble_config_node.getImplementationType() == ErtImplType.GEN_KW

        c_pointer = self._alloc(ensemble_config_node)
        super(EnsemblePlotGenKW, self).__init__(c_pointer)

        self.__load(file_system, input_mask)


    def __load(self, file_system, input_mask=None):
        assert isinstance(file_system, EnkfFs)
        if not input_mask is None:
            assert isinstance(input_mask, BoolVector)

        self._load(file_system, True, 0, input_mask)

    def __len__(self):
        """ @rtype: int """
        return self._size()

    def __getitem__(self, index):
        """ @rtype: EnsemblePlotGenKWVector """
        return self._get(index)

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1

    def getKeyWordCount(self):
        """ @rtype: int """
        return self._get_keyword_count()

    def getKeyWordForIndex(self, index):
        """ @rtype: str """
        return self._iget_key(index)

    def getIndexForKeyword(self, keyword):
        """ @rtype: int """
        for index in range(self.getKeyWordCount()):
            kw = self.getKeyWordForIndex(index)
            if kw == keyword:
                return index
        return None

    def shouldUseLogScale(self, index):
        """ @rtype: bool """
        return bool(self._should_use_log_scale(index))

    def free(self):
        self._free()

    def __repr__(self):
        return 'EnsemblePlotGenKW(size = %d) %s' % (len(self), self._ad_str())
