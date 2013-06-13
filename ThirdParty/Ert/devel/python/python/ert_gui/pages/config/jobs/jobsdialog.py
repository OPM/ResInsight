#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'jobsdialog.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4 import QtGui, QtCore
from ert_gui.widgets.configpanel import ConfigPanel
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.tablewidgets import KeywordTable
from ert_gui.widgets.tablewidgets import KeywordList
from ert_gui.widgets.stringbox import StringBox
import os
from ert_gui.widgets.spinnerwidgets import IntegerSpinner
import ert_gui.widgets.util
from ert_gui.widgets.helpedwidget import ContentModelProxy

class EditJobDialog(QtGui.QDialog):
    """
    A panel for creating custom jobs.
    """
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent)
        self.setModal(True)
        self.setWindowTitle("Edit job")
        self.setMinimumWidth(650)

        layout = QtGui.QVBoxLayout()

        self.jobPanel = JobConfigPanel(parent)

        layout.addWidget(self.jobPanel)

        self.doneButton = QtGui.QPushButton("Done", self)
        self.cancelButton = QtGui.QPushButton("Cancel", self)
        self.connect(self.doneButton, QtCore.SIGNAL('clicked()'), self.saveJob)
        self.connect(self.cancelButton, QtCore.SIGNAL('clicked()'), self.reject)

        self.validationInfo = widgets.util.ValidationInfo()

        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addWidget(self.validationInfo)
        buttonLayout.addStretch(1)
        buttonLayout.addWidget(self.doneButton)
        buttonLayout.addWidget(self.cancelButton)

        layout.addSpacing(10)
        layout.addLayout(buttonLayout)

        self.setLayout(layout)


    def keyPressEvent(self, event):
        if not event.key() == QtCore.Qt.Key_Escape:
            QtGui.QDialog.keyPressEvent(self, event)

    def setJob(self, job):
        self.jobPanel.setJob(job)

    def saveJob(self):
        msg = self.jobPanel.saveJob()
        if msg is None:
            self.accept()
        else:
            self.validationInfo.setMessage(msg)


