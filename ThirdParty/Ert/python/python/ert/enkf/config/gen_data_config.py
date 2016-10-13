#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'gen_data_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf.enums import GenDataFileType


class GenDataConfig(BaseCClass):
    def __init__(self, key , input_format = GenDataFileType.ASCII):
        # Can currently only create GEN_DATA instances which should be used
        # as result variables.
        c_pointer = GenDataConfig.cNamespace().alloc( key , input_format )
        super(GenDataConfig, self).__init__(c_pointer)


    def get_template_file(self):
        return GenDataConfig.cNamespace().get_template_file(self)

    def get_template_key(self):
        return GenDataConfig.cNamespace().get_template_key(self)

    def getDataSize(self , report_step):
        data_size = GenDataConfig.cNamespace().get_data_size(self , report_step)
        if data_size < 0:
            raise ValueError("No data has been loaded for %s at report step:%d " % (self.getName() , report_step))
        else:
            return data_size
            
    def getActiveMask(self):
         return GenDataConfig.cNamespace().get_active_mask(self)

    def getName(self):
        return GenDataConfig.cNamespace().get_key(self)


    def get_initial_size(self):
        return GenDataConfig.cNamespace().get_initial_size(self)

    def getOutputFormat(self):
        return GenDataConfig.cNamespace().get_output_format(self)

    def getInputFormat(self):
        return GenDataConfig.cNamespace().get_input_format(self)

    def free(self):
        GenDataConfig.cNamespace().free(self)

    def hasReportStep(self, report_step):
        """ @rtype: bool """
        return GenDataConfig.cNamespace().has_report_step(self, report_step)

    def getNumReportStep(self):
        """ @rtype: int """
        return GenDataConfig.cNamespace().get_num_report_step(self)

    def getReportStep(self, index):
        """ @rtype: int """
        return GenDataConfig.cNamespace().iget_report_step(self, index)

    def getReportSteps(self):
        """ @rtype: list of int """
        return [self.getReportStep(index) for index in range(self.getNumReportStep())]


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("gen_data_config", GenDataConfig)


GenDataConfig.cNamespace().alloc = cwrapper.prototype("c_void_p gen_data_config_alloc_GEN_DATA_result( char* , gen_data_file_format_type)")
GenDataConfig.cNamespace().free  = cwrapper.prototype("void gen_data_config_free( gen_data_config )")
GenDataConfig.cNamespace().get_output_format = cwrapper.prototype("gen_data_file_format_type gen_data_config_get_output_format(gen_data_config)")
GenDataConfig.cNamespace().get_input_format = cwrapper.prototype("gen_data_file_format_type gen_data_config_get_input_format(gen_data_config)")
GenDataConfig.cNamespace().get_template_file = cwrapper.prototype("char* gen_data_config_get_template_file(gen_data_config)")
GenDataConfig.cNamespace().get_template_key = cwrapper.prototype("char* gen_data_config_get_template_key(gen_data_config)")
GenDataConfig.cNamespace().get_initial_size = cwrapper.prototype("int gen_data_config_get_initial_size(gen_data_config)")
GenDataConfig.cNamespace().has_report_step = cwrapper.prototype("bool gen_data_config_has_report_step(gen_data_config, int)")
GenDataConfig.cNamespace().get_data_size    = cwrapper.prototype("int gen_data_config_get_data_size__(gen_data_config , int)")
GenDataConfig.cNamespace().get_key          = cwrapper.prototype("char* gen_data_config_get_key(gen_data_config)")
GenDataConfig.cNamespace().get_active_mask  = cwrapper.prototype("bool_vector_ref gen_data_config_get_active_mask(gen_data_config)")

GenDataConfig.cNamespace().get_num_report_step = cwrapper.prototype("int gen_data_config_num_report_step(gen_data_config)")
GenDataConfig.cNamespace().iget_report_step    = cwrapper.prototype("int gen_data_config_iget_report_step(gen_data_config, int)")

