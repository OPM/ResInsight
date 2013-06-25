#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'eclipse.py' is part of ERT - Ensemble based Reservoir Tool. 
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
# Eclipse tab
# ----------------------------------------------------------------------------------------------
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.tablewidgets import KeywordTable, KeywordList
from ert_gui.widgets.configpanel import ConfigPanel
import ert.enkf

def createEclipsePage(configPanel, parent):
    configPanel.startPage("Eclipse")

    r = configPanel.addRow(PathChooser(parent, "Eclipse Base", "config/eclipse/eclbase", path_format=True))
    r.getter = lambda ert : ert.enkf.ecl_config_get_eclbase(ert.ecl_config)
    r.setter = lambda ert, value : ert.enkf.enkf_main_set_eclbase(ert.main , str(value))

    r = configPanel.addRow(PathChooser(parent, "Data file", "config/eclipse/data_file", show_files=True))
    r.getter = lambda ert : ert.enkf.ecl_config_get_data_file(ert.ecl_config)
    r.setter = lambda ert, value : ert.enkf.enkf_main_set_data_file(ert.main , str(value))

    r = configPanel.addRow(PathChooser(parent, "Grid", "config/eclipse/grid", show_files=True))
    r.getter = lambda ert : ert.enkf.ecl_config_get_gridfile(ert.ecl_config)
    r.setter = lambda ert, value : ert.enkf.ecl_config_set_grid(ert.ecl_config, str(value))

    r = configPanel.addRow(PathChooser(parent, "Schedule file" , "config/eclipse/schedule_file" , show_files = True))
    r.getter = lambda ert : ert.enkf.ecl_config_get_schedule_file(ert.ecl_config)
    r.setter = lambda ert, value : ert.enkf.ecl_config_set_schedule_file(ert.ecl_config, str(value))


    r = configPanel.addRow(PathChooser(parent, "Init section", "config/eclipse/init_section", show_files=True))
    r.getter = lambda ert : ert.enkf.ecl_config_get_init_section(ert.ecl_config)
    r.setter = lambda ert, value : ert.enkf.ecl_config_set_init_section(ert.ecl_config, str(value))


    r = configPanel.addRow(PathChooser(parent, "Refcase", "config/eclipse/refcase", show_files=True))
    r.getter = lambda ert : ert.enkf.ecl_config_get_refcase_name(ert.ecl_config)
    r.setter = lambda ert, value : ert.enkf.ecl_config_load_refcase(ert.ecl_config, str(value))

    r = configPanel.addRow(PathChooser(parent, "Schedule prediction file", "config/eclipse/schedule_prediction_file", show_files=True))
    r.getter = lambda ert : ert.enkf.enkf_main_get_schedule_prediction_file(ert.main)
    r.setter = lambda ert, value : ert.enkf.enkf_main_set_schedule_prediction_file(ert.main, ert.nonify( value ))

    r = configPanel.addRow(KeywordTable(parent, "Data keywords", "config/eclipse/data_kw"))
    r.getter = lambda ert : ert.getSubstitutionList(ert.enkf.enkf_main_get_data_kw(ert.main))

    def add_data_kw(ert, listOfKeywords):
        ert.enkf.enkf_main_clear_data_kw(ert.main)

        for keyword in listOfKeywords:
            ert.enkf.enkf_main_add_data_kw(ert.main, keyword[0], keyword[1])

    r.setter = add_data_kw



    configPanel.addSeparator()

    internalPanel = ConfigPanel(parent)

    internalPanel.startPage("Static keywords")

    r = internalPanel.addRow(KeywordList(parent, "", "config/eclipse/add_static_kw"))
    r.getter = lambda ert : ert.getStringList(ert.enkf.ecl_config_get_static_kw_list(ert.ecl_config))

    def add_static_kw(ert, listOfKeywords):
        ert.enkf.ecl_config_clear_static_kw(ert.ecl_config)

        for keyword in listOfKeywords:
            ert.enkf.ecl_config_add_static_kw(ert.ecl_config, keyword)

    r.setter = add_static_kw

    internalPanel.endPage()

    # todo: add support for fixed length schedule keywords
    #internalPanel.startPage("Fixed length schedule keywords")
    #
    #r = internalPanel.addRow(KeywordList(widget, "", "add_fixed_length_schedule_kw"))
    #r.getter = lambda ert : ert.getAttribute("add_fixed_length_schedule_kw")
    #r.setter = lambda ert, value : ert.setAttribute("add_fixed_length_schedule_kw", value)
    #
    #internalPanel.endPage()

    configPanel.addRow(internalPanel)

    configPanel.endPage()