class JobConfigPanel(ConfigPanel):
    def __init__(self, parent=None):
        ConfigPanel.__init__(self, parent)

        self.initialized = False

        layout = QtGui.QFormLayout()
        layout.setLabelAlignment(QtCore.Qt.AlignRight)

        def jid(ert):
            """Returns the pointer to the current job (self.job)"""
            jl = ert.main.site_config.get_installed_jobs
            return jl.get_job(self.job.name)

        def set_stdin(ert,value):
            job = jid(ert)
            job.set_stdin_file(value)

        def get_stdin(ert):
            job = jid(ert)
            return job.get_stdin_file
        
        self.stdin = PathChooser(self, "", "config/systemenv/install_job_stdin", show_files=True, must_be_set=False, must_exist=True)
        self.stdin.setter = set_stdin
        self.stdin.getter = get_stdin
        self.stdin.initialize =get_stdin

        def set_stdout(ert,value):
            job = jid(ert)
            job.set_stdout_file(value)

        def get_stdout(ert):
            job = jid(ert)
            return job.get_stdout_file
        
        self.stdout = PathChooser(self, "", "config/systemenv/install_job_stdout", show_files=True, must_be_set=True, must_exist=False)
        self.stdout.setter = set_stdout
        self.stdout.getter = get_stdout
        self.stdout.initialize = get_stdout

        def set_stderr(ert,value):
            job = jid(ert)
            job.set_stderr_file(value)

        def get_stderr(ert):
            job = jid(ert)
            return job.get_stderr_file
        
        self.stderr = PathChooser(self, "", "config/systemenv/install_job_stderr", show_files=True, must_be_set=True, must_exist=False)
        self.stderr.setter = set_stderr
        self.stderr.getter = get_stderr
        self.stderr.initialize = get_stderr

        def set_target_file(ert,value):
            job = jid(ert)
            job.set_target_file(value)

        def get_target_file(ert):
            job = jid(ert)
            return job.get_target_file
        
        self.target_file = PathChooser(self, "", "config/systemenv/install_job_target_file", show_files=True, must_be_set=False, must_exist=False)
        self.target_file.setter = set_target_file
        self.target_file.getter = get_target_file
        self.target_file.initialize = get_target_file

        def set_executable(ert,value):
            job = jid(ert)
            job.set_executable(value)

        def get_executable(ert):
            job = jid(ert)
            return job.get_executable
        
        self.executable = PathChooser(self, "", "config/systemenv/install_job_executable", show_files=True, must_be_set=True,
                                      must_exist=True, is_executable_file=True)
        self.executable.setter = set_executable
        self.executable.getter = get_executable

        def setEnv(ert, value):
            job = jid(ert)
            job.clear_environment

            for env in value:
                job.set_environment(env[0], env[1])

        def getEnv(ert):
            job = jid(ert)
            return ert.getHash(job.get_environment)

        self.env = KeywordTable(self, "", "config/systemenv/install_job_env", colHead1="Variable", colHead2="Value")
        self.env.setter = setEnv
        self.env.getter = getEnv

        def set_arglist(ert,value):
            job = jid(ert)
            job.set_private_args_from_string(value)

        def get_arglist(ert):
            job = jid(ert)
            return job.get_private_args_as_string

        self.arglist = StringBox(self, "", "config/systemenv/install_job_arglist")
        self.arglist.setter = set_arglist
        self.arglist.getter = get_arglist

        def set_max_running(ert,value):
            job = jid(ert)
            job.set_max_running(value)

        def get_max_running(ert):
            job = jid(ert)
            return job.get_max_running
        
        self.max_running = IntegerSpinner(self, "", "config/systemenv/install_job_max_running", 0, 10000)
        self.max_running.setter = set_max_running
        self.max_running.getter = get_max_running

        def set_max_running_minutes(ert,value):
            job = jid(ert)
            job.set_max_running_minutes(value)

        def get_max_running(ert):
            job = jid(ert)
            return job.get_max_running_minutes
        
        self.max_running_minutes = IntegerSpinner(self, "", "config/systemenv/install_job_max_running_minutes", 0, 10000)
        self.max_running_minutes.setter = set_max_running_minutes
        self.max_running_minutes.getter = get_max_running_minutes


        self.startPage("Standard")
        self.add("Executable.:", self.executable)
        self.add("Stdout:", self.stdout)
        self.add("Stderr:", self.stderr)
        self.add("Target file:", self.target_file)
        self.add("Arglist.:", self.arglist)
        self.endPage()

        self.startPage("Advanced")
        self.add("Stdin:", self.stdin)
        self.add("Max running:", self.max_running)
        self.max_running.setInfo("(0=unlimited)")
        self.add("Max running minutes:", self.max_running_minutes)
        self.max_running_minutes.setInfo("(0=unlimited)")
        self.add("Env.:", self.env)
        self.endPage()

    def add(self, label, widget):
        self.addRow(widget, label)


    def setJob(self, job):
        self.job = job

        self.initialize(self.stdin.getModel())

        self.cmproxy = ContentModelProxy() #Since only the last change matters and no insert and remove is done
        self.cmproxy.proxify(self.stdin, self.stdout, self.stderr, self.target_file, self.executable,
                             self.env, self.arglist, 
                             self.max_running, self.max_running_minutes)

        self.stdin.fetchContent()
        self.stdout.fetchContent()
        self.stderr.fetchContent()
        self.target_file.fetchContent()
        self.executable.fetchContent()
        self.env.fetchContent()
        self.arglist.fetchContent()
        self.max_running.fetchContent()
        self.max_running_minutes.fetchContent()

    def saveJob(self):
        if self.executable.isValid() and self.stderr.isValid() and self.stdout.isValid():
            self.cmproxy.apply()

            ert = self.stdin.getModel()
            jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)
            jid = ert.job_queue.ext_joblist_get_job(jl, self.job.name)
            ert.job_queue.ext_job_save(jid)
            return None
        else:
            return "These fields are required: executable, stdout and stderr!"

