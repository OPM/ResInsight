#  Copyright (C) 2014 Statoil ASA, Norway.
#
#  The file 'ensemble_plot_gen_data.py' is part of ERT - Ensemble based Reservoir Tool.
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

from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.config import EnkfConfigNode
from ert.enkf.enkf_fs import EnkfFs
from ert.enkf.enums.ert_impl_type_enum import ErtImplType
from ert.util import BoolVector, DoubleVector


class EnsemblePlotGenData(BaseCClass):
    def __init__(self, ensemble_config_node, file_system, report_step, input_mask=None):
        assert isinstance(ensemble_config_node, EnkfConfigNode)
        assert ensemble_config_node.getImplementationType() == ErtImplType.GEN_DATA

        c_pointer = EnsemblePlotGenData.cNamespace().alloc(ensemble_config_node)
        super(EnsemblePlotGenData, self).__init__(c_pointer)

        self.__load(file_system, report_step, input_mask)


    def __load(self, file_system, report_step, input_mask=None):
        assert isinstance(file_system, EnkfFs)
        if not input_mask is None:
            assert isinstance(input_mask, BoolVector)

        EnsemblePlotGenData.cNamespace().load(self, file_system, report_step, input_mask)

    def __len__(self):
        """ @rtype: int """
        return EnsemblePlotGenData.cNamespace().size(self)

    def __getitem__(self, index):
        """ @rtype: EnsemblePlotGenDataVector """
        return EnsemblePlotGenData.cNamespace().get(self, index)

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1


    def getMaxValues(self):
        """ @rtype: DoubleVector """
        return EnsemblePlotGenData.cNamespace().max_values(self).setParent(self)

    def getMinValues(self):
        """ @rtype: DoubleVector """
        return EnsemblePlotGenData.cNamespace().min_values(self).setParent(self)

    def free(self):
        EnsemblePlotGenData.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ensemble_plot_gen_data", EnsemblePlotGenData)
cwrapper.registerType("ensemble_plot_gen_data_obj", EnsemblePlotGenData.createPythonObject)
cwrapper.registerType("ensemble_plot_gen_data_ref", EnsemblePlotGenData.createCReference)

EnsemblePlotGenData.cNamespace().free = cwrapper.prototype("void enkf_plot_gendata_free(ensemble_plot_gen_data)")
EnsemblePlotGenData.cNamespace().alloc = cwrapper.prototype("c_void_p enkf_plot_gendata_alloc(enkf_config_node)")

EnsemblePlotGenData.cNamespace().size = cwrapper.prototype("int enkf_plot_gendata_get_size(ensemble_plot_gen_data)")
EnsemblePlotGenData.cNamespace().load = cwrapper.prototype("void enkf_plot_gendata_load(ensemble_plot_gen_data, enkf_fs, int, bool_vector)")
EnsemblePlotGenData.cNamespace().get = cwrapper.prototype("ensemble_plot_gen_data_vector_ref enkf_plot_gendata_iget(ensemble_plot_gen_data, int)")

EnsemblePlotGenData.cNamespace().min_values = cwrapper.prototype("double_vector_ref enkf_plot_gendata_get_min_values(ensemble_plot_gen_data)")
EnsemblePlotGenData.cNamespace().max_values = cwrapper.prototype("double_vector_ref enkf_plot_gendata_get_max_values(ensemble_plot_gen_data)")




