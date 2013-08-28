#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'rftfetcher.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from fetcher import PlotDataFetcherHandler
import ert.ert.ertwrapper as ertwrapper
import ert.ert.enums as enums
from ert.ert.enums import ert_state_enum, obs_impl_type
import numpy

class RFTFetcher(PlotDataFetcherHandler):

    def __init__(self):
        PlotDataFetcherHandler.__init__(self)

    def initialize(self, ert):
        self.initialized = True

    def isHandlerFor(self, ert, key):
        enkf_obs = ert.main.get_obs
        key_list = enkf_obs.alloc_typed_keylist(obs_impl_type.GEN_OBS.value())
        return key in key_list

    def fetch(self, ert, key, parameter, data, comparison_fs):
        enkf_obs = ert.main.get_obs
        obs_vector = enkf_obs.get_vector(key)

        num_active = obs_vector.get_num_active
        if num_active == 1:
            report_step = obs_vector.get_active_report_step
        elif num_active > 1:
            history_length = ert.main.get_history_length
            active = []
            for index in range(history_length):
                if obs_vector.iget_active(index):
                    active.append(index)
            print "Active:", active
            report_step = active[0] #todo: enable selection from GUI
        else:
            return

        fs = ert.main.get_fs
        state_kw = obs_vector.get_state_kw

        ens_size = ert.main.get_ensemble_size

        config_node = ert.main.ensemble_config.get_node(state_kw)
        field_config = config_node.get_ref
        block_obs = obs_vector.iget_node(report_step)

        obs_size = block_obs.get_size
        grid = field_config.get_grid

        node = config_node.alloc_node

        y_obs = []
        x_obs = []
        x_std = []
        xpos = (ertwrapper.c_double)()
        ypos = (ertwrapper.c_double)()
        zpos = (ertwrapper.c_double)()
        value = (ertwrapper.c_double)()
        std = (ertwrapper.c_double)()
        for index in range(obs_size):
            grid.get_xyz3(block_obs.iget_i(index),block_obs.iget_j(index),block_obs.iget_k(index), xpos, ypos , zpos)
            y_obs.append(zpos.value)
            block_obs.iget(index, value, std)
            x_obs.append(value.value)
            x_std.append(std.value)
            data.checkMaxMin(value.value + std.value)
            data.checkMaxMin(value.value - std.value)
        data.obs_y = numpy.array(y_obs)
        data.obs_x = numpy.array(x_obs)
        data.obs_std_x = numpy.array(x_std)
        data.obs_std_y = None
        var_type = enums.enkf_var_type.DYNAMIC_RESULT

        for member in range(ens_size):
            if node.vector_storage:
                if fs.has_vector(config_node, member, ert_state_enum.ANALYZED.value()):
                    fs.fread_vector(node, member, ert_state_enum.ANALYZED.value())
                elif fs.has_vector(config_node, member, ert_state_enum.FORECAST.value()):
                    fs.fread_vector(node, member, ert_state_enum.FORECAST.value())
                else:
                    print "No data found for member %d/%d." % (member, report_step)
                    continue
            else:
                if fs.has_node(config_node, var_type, report_step, member, ert_state_enum.ANALYZED.value()):
                    fs.fread_node(node, var_type, report_step, member, ert_state_enum.ANALYZED.value())
                elif fs.has_node(config_node, var_type, report_step, member, ert_state_enum.FORECAST.value()):
                    fs.fread_node(node, var_type, report_step, member, ert_state_enum.FORECAST.value())
                else:
                    print "No data found for member %d/%d." % (member, report_step)
                    continue
                
            data.x_data[member] = []
            data.y_data[member] = []
            x_data = data.x_data[member]
            y_data = data.y_data[member]

            field = node.value_ptr
            for index in range(obs_size):
                value = field.ijk_get_double(i[index] , j[index] , k[index])
                x_data.append(value)
                y_data.append(y_obs[index])
                data.checkMaxMin(value)

            data.x_data[member] = numpy.array(x_data)
            data.y_data[member] = numpy.array(y_data)

        if not comparison_fs is None:
            comp_node = config_node.alloc
            for member in range(ens_size):
                if comparison_fs.has_node(config_node, report_step, member, ert_state_enum.ANALYZED.value()):
                    comparison_fs.fread_node(comp_node, report_step, member, ert_state_enum.ANALYZED.value())
                elif comparison_fs.has_node(config_node, report_step, member, ert_state_enum.FORECAST.value()):
                    comparison_fs.fread_node(comp_node, report_step, member, ert_state_enum.FORECAST.value())
                else:
                    print "No data found for member %d/%d." % (member, report_step)
                    continue

                data.x_comp_data[member] = []
                data.y_comp_data[member] = []
                x_data = data.x_comp_data[member]
                y_data = data.y_comp_data[member]

                field = ert.enkf.enkf_node_value_ptr(comp_node)
                for index in range(obs_size):
                    value = field.ijk_get_double(block_obs.iget_i(index),block_obs.iget_j(index),block_obs.iget_k(index))
                    x_data.append(value)
                    y_data.append(y_obs[index])
                    data.checkMaxMin(value)

                data.x_comp_data[member] = numpy.array(x_data)
                data.y_comp_data[member] = numpy.array(y_data)

            comp_node.free

        node.free

        data.x_data_type = "number"
        data.inverted_y_axis = True

    def getConfigurationWidget(self, context_data):
        return None

    def configure(self, parameter, context_data):
        pass #nothing to configure, yet



