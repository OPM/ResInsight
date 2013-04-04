#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plotdata.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from ert.ert.erttypes import time_t
from ert_gui.widgets.helpedwidget import ContentModel
from ert_gui.widgets.util import print_timing, resourceIcon
from ert_gui.pages.config.parameters.parametermodels import DataModel, KeywordModel, FieldModel, SummaryModel
from ert_gui.pages.config.parameters.parameterpanel import Parameter
import ert.ert.enums as enums
import sys
from ert.ert.enums import obs_impl_type

from ensemblefetcher import EnsembleFetcher
from rftfetcher import RFTFetcher
from PyQt4.QtGui import QFrame
from PyQt4.QtCore import SIGNAL, QObject
import ert_gui.widgets as widgets




class PlotDataFetcher(ContentModel, QObject):

    def __init__(self):
        ContentModel.__init__(self)
        QObject.__init__(self)
        self.parameter = None

        # the order of these handlers depend on ERT's way of keeping track of the keys
        self.handlers = [RFTFetcher(), EnsembleFetcher()]
        self.current_handler = None
        self.empty_panel = QFrame()

        self.fs_for_comparison_plots = None
        self.comparison_fs_name = "None"
        self.data = None

    def initialize(self, ert):
        for handler in self.handlers:
            handler.initialize(ert)
            self.connect(handler, SIGNAL('dataChanged()'), self.__dataChanged)

        ert.prototype("long enkf_main_mount_extra_fs(long, char*)")
        ert.prototype("void enkf_fs_free(long)")


    #@print_timing
    @widgets.util.may_take_a_long_time
    def getter(self, ert):
        data = PlotData()
        if not self.parameter is None:
            key = self.parameter.getName()
            data.setName(key)

            for handler in self.handlers:
                if handler.isHandlerFor(ert, key):
                    handler.fetch(ert, key, self.parameter, data, self.fs_for_comparison_plots)
                    self.current_handler = handler
                    break

        return data


    def __dataChanged(self):
        self.fetchContent()
        self.emit(SIGNAL('dataChanged()'))

    def fetchContent(self):
        self.data = self.getFromModel()

    def setParameter(self, parameter, context_data):
        self.findHandler(parameter.getName())
        if not self.current_handler is None:
            self.parameter = parameter
            self.current_handler.configure(parameter, context_data)

    def getParameter(self):
        return self.parameter

    def getConfigurationWidget(self, context_data):
        if self.current_handler is None:
            return self.empty_panel
        else:
            cw = self.current_handler.getConfigurationWidget(context_data)
            if cw is None:
                cw = self.empty_panel
            return cw

    def findHandler(self, key):
        ert = self.getModel()
        self.current_handler = None
        for handler in self.handlers:
            if handler.isHandlerFor(ert, key): #todo: what about multiple hits?
                self.current_handler = handler
                break

    def updateComparisonFS(self, new_fs):
        if not new_fs == self.comparison_fs_name:
            ert = self.getModel()

            if self.fs_for_comparison_plots:
                ert.enkf.enkf_fs_free(self.fs_for_comparison_plots)
                self.fs_for_comparison_plots = None
                self.comparison_fs_name = "None"

            if not new_fs == "None":
                self.fs_for_comparison_plots = ert.enkf.enkf_main_mount_extra_fs(ert.main, new_fs)
                self.comparison_fs_name = new_fs

            self.__dataChanged()



class PlotData:
    def __init__(self, name="undefined"):
        self.name = name
        self.key_index = None

        self.x_data = {}
        self.y_data = {}

        self.x_comp_data = {}
        self.y_comp_data = {}

        self.obs_x = None
        self.obs_y = None
        self.obs_std_x = None
        self.obs_std_y = None

        self.refcase_x = None
        self.refcase_y = None


        self.x_min = None
        self.x_max = None

        self.y_min = None
        self.y_max = None

        self.y_data_type = "number"
        self.x_data_type = "time"

        self.inverted_y_axis = False

        self.valid = True
        self.key_index_is_index = False

    def checkMaxMin(self, value):
        if self.x_min is None or self.x_max is None:
            self.x_min = value
            self.x_max = value
            
        self.x_min = min(value, self.x_min)
        self.x_max = max(value, self.x_max)

    def checkMaxMinY(self, value):
        if self.y_min is None or self.y_max is None:
            self.y_min = value
            self.y_max = value

        self.y_min = min(value, self.y_min)
        self.y_max = max(value, self.y_max)

    def getName(self):
        return self.name

    def setName(self, name):
        self.name = name

    def setKeyIndex(self, key_index):
        self.key_index = key_index

    def getKeyIndex(self):
        return self.key_index

    def getXDataType(self):
        return self.x_data_type

    def getYDataType(self):
        return self.y_data_type

    def hasInvertedYAxis(self):
        return self.inverted_y_axis

    def getTitle(self):
        name = self.getName()
        key_index = self.getKeyIndex()

        title = name
        if not key_index is None:
            title = "%s (%s)" % (name, key_index)

        return title

    def isValid(self):
        return self.valid

    def setValid(self, valid):
        self.valid = valid

    def setKeyIndexIsIndex(self, bool):
        self.key_index_is_index = bool

    def getSaveName(self):
        if self.key_index_is_index or self.getKeyIndex() is None:
            return self.getName()
        else:
            return ("%s.%s" % (self.getName(), self.getKeyIndex()))
        


