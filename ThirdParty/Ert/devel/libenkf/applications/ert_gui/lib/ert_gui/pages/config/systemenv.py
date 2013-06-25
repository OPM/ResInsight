#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'systemenv.py' is part of ERT - Ensemble based Reservoir Tool. 
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


#----------------------------------------------------------------------------------------------
# System tab
# ----------------------------------------------------------------------------------------------
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.configpanel import ConfigPanel
from ert_gui.widgets.tablewidgets import KeywordTable, KeywordList
from PyQt4 import QtGui, QtCore
from jobs.jobspanel import JobsPanel, Job
import os
import ert_gui.widgets.spinnerwidgets
from ert_gui.widgets.activelabel import ActiveLabel

def createSystemPage(configPanel, parent):
    configPanel.startPage("System")

    # Should call enkf_main_get_site_config_file() to get the name of
    # the site configuration file; this should only be a label - not
    # user editable.
    r = configPanel.addRow(ActiveLabel(None, "Site Config", "", "Not specified."))
    r.getter = lambda ert : ert.enkf.enkf_main_get_site_config_file(ert.main)
    r.modelConnect("casesUpdated()", r.fetchContent)
    


    r = configPanel.addRow(PathChooser(parent, "Job script", "config/systemenv/job_script", True))
    r.getter = lambda ert : ert.enkf.site_config_get_job_script(ert.site_config)
    r.setter = lambda ert, value : ert.enkf.site_config_set_job_script(ert.site_config, str(value))

    internalPanel = ConfigPanel(parent)
    internalPanel.startPage("setenv")

    r = internalPanel.addRow(KeywordTable(parent, "", "config/systemenv/setenv"))
    r.getter = lambda ert : ert.getHash(ert.enkf.site_config_get_env_hash(ert.site_config))

    def setenv(ert, value):
        ert.enkf.site_config_clear_env(ert.site_config)
        for env in value:
            ert.enkf.site_config_setenv(ert.site_config, env[0], env[1])

    r.setter = setenv

    internalPanel.endPage()

    internalPanel.startPage("Update path")

    r = internalPanel.addRow(KeywordTable(parent, "", "config/systemenv/update_path"))
    def get_update_path(ert):
        paths = ert.getStringList(ert.enkf.site_config_get_path_variables(ert.site_config))
        values =  ert.getStringList(ert.enkf.site_config_get_path_values(ert.site_config))

        return [[p, v] for p, v in zip(paths, values)]

    r.getter = get_update_path

    def update_pathvar(ert, value):
        ert.enkf.site_config_clear_pathvar(ert.site_config)

        for pathvar in value:
            ert.enkf.site_config_update_pathvar(ert.site_config, pathvar[0], pathvar[1])

    r.setter = update_pathvar

    internalPanel.endPage()


    internalPanel.startPage("Jobs")

    r = internalPanel.addRow(JobsPanel(parent))
    def get_jobs(ert):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)
        h  = ert.job_queue.ext_joblist_get_jobs(jl)

        jobs = ert.getHash(h, return_type="long")

        private_jobs = []
        for k, v in jobs:
            v = int(v)
            path = ert.job_queue.ext_job_get_config_file(v)
            if ert.job_queue.ext_job_is_private(v):
                private_jobs.append(Job(k, path))

        return private_jobs

    def update_job(ert, value):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)

        if os.path.exists(value.path):
            license = ert.enkf.site_config_get_license_root_path(ert.site_config)
            job = ert.job_queue.ext_job_fscanf_alloc(value.name, license, True, value.path)
            ert.job_queue.ext_joblist_add_job(jl, value.name, job)
        else:
            job = ert.job_queue.ext_joblist_get_job(jl, value.name)
            ert.job_queue.ext_job_set_config_file(job, value.path)


    def add_job(ert, value):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)
        if not ert.job_queue.ext_joblist_has_job(jl, value.name):
            license = ert.enkf.site_config_get_license_root_path(ert.site_config)
            if os.path.exists(value.path):
                job = ert.job_queue.ext_job_fscanf_alloc(value.name, license, True, value.path)
                ert.job_queue.ext_joblist_add_job(jl, value.name, job)
            else:
                job = ert.job_queue.ext_job_alloc(value.name, license, True)
                ert.job_queue.ext_job_set_config_file(job, value.path)
                ert.job_queue.ext_joblist_add_job(jl, value.name, job)
            return True

        return False

    def remove_job(ert, value):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)
        success = ert.job_queue.ext_joblist_del_job(jl, value.name)

        if not success:
            QtGui.QMessageBox.question(parent, "Failed", "Unable to delete job!", QtGui.QMessageBox.Ok)
            return False
        return True


    r.getter = get_jobs
    r.setter = update_job
    r.insert = add_job
    r.remove = remove_job



    internalPanel.endPage()
    configPanel.addRow(internalPanel)

    r = configPanel.addRow(PathChooser(parent, "Log file", "config/systemenv/log_file", True))
    r.getter = lambda ert : ert.util.log_get_filename(ert.logh)
    r.setter = lambda ert, value : ert.util.log_reset_filename(ert.logh, value)

    r = configPanel.addRow(ert_gui.widgets.spinnerwidgets.IntegerSpinner(parent, "Log level", "config/systemenv/log_level", 0, 1000))
    r.getter = lambda ert : ert.util.log_get_level(ert.logh)
    r.setter = lambda ert, value : ert.util.log_set_level(ert.logh, value)

    configPanel.endPage()





