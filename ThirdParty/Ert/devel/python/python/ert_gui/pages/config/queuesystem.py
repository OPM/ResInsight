#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'queuesystem.py' is part of ERT - Ensemble based Reservoir Tool. 
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
# Queue System tab
# ----------------------------------------------------------------------------------------------
from ert_gui.widgets.configpanel import ConfigPanel
from ert_gui.widgets.combochoice import ComboChoice
from ert_gui.widgets.stringbox import StringBox
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.spinnerwidgets import IntegerSpinner
from ert_gui.widgets.tablewidgets import KeywordTable

def createQueueSystemPage(configPanel, parent):
    configPanel.startPage("Queue System")

    r = configPanel.addRow(ComboChoice(parent, ["LSF", "RSH", "LOCAL"], "Queue system", "config/queue_system/queue_system"))
    r.initialize = lambda ert : ert.main.site_config.get_queue_name
    r.getter = lambda ert : ert.main.site_config.get_queue_name
    r.setter = lambda ert, value : ert.main.site_config.set_job_queue(str(value))

    internalPanel = ConfigPanel(parent)

    internalPanel.startPage("LSF")

    r = internalPanel.addRow(StringBox(parent, "LSF Queue", "config/queue_system/lsf_queue"))
    r.initialize = lambda ert : ert.main.site_config.get_lsf_queue
    r.getter = lambda ert : ert.main.site_config.get_lsf_queue
    r.setter = lambda ert, value : ert.main.site_config.set_lsf_queue( str(value))

    r = internalPanel.addRow(IntegerSpinner(parent, "Max running", "config/queue_system/max_running_lsf", 1, 1000))
    r.initialize = lambda ert : ert.main.site_config.get_max_running_lsf
    r.getter = lambda ert : ert.main.site_config.get_max_running_lsf
    r.setter = lambda ert, value : ert.main.site_config.set_max_running_lsf( value)

    r = internalPanel.addRow(StringBox(parent, "Resources", "config/queue_system/lsf_resources"))
    r.initialize = lambda ert : ert.main.site_config.get_lsf_request
    r.getter = lambda ert : ert.main.site_config.get_lsf_request
    r.setter = lambda ert, value : ert.main.site_config.set_lsf_request(str(value))

    internalPanel.endPage()


    internalPanel.startPage("RSH")

    r = internalPanel.addRow(PathChooser(parent, "Command", "config/queue_system/rsh_command", show_files=True, must_exist=True, is_executable_file=True))
    r.initialize = lambda ert : ert.main.site_config.get_rsh_command
    r.getter = lambda ert : ert.main.site_config.get_rsh_command
    r.setter = lambda ert, value : ert.main.site_config.set_rsh_command(str(value))

    r = internalPanel.addRow(IntegerSpinner(parent, "Max running", "config/queue_system/max_running_rsh", 1, 1000))
    r.initialize = lambda ert : ert.main.site_config.get_max_running_rsh
    r.getter = lambda ert : ert.main.site_config.get_max_running_rsh
    r.setter = lambda ert, value : ert.main.site_config.set_max_running_rsh( value)


    r = internalPanel.addRow(KeywordTable(parent, "Host List", "config/queue_system/rsh_host_list", "Host", "Number of jobs"))
    r.initialize = lambda ert : ert.getHash(ert.main.site_config.get_rsh_host_list, True)
    r.getter = lambda ert : ert.getHash(ert.main.site_config.get_rsh_host_list, True)

    def add_rsh_host(ert, listOfKeywords):
        ert.main.site_config.clear_rsh_host_list

        for keyword in listOfKeywords:
            if keyword[1].strip() == "":
                max_running = 1
            else:
                max_running = int(keyword[1])

            ert.main.site_config.add_rsh_host( keyword[0], max_running)

    r.setter = add_rsh_host


    internalPanel.endPage()

    internalPanel.startPage("LOCAL")

    r = internalPanel.addRow(IntegerSpinner(parent, "Max running", "config/queue_system/max_running_local", 1, 1000))
    r.initialize = lambda ert : ert.main.site_config.get_max_running_local
    r.getter = lambda ert : ert.main.site_config.get_max_running_local
    r.setter = lambda ert, value : ert.main.site_config.set_max_running_local(value)

    internalPanel.endPage()
    configPanel.addRow(internalPanel)

    configPanel.endPage()
