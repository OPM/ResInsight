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
from ert.job_queue.ext_job import ExtJob

def createSystemPage(configPanel, parent):
    configPanel.startPage("System")

    # Should call enkf_main_get_site_config_file() to get the name of
    # the site configuration file; this should only be a label - not
    # user editable.
    r = configPanel.addRow(ActiveLabel(None, "Site Config", "", "Not specified."))
    r.initialize = lambda ert : ert.main.get_site_config_file
    r.getter = lambda ert : ert.main.get_site_config_file
    r.modelConnect("casesUpdated()", r.fetchContent)
    


    r = configPanel.addRow(PathChooser(parent, "Job script", "config/systemenv/job_script", True))
    r.initialize = lambda ert : ert.main.site_config.get_job_script
    r.getter = lambda ert : ert.main.site_config.get_job_script
    r.setter = lambda ert, value : ert.main.site_config.set_job_script( str(value))

    internalPanel = ConfigPanel(parent)
    internalPanel.startPage("setenv")

    r = internalPanel.addRow(KeywordTable(parent, "", "config/systemenv/setenv"))
    r.initialize = lambda ert : ert.getHash(ert.main.site_config.get_env_hash)
    r.getter = lambda ert : ert.getHash(ert.main.site_config.get_env_hash)

    def setenv(ert, value):
        ert.main.site_config.clear_env
        for env in value:
            ert.main.site_config.setenv( env[0], env[1])

    r.setter = setenv

    internalPanel.endPage()

    internalPanel.startPage("Update path")

    r = internalPanel.addRow(KeywordTable(parent, "", "config/systemenv/update_path"))

    def get_update_path(ert):
        paths = ert.main.site_config.get_path_variables
        values =  ert.main.site_config.get_path_values

        return [[p, v] for p, v in zip(paths, values)]

    r.getter = get_update_path
    r.initialize = get_update_path
    
    def update_pathvar(ert, value):
        ert.main.site_config.clear_pathvar

        for pathvar in value:
            ert.main.site_config.update_pathvar( pathvar[0], pathvar[1])

    r.setter = update_pathvar

    internalPanel.endPage()


    internalPanel.startPage("Jobs")

    r = internalPanel.addRow(JobsPanel(parent))

    def get_jobs(ert):
        jl = ert.main.site_config.get_installed_jobs
        h  = jl.get_jobs
        stringlist = jl.alloc_list
        jobs = ert.getHash(h, return_type="c_void_p")

        private_jobs = []
        for v in stringlist:
            job = jl.get_job(v)
            path = job.get_config_file
            if job.is_private:
                private_jobs.append(Job(v, path))
        #for k, v in jobs:
        #    job = jl.get_job(v)
        #    path = job.get_config_file
        #    if v.is_private:
        #        private_jobs.append(Job(k, path))

        return private_jobs

    def update_job(ert, value):
        jl = ert.main.site_config.get_installed_jobs

        if os.path.exists(value.path):
            license = ert.main.site_config.get_license_root_path
            job = ert.job_queue.ext_job_fscanf_alloc(value.name, license, True, value.path)
            jl.add_job(value.name, job)
        else:
            job = jl.get_job(value.name)
            job.set_config_file(value.path)


    def add_job(ert, value):
        jl = ert.main.site_config.get_installed_jobs
        if not jl.has_job(value.name):
            license = ert.main.site_config.get_license_root_path
            if os.path.exists(value.path):
                job = ert.job_queue.ext_job_fscanf_alloc(value.name, license, True, value.path)
                jl.add_job(value.name, job)
            else:
                job = ert.job_queue.ext_job_alloc(value.name, license, True)
                job.set_config_file(value.path)
                jl.add_job(value.name, job)
            return True

        return False

    def remove_job(ert, value):
        jl = ert.main.site_config.get_installed_jobs
        success = jl.del_job(value.name)

        if not success:
            QtGui.QMessageBox.question(parent, "Failed", "Unable to delete job!", QtGui.QMessageBox.Ok)
            return False
        return True

    
    r.getter = get_jobs
    r.initialize = get_jobs
    r.setter = update_job
    r.insert = add_job
    r.remove = remove_job



    internalPanel.endPage()
    configPanel.addRow(internalPanel)

    r = configPanel.addRow(PathChooser(parent, "Log file", "config/systemenv/log_file", True))
    r.initialize = lambda ert: ert.main.logh.get_filename
    r.getter = lambda ert : ert.main.logh.get_filename
    r.setter = lambda ert, value : ert.main.logh.reopen(value)

    r = configPanel.addRow(ert_gui.widgets.spinnerwidgets.IntegerSpinner(parent, "Log level", "config/systemenv/log_level", 0, 1000))
    r.initialize = lambda ert : ert.main.logh.get_level
    r.getter = lambda ert : ert.main.logh.get_level
    r.setter = lambda ert, value : ert.main.logh.set_level( value)

    configPanel.endPage()