class PlotContextDataFetcher(ContentModel):

    observation_icon = resourceIcon("observation")

    def __init__(self):
        ContentModel.__init__(self)

    def initialize(self, ert):

        ert.prototype("long enkf_config_node_get_impl_type(long)")
        ert.prototype("long enkf_config_node_get_ref(long)")

        ert.prototype("long gen_kw_config_alloc_name_list(long)")

        ert.prototype("long gen_data_config_get_initial_size(long)")

        ert.prototype("int field_config_get_nx(long)")
        ert.prototype("int field_config_get_ny(long)")
        ert.prototype("int field_config_get_nz(long)")

        ert.prototype("long enkf_main_get_fs(long)")
        ert.prototype("char* enkf_fs_get_read_dir(long)")
        ert.prototype("long enkf_fs_alloc_dirlist(long)")

        ert.prototype("int plot_config_get_errorbar_max(long)")
        ert.prototype("char* plot_config_get_path(long)")

        ert.prototype("long enkf_main_get_obs(long)")
        ert.prototype("long enkf_obs_alloc_typed_keylist(long, int)")

        self.modelConnect("casesUpdated()", self.fetchContent)


    #@print_timing
    def getter(self, ert):
        data = PlotContextData()

        keys = ert.getStringList(ert.enkf.ensemble_config_alloc_keylist(ert.ensemble_config), free_after_use=True)
        data.keys = keys
        data.parameters = []

        for key in keys:
            config_node = ert.enkf.ensemble_config_get_node(ert.ensemble_config, key)
            type = ert.enkf.enkf_config_node_get_impl_type(config_node)
            node_ref = ert.enkf.enkf_config_node_get_ref(config_node)

            if type == SummaryModel.TYPE:
                p = Parameter(key, SummaryModel.TYPE)
                data.parameters.append(p)

            elif type == FieldModel.TYPE:
                p = Parameter(key, FieldModel.TYPE)
                data.parameters.append(p)

                if data.field_bounds is None:
                    x = ert.enkf.field_config_get_nx(node_ref)
                    y = ert.enkf.field_config_get_ny(node_ref)
                    z = ert.enkf.field_config_get_nz(node_ref)
                    data.field_bounds = (x,y,z)

            elif type == DataModel.TYPE:
                data.parameters.append(Parameter(key, DataModel.TYPE))
                data.gen_data_size = ert.enkf.gen_data_config_get_initial_size(node_ref)

            elif type == KeywordModel.TYPE:
                p = Parameter(key, KeywordModel.TYPE)
                data.parameters.append(p)
                s = ert.enkf.gen_kw_config_alloc_name_list(node_ref)
                data.key_index_list[key] = ert.getStringList(s, free_after_use=True)

        data.errorbar_max = ert.enkf.plot_config_get_errorbar_max(ert.plot_config)

        fs = ert.enkf.enkf_main_get_fs(ert.main)
        current_case = "default" #ert.enkf.enkf_fs_get_read_dir(fs)

        data.plot_config_path = ert.enkf.plot_config_get_path(ert.plot_config)
        #data.plot_path = ert.enkf.plot_config_get_path(ert.plot_config) + "/" + current_case
        data.plot_path = "PLOTXXX"
        
        enkf_obs = ert.enkf.enkf_main_get_obs(ert.main)
        key_list = ert.enkf.enkf_obs_alloc_typed_keylist(enkf_obs, obs_impl_type.FIELD_OBS.value())
        field_obs = ert.getStringList(key_list, free_after_use=True)

        for obs in field_obs:
            p = Parameter(obs, obs_impl_type.FIELD_OBS, PlotContextDataFetcher.observation_icon)
            data.parameters.append(p)


        #case_list_pointer = ert.enkf.enkf_fs_alloc_dirlist(fs)
        #case_list = ert.getStringList(case_list_pointer)
        case_list = ["default"]
        data.current_case = current_case
        #ert.freeStringList(case_list_pointer)

        for case in case_list:
            data.case_list.append(case)

        return data

    def fetchContent(self):
        self.data = self.getFromModel()


class PlotContextData:
    def __init__(self):
        self.keys = None
        self.parameters = None
        self.key_index_list = {}
        self.errorbar_max = 0
        self.plot_path = ""
        self.plot_config_path = ""
        self.field_bounds = None
        self.gen_data_size = 0
        self.case_list = ["None"]
        self.current_case = None

    def getKeyIndexList(self, key):
        if self.key_index_list.has_key(key):
            return self.key_index_list[key]
        else:
            return []

    def getComparableCases(self):
        cases = []

        for case in self.case_list:
            if not case == self.current_case:
                cases.append(case)

        return cases

