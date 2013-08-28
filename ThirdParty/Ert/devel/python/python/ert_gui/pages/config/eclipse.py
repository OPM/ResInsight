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
    r.initialize = lambda ert : ert.main.ecl_config.get_eclbase
    r.getter = lambda ert : ert.main.ecl_config.get_eclbase
    r.setter = lambda ert, value : ert.main.set_eclbase(str(value))

    r = configPanel.addRow(PathChooser(parent, "Data file", "config/eclipse/data_file", show_files=True))
    r.initialize = lambda ert : ert.main.ecl_config.get_data_file
    r.getter = lambda ert : ert.main.ecl_config.get_data_file
    r.setter = lambda ert, value : ert.main.set_datafile(str(value))

    r = configPanel.addRow(PathChooser(parent, "Grid", "config/eclipse/grid", show_files=True))
    r.initialize = lambda ert : ert.main.ecl_config.get_gridfile
    r.getter = lambda ert : ert.main.ecl_config.get_gridfile
    r.setter = lambda ert, value : ert.main.ecl_config.set_gridfile(str(value))

    r = configPanel.addRow(PathChooser(parent, "Schedule file" , "config/eclipse/schedule_file" , show_files = True))
    r.initialize = lambda ert : ert.main.ecl_config.get_schedule_file
    r.getter = lambda ert : ert.main.ecl_config.get_schedule_file
    r.setter = lambda ert, value : ert.main.ecl_config.set_schedule_file( str(value))


    r = configPanel.addRow(PathChooser(parent, "Init section", "config/eclipse/init_section", show_files=True))
    r.initialize = lambda ert : ert.main.ecl_config.get_init_section
    r.getter = lambda ert : ert.main.ecl_config.get_init_section
    r.setter = lambda ert, value : ert.main.ecl_config.set_init_section( str(value))


    r = configPanel.addRow(PathChooser(parent, "Refcase", "config/eclipse/refcase", show_files=True))
    r.initialize = lambda ert : ert.main.ecl_config.get_refcase_name
    r.getter = lambda ert : ert.main.ecl_config.get_refcase_name
    r.setter = lambda ert, value : ert.main.ecl_config.load_refcase( str(value))

    r = configPanel.addRow(PathChooser(parent, "Schedule prediction file", "config/eclipse/schedule_prediction_file", show_files=True))
    r.initialize = lambda ert : ert.main.get_schedule_prediction_file
    r.getter = lambda ert : ert.main.get_schedule_prediction_file
    r.setter = lambda ert, value : ert.main.set_schedule_prediction_file( ert.nonify( value ))

    r = configPanel.addRow(KeywordTable(parent, "Data keywords", "config/eclipse/data_kw"))
    r.getter = lambda ert : ert.getSubstitutionList(ert.main.get_data_kw)

    def add_data_kw(ert, listOfKeywords):
        ert.main.clear_data_kw

        for keyword in listOfKeywords:
            ert.main.add_data_kw( keyword[0], keyword[1])

    r.setter = add_data_kw

    r.initialize = lambda ert : ert.getSubstitutionList(ert.main.get_data_kw)

    configPanel.addSeparator()

    internalPanel = ConfigPanel(parent)

    internalPanel.startPage("Static keywords")

    r = internalPanel.addRow(KeywordList(parent, "", "config/eclipse/add_static_kw"))
    r.getter = lambda ert : ert.main.ecl_config.get_static_kw_list

    def add_static_kw(ert, listOfKeywords):
        ert.main.ecl_config.clear_static_kw

        for keyword in listOfKeywords:
            ert.main.ecl_config.add_static_kw(keyword)

    r.setter = add_static_kw
    r.initialize = lambda ert : ert.main.ecl_config.get_static_kw_list
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
