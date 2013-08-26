#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'simulation.py' is part of ERT - Ensemble based Reservoir Tool. 
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
# Simulations tab
# ----------------------------------------------------------------------------------------------
from PyQt4 import QtCore
from ert_gui.widgets.spinnerwidgets import IntegerSpinner
from ert_gui.widgets.tablewidgets import KeywordTable
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.checkbox import CheckBox
from ert_gui.widgets.configpanel import ConfigPanel
from ert_gui.widgets.stringbox import StringBox
from jobs.forwardmodelpanel import ForwardModelPanel
from simulations.runpathpanel import RunpathMemberList, RunpathMemberPanel
from ert.ert.enums import keep_runpath_type
from simulations.runtemplatepanel import RunTemplatePanel
import ert_gui.widgets.helpedwidget
import os
from ert.util.stringlist import StringList

def createSimulationsPage(configPanel, parent):
    configPanel.startPage("Simulations")


    r = configPanel.addRow(IntegerSpinner(parent, "Max submit", "config/simulation/max_submit", 1, 10000))
    r.initialize = lambda ert : ert.main.site_config.get_max_submit
    r.getter = lambda ert : ert.main.site_config.get_max_submit
    r.setter = lambda ert, value : ert.main.site_config.set_max_submit( value)

    r = configPanel.addRow(IntegerSpinner(parent, "Max internal submit", "config/simulation/max_resample", 1, 10000))
    r.initialize = lambda ert : ert.main.model_config.get_max_internal_submit
    r.getter = lambda ert : ert.main.model_config.get_max_internal_submit
    r.setter = lambda ert, value : ert.main.model_config.set_max_internal_submit( value)



    r = configPanel.addRow(ForwardModelPanel(parent))


    def get_forward_model(ert):
        site_config = ert.main.site_config
        installed_jobs = ert.main.site_config.get_installed_jobs
        available_jobs = installed_jobs.alloc_list

        result = {'available_jobs': available_jobs}

        model_config = ert.main.model_config
        forward_model = model_config.get_forward_model
        job_names = forward_model.alloc_joblist
        
        forward_model_jobs = []

        count = 0
        for name in job_names:
            ext_job    = forward_model.iget_job( count)
            arg_string = ext_job.get_private_args_as_string
            help_text  = ext_job.get_help_text
            forward_model_jobs.append((name, arg_string, help_text))
            count+=1

        result['forward_model'] = forward_model_jobs

        return result

    r.getter = get_forward_model
    r.initialize = get_forward_model

    def update_forward_model(ert, forward_model):
        forward_model_object = ert.main.model_config.get_forward_model
        forward_model_object.clear

        for job in forward_model:
            name = job[0]
            args = job[1]
            ext_job = forward_model.add_job( name)
            ext_job.set_private_args_from_string( args)

    r.setter = update_forward_model



    r = configPanel.addRow(PathChooser(parent, "Case table", "config/simulation/case_table"))


    def get_case_table(ert):
        return ert.main.model_config.get_case_table_file
    r.getter = get_case_table
    r.initialize = get_case_table
    
    def set_case_table(ert, value):
        if os.path.exists(value):
            ert.main.set_case_table( ert.nonify(value))
    r.setter = set_case_table

    
    r = configPanel.addRow(PathChooser(parent, "License path", "config/simulation/license_path"))
    r.getter = lambda ert : ert.main.site_config.get_license_root_path
    r.initialize = lambda ert : ert.main.site_config.get_license_root_path

    def ls(string):
        if string is None:
            return ""
        else:
            return string

    r.setter = lambda ert, value : ert.main.site_config.set_license_root_path( ls(value))



    internalPanel = ConfigPanel(parent)

    internalPanel.startPage("Runpath")

    r = internalPanel.addRow(PathChooser(parent, "Runpath", "config/simulation/runpath", path_format=True))

    r.getter = lambda ert : ert.main.model_config.get_runpath_as_char
    r.initialize = lambda ert : ert.main.model_config.get_runpath_as_char
    r.setter = lambda ert, value : ert.main.model_config.select_runpath( str(value))
    parent.connect(r, QtCore.SIGNAL("contentsChanged()"), lambda : r.modelEmit("runpathChanged()"))


    r = internalPanel.addRow(CheckBox(parent, "Pre clear", "config/simulation/pre_clear_runpath", "Perform pre clear"))

    r.getter = lambda ert : ert.main.get_pre_clear_runpath
    r.initialize = lambda ert : ert.main.get_pre_clear_runpath
    r.setter = lambda ert, value : ert.main.set_pre_clear_runpath( value)


    r = internalPanel.addRow(RunpathMemberPanel(widgetLabel="Retain runpath", helpLabel="config/simulation/runpath_retain"))
    def get_runpath_retain_state(ert):
        ensemble_size = ert.main.ens_size

        result = []
        for index in range(ensemble_size):
            state = ert.main.iget_keep_runpath( index)
            result.append((index, keep_runpath_type.resolveValue(state)))
            
        return result

    r.getter = get_runpath_retain_state
    r.initialize = get_runpath_retain_state
    
    def set_runpath_retain_state(ert, items):
        for item in items:
            ert.main.iset_keep_runpath(item.member, item.runpath_state.value())

    r.setter = set_runpath_retain_state


    internalPanel.endPage()

    internalPanel.startPage("Run Template")

    r = internalPanel.addRow(RunTemplatePanel(parent))

    def get_run_templates(ert):
        templates = ert.main.get_templates
        template_names = templates.alloc_list
        
        result = []
        for name in template_names:
            template = templates.get_template(name)
            template_file = template.get_template_file
            target_file = template.get_target_file
            arguments = template.get_args_as_string
            result.append((name, template_file, target_file, arguments))
        return result

    r.getter = get_run_templates
    r.initialize = get_run_templates

    def set_run_templates(ert, template_list):
        templates_object = ert.main.get_templates
        templates_object.clear

        for template in template_list:
            templates_object.add_template( template[0], template[1], template[2], template[3])

    r.setter = set_run_templates  
    
#    r = internalPanel.addRow(MultiColumnTable(parent, "", "run_template", ["Template", "Target file", "Arguments"]))
#    r.getter = lambda ert : ert.getAttribute("run_template")
#    r.setter = lambda ert, value : ert.setAttribute("run_template", value)

    internalPanel.endPage()
    configPanel.addRow(internalPanel)


    configPanel.endPage()
