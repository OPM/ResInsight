#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'analysis.py' is part of ERT - Ensemble based Reservoir Tool. 
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


# ----------------------------------------------------------------------------------------------
# Analysis tab
# ----------------------------------------------------------------------------------------------
from ert_gui.widgets.checkbox import CheckBox
from ert_gui.widgets.spinnerwidgets import IntegerSpinner, DoubleSpinner, DoubleSpinner
import ert_gui.widgets.tablewidgets
from ert_gui.widgets.pathchooser import PathChooser
from PyQt4 import QtGui
import ert.enkf

def createAnalysisPage(configPanel, parent):
    configPanel.startPage("Analysis")

    r = configPanel.addRow(CheckBox(parent, "ENKF rerun", "config/analysis/enkf_rerun", "Perform rerun"))
    r.initialize = lambda ert : ert.main.analysis_config.get_rerun 
    r.getter = lambda ert : ert.main.analysis_config.get_rerun
    r.setter = lambda ert, value : ert.main.analysis_config.set_rerun( value)

    r = configPanel.addRow(IntegerSpinner(parent, "Rerun start", "config/analysis/rerun_start",  0, 100000))
    r.initialize = lambda ert : ert.main.analysis_config.get_rerun_start
    r.getter = lambda ert : ert.main.analysis_config.get_rerun_start
    r.setter = lambda ert, value : ert.main.analysis_config.set_rerun_start( value)

    r = configPanel.addRow(PathChooser(parent, "ENKF schedule file", "config/analysis/enkf_sched_file"))
    r.initialize = lambda ert : ert.main.model_config.get_enkf_sched_file
    r.getter = lambda ert : ert.main.model_config.get_enkf_sched_file
    r.setter = lambda ert, value : ert.main.model_config.set_enkf_sched_file(str(value))

    r = configPanel.addRow(ert_gui.widgets.tablewidgets.KeywordList(parent, "Local config", "config/analysis/local_config"))
    r.newKeywordPopup = lambda list : QtGui.QFileDialog.getOpenFileName(r, "Select a path", "")

    def get_local_config_files(ert):
        local_config = ert.main.local_config
        config_files_pointer = ert.main.local_config.get_config_files
        return config_files_pointer

    r.initialize = get_local_config_files
    r.getter = get_local_config_files

    def add_config_file(ert, value):
        local_config = ert.main.local_config
        ert.main.local_config.clear_config_files

        for file in value:
            ert.main.local_config.add_config_file( file)

    r.setter = add_config_file

    r = configPanel.addRow(PathChooser(parent, "Update log", "config/analysis/update_log"))
    r.initialize = lambda ert : ert.main.analysis_config.get_log_path
    r.getter = lambda ert : ert.main.analysis_config.get_log_path
    r.setter = lambda ert, value : ert.main.analysis_config.set_log_path( str(value))


    configPanel.startGroup("EnKF")

    r = configPanel.addRow(DoubleSpinner(parent, "Alpha", "config/analysis/enkf_alpha", 0, 100000, 2))
    r.initialize = lambda ert : ert.main.analysis_config.get_alpha
    r.getter = lambda ert : ert.main.analysis_config.get_alpha
    r.setter = lambda ert, value : ert.main.analysis_config.set_alpha( value)

    r = configPanel.addRow(CheckBox(parent, "Merge Observations", "config/analysis/enkf_merge_observations", "Perform merge"))
    r.initialize = lambda ert : ert.main.analysis_config.get_merge_observations 
    r.getter = lambda ert : ert.main.analysis_config.get_merge_observations
    r.setter = lambda ert, value : ert.main.analysis_config.set_merge_observations( value)

    configPanel.endGroup()
    configPanel.endPage()
