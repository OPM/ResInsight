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

def createSimulationsPage(configPanel, parent):
    configPanel.startPage("Simulations")


    r = configPanel.addRow(IntegerSpinner(parent, "Max submit", "config/simulation/max_submit", 1, 10000))
    r.getter = lambda ert : ert.enkf.site_config_get_max_submit(ert.site_config)
    r.setter = lambda ert, value : ert.enkf.site_config_set_max_submit(ert.site_config, value)

    r = configPanel.addRow(IntegerSpinner(parent, "Max resample", "config/simulation/max_resample", 1, 10000))
    r.getter = lambda ert : ert.enkf.model_config_get_max_resample(ert.model_config)
    r.setter = lambda ert, value : ert.enkf.model_config_set_max_resample(ert.model_config, value)



    r = configPanel.addRow(ForwardModelPanel(parent))


    def get_forward_model(ert):
        site_config = ert.site_config
        installed_jobs_pointer = ert.enkf.site_config_get_installed_jobs(site_config)
        installed_jobs_stringlist_pointer = ert.job_queue.ext_joblist_alloc_list(installed_jobs_pointer)
        available_jobs = ert.getStringList(installed_jobs_stringlist_pointer , free_after_use=True)

        result = {'available_jobs': available_jobs}

        model_config = ert.model_config
        forward_model = ert.enkf.model_config_get_forward_model(model_config)
        name_string_list = ert.job_queue.forward_model_alloc_joblist(forward_model)
        job_names = ert.getStringList(name_string_list, free_after_use=True)

        forward_model_jobs = []

        count = 0
        for name in job_names:
            ext_job = ert.job_queue.forward_model_iget_job(forward_model, count)
            arg_string = ert.job_queue.ext_job_get_private_args_as_string(ext_job)
            help_text = ert.job_queue.ext_job_get_help_text(ext_job)
            forward_model_jobs.append((name, arg_string, help_text))
            count+=1

        result['forward_model'] = forward_model_jobs

        return result

    r.getter = get_forward_model

    def update_forward_model(ert, forward_model):
        forward_model_pointer = ert.enkf.model_config_get_forward_model(ert.model_config)
        ert.job_queue.forward_model_clear(forward_model_pointer)

        for job in forward_model:
            name = job[0]
            args = job[1]
            ext_job = ert.job_queue.forward_model_add_job(forward_model_pointer, name)
            ert.job_queue.ext_job_set_private_args_from_string(ext_job, args)

    r.setter = update_forward_model



    r = configPanel.addRow(PathChooser(parent, "Case table", "config/simulation/case_table"))


    def get_case_table(ert):
        return ert.enkf.model_config_get_case_table_file(ert.model_config)
    r.getter = get_case_table

    def set_case_table(ert, value):
        if os.path.exists(value):
            ert.enkf.enkf_main_set_case_table(ert.model_config, ert.nonify(value))
    r.setter = set_case_table

    
    r = configPanel.addRow(PathChooser(parent, "License path", "config/simulation/license_path"))
    r.getter = lambda ert : ert.enkf.site_config_get_license_root_path(ert.site_config)

    def ls(string):
        if string is None:
            return ""
        else:
            return string

    r.setter = lambda ert, value : ert.enkf.site_config_set_license_root_path(ert.site_config, ls(value))



    internalPanel = ConfigPanel(parent)

    internalPanel.startPage("Runpath")

    r = internalPanel.addRow(PathChooser(parent, "Runpath", "config/simulation/runpath", path_format=True))

    r.getter = lambda ert : ert.enkf.model_config_get_runpath_as_char(ert.model_config)
    r.setter = lambda ert, value : ert.enkf.model_config_set_runpath_fmt(ert.model_config, str(value))
    parent.connect(r, QtCore.SIGNAL("contentsChanged()"), lambda : r.modelEmit("runpathChanged()"))


    r = internalPanel.addRow(CheckBox(parent, "Pre clear", "config/simulation/pre_clear_runpath", "Perform pre clear"))

    r.getter = lambda ert : ert.enkf.enkf_main_get_pre_clear_runpath(ert.main)
    r.setter = lambda ert, value : ert.enkf.enkf_main_set_pre_clear_runpath(ert.main, value)


    r = internalPanel.addRow(RunpathMemberPanel(widgetLabel="Retain runpath", helpLabel="config/simulation/runpath_retain"))
    def get_runpath_retain_state(ert):
        ensemble_size = ert.enkf.enkf_main_get_ensemble_size(ert.main)

        result = []
        for index in range(ensemble_size):
            state = ert.enkf.enkf_main_iget_keep_runpath(ert.main, index)
            result.append((index, keep_runpath_type.resolveValue(state)))
            
        return result

    r.getter = get_runpath_retain_state

    def set_runpath_retain_state(ert, items):
        for item in items:
            ert.enkf.enkf_main_iset_keep_runpath(ert.main, item.member, item.runpath_state.value())

    r.setter = set_runpath_retain_state


    internalPanel.endPage()

    internalPanel.startPage("Run Template")

    r = internalPanel.addRow(RunTemplatePanel(parent))

    def get_run_templates(ert):
        templates = ert.enkf.enkf_main_get_templates(ert.main)
        template_list = ert.enkf.ert_template_alloc_list(templates)

        template_names = ert.getStringList(template_list, free_after_use=True)
        result = []
        for name in template_names:
            template = ert.enkf.ert_template_get_template(templates, name)
            template_file = ert.enkf.ert_template_get_template_file(template)
            target_file = ert.enkf.ert_template_get_target_file(template)
            arguments = ert.enkf.ert_template_get_args_as_string(template)
            result.append((name, template_file, target_file, arguments))
        return result

    r.getter = get_run_templates

    def set_run_templates(ert, template_list):
        templates_pointer = ert.enkf.enkf_main_get_templates(ert.main)
        ert.enkf.ert_template_clear(templates_pointer)

        for template in template_list:
            ert.enkf.ert_template_add_template(templates_pointer, template[0], template[1], template[2], template[3])

    r.setter = set_run_templates  
    
#    r = internalPanel.addRow(MultiColumnTable(parent, "", "run_template", ["Template", "Target file", "Arguments"]))
#    r.getter = lambda ert : ert.getAttribute("run_template")
#    r.setter = lambda ert, value : ert.setAttribute("run_template", value)

    internalPanel.endPage()
    configPanel.addRow(internalPanel)


    configPanel.endPage()
