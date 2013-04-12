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


from __future__ import division
from PyQt4 import QtGui, QtCore
from ert_gui.widgets.util import resourceIcon, resourceStateIcon, shortTime
import time
from ert.ert.enums import ert_job_status_type


class SimulationList(QtGui.QListWidget):
    """A list widget with custom items representing simulation jobs"""
    def __init__(self):
        QtGui.QListWidget.__init__(self)

        self.setViewMode(QtGui.QListView.IconMode)
        self.setMovement(QtGui.QListView.Static)
        self.setResizeMode(QtGui.QListView.Adjust)

        self.setItemDelegate(SimulationItemDelegate())
        self.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        self.setSelectionRectVisible(False)

        self.setSortingEnabled(True)
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)

        
class SimulationItem(QtGui.QListWidgetItem):
    """Items for the custom SimulationList"""
    def __init__(self, simulation):
        self.simulation = simulation
        QtGui.QListWidgetItem.__init__(self, type=9901)
        self.setData(QtCore.Qt.DisplayRole, self.simulation)

    def __ge__(self, other):
        return self.simulation >= other.simulation

    def __lt__(self, other):
        return not self >= other


class SimulationItemDelegate(QtGui.QStyledItemDelegate):
    """The delegate that renders the custom SimulationListItems"""
    waiting = QtGui.QColor(164 , 164 , 255)
    pending = QtGui.QColor(164 , 200 , 255)
    running = QtGui.QColor(200 , 255 , 200)
    failed  = QtGui.QColor(255 , 200 , 200)
    
    userkilled = QtGui.QColor(255, 255, 200)
    finished   = QtGui.QColor(200, 200, 200)
    notactive  = QtGui.QColor(255, 255, 255)
    unknown    = running    # Loading + the intermidate states.

    #Cool orange color: QtGui.QColor(255, 200, 128)   

    size = QtCore.QSize(32, 18)

    def __init__(self):
        QtGui.QStyledItemDelegate.__init__(self)

    def paint(self, painter, option, index):
        """Renders the item"""
        painter.save()
        painter.setRenderHint(QtGui.QPainter.Antialiasing)

        data = index.data(QtCore.Qt.DisplayRole)

        if data is None:
            data = Simulation("0")
            data.status = 0
        else:
            data = data.toPyObject()

        if data.isWaiting():
            color = self.waiting
        elif data.isPending():
            color = self.pending
        elif data.isRunning():
            color = self.running
        elif data.finishedSuccessfully():
            color = self.finished
        elif data.hasFailed():
            color = self.failed
        elif data.notActive():
            color = self.notactive
        elif data.isUserKilled():
            color = self.userkilled
        else:
            color = self.unknown

        painter.setPen(color)
        rect = QtCore.QRect(option.rect)
        rect.setX(rect.x() + 1)
        rect.setY(rect.y() + 1)
        rect.setWidth(rect.width() - 2)
        rect.setHeight(rect.height() - 2)
        painter.fillRect(rect, color)

        painter.setPen(QtCore.Qt.black)

        painter.setRenderHint(QtGui.QPainter.Antialiasing, False)
        painter.drawRect(rect)

        if option.state & QtGui.QStyle.State_Selected:
            painter.fillRect(option.rect, QtGui.QColor(255, 255, 255, 150))

        painter.drawText(rect, QtCore.Qt.AlignCenter + QtCore.Qt.AlignVCenter, str(data.name))

        painter.restore()

    def sizeHint(self, option, index):
        """Returns the size of the item"""
        return self.size


class SimulationPanel(QtGui.QStackedWidget):
    """
    A panel that shows information and enables interaction with jobs.
    Three different views: no selected jobs, one selected job and multiple selected jobs.
    """

    def __init__(self, parent=None):
        QtGui.QStackedWidget.__init__(self, parent)
        self.setFrameShape(QtGui.QFrame.Panel)
        self.setFrameShadow(QtGui.QFrame.Raised)

        self.setMinimumWidth(200)
        self.setMaximumWidth(200)

        self.ctrl = SimulationPanelController(self)

        self.createNoSelectionsPanel()
        self.createSingleSelectionsPanel()
        self.createManySelectionsPanel()

        self.addWidget(self.noSimulationsPanel)
        self.addWidget(self.singleSimulationsPanel)
        self.addWidget(self.manySimulationsPanel)

        
    def createButtons(self):
        """Create kill, restart and resample and restart buttons"""
        self.killButton = QtGui.QToolButton(self)
        self.killButton.setIcon(resourceIcon("cross"))
        self.killButton.setToolTip("Kill job")
        self.connect(self.killButton, QtCore.SIGNAL('clicked()'), self.ctrl.kill)

        self.restartButton = QtGui.QToolButton(self)
        self.restartButton.setIcon(resourceIcon("refresh"))
        self.restartButton.setToolTip("Restart job")
        self.connect(self.restartButton, QtCore.SIGNAL('clicked()'), lambda : self.ctrl.restart(False))

        self.rrButton = QtGui.QToolButton(self)
        self.rrButton.setIcon(resourceIcon("refresh_resample"))
        self.rrButton.setToolTip("Resample and restart job")
        self.connect(self.rrButton, QtCore.SIGNAL('clicked()'), lambda : self.ctrl.restart(True))

        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addWidget(self.killButton)
        buttonLayout.addWidget(self.restartButton)
        buttonLayout.addWidget(self.rrButton)

        return buttonLayout

    def createButtonedLayout(self, layout, prestretch=True):
        """A centered layout for buttons"""
        btnlayout = QtGui.QVBoxLayout()
        btnlayout.addLayout(layout)

        if prestretch:
            btnlayout.addStretch(1)

        btnlayout.addLayout(self.createButtons())
        return btnlayout


    def createManySelectionsPanel(self):
        """The panel for multiple selected jobs"""
        self.manySimulationsPanel = QtGui.QWidget()

        layout = QtGui.QVBoxLayout()
        label = QtGui.QLabel("Selected jobs:")
        label.setAlignment(QtCore.Qt.AlignHCenter)
        layout.addWidget(label)

        self.selectedSimulationsLabel = QtGui.QLabel()
        self.selectedSimulationsLabel.setWordWrap(True)
        font = self.selectedSimulationsLabel.font()
        font.setWeight(QtGui.QFont.Bold)
        self.selectedSimulationsLabel.setFont(font)

        scrolledLabel = QtGui.QScrollArea()
        scrolledLabel.setWidget(self.selectedSimulationsLabel)
        scrolledLabel.setWidgetResizable(True)
        layout.addWidget(scrolledLabel)

        self.manySimulationsPanel.setLayout(self.createButtonedLayout(layout, False))

    def createSingleSelectionsPanel(self):
        """The panel for a single selected job"""
        self.singleSimulationsPanel = QtGui.QWidget()

        layout = QtGui.QFormLayout()
        layout.setLabelAlignment(QtCore.Qt.AlignRight)
        self.jobLabel = QtGui.QLabel()
        self.submitLabel = QtGui.QLabel()
        self.startLabel = QtGui.QLabel()
        self.finishLabel = QtGui.QLabel()
        self.waitingLabel = QtGui.QLabel()
        self.runningLabel = QtGui.QLabel()
        self.stateLabel = QtGui.QLabel()

        layout.addRow("Job #:", self.jobLabel)
        layout.addRow("Submitted:", self.submitLabel)
        layout.addRow("Started:", self.startLabel)
        layout.addRow("Finished:", self.finishLabel)
        layout.addRow("Waiting:", self.runningLabel)
        layout.addRow("Running:", self.waitingLabel)
        layout.addRow("State:", self.stateLabel)

        self.singleSimulationsPanel.setLayout(self.createButtonedLayout(layout))


    def createNoSelectionsPanel(self):
        """The panel for no selected jobs. Enables pausing and killing the entire simulation"""
        self.noSimulationsPanel = QtGui.QWidget()

        layout = QtGui.QVBoxLayout()
        label = QtGui.QLabel("Pause queue after currently running jobs are finished:")
        label.setWordWrap(True)
        layout.addWidget(label)

        self.pauseButton = QtGui.QToolButton(self)
        self.pauseButton.setIcon(resourceIcon("pause"))
        self.pauseButton.setCheckable(True)
        self.connect(self.pauseButton, QtCore.SIGNAL('clicked()'), lambda : self.ctrl.pause(self.pauseButton.isChecked()))


        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addStretch(1)
        buttonLayout.addWidget(self.pauseButton)
        buttonLayout.addStretch(1)
        layout.addLayout(buttonLayout)

        label = QtGui.QLabel("Remove all jobs from the queue:")
        label.setWordWrap(True)
        layout.addWidget(label)

        self.killAllButton = QtGui.QToolButton(self)
        self.killAllButton.setIcon(resourceIcon("cancel"))
        self.connect(self.killAllButton, QtCore.SIGNAL('clicked()'), self.ctrl.killAll)

        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addStretch(1)
        buttonLayout.addWidget(self.killAllButton)
        buttonLayout.addStretch(1)

        layout.addLayout(buttonLayout)

        self.noSimulationsPanel.setLayout(layout)



    def setSimulations(self, selection=None):
        """Set the list of selected jobs"""
        if selection is None: selection = []
        self.ctrl.setSimulations(selection)

#    def markText(self, a, b):
#        if b.isRunning():
#            c = SimulationItemDelegate.running
#        elif b.isWaiting():
#            c = SimulationItemDelegate.waiting
#        else:
#            c = QtGui.QColor(255, 255, 255, 0)
#
#        color = "rgb(%d, %d, %d)" % (c.red(), c.green(), c.blue())
#
#        b = "<span style='background: " + color + ";'>" + str(b) + "</span>"
#
#        if not a == "":
#            return a + " " + b
#        else:
#            return b

    def setModel(self, ert):
        """Set the reference to ERT (ertwrapper instance)"""
        self.ctrl.setModel(ert)

    def setSimulationStatistics(self, statistics):
        """Set the associated simulation statistics"""
        self.ctrl.setSimulationStatistics(statistics)


class SimulationPanelController:
    """Controller code for the simulation panel"""
    def __init__(self, view):
        self.view = view
        self.initialized = False
        self.selectedSimulations = []
        self.view.connect(self.view, QtCore.SIGNAL('simulationsUpdated()'), self.showSelectedSimulations)

    def initialize(self, ert):
        """Set prototypes for ERT"""
        if not self.initialized:
            ert.prototype("bool job_queue_get_pause(long)", lib = ert.job_queue)
            ert.prototype("void job_queue_set_pause_on(long)", lib = ert.job_queue)
            ert.prototype("void job_queue_set_pause_off(long)", lib = ert.job_queue)
            ert.prototype("void job_queue_user_exit(long)", lib = ert.job_queue)
            ert.prototype("long enkf_main_iget_state(long, int)")
            ert.prototype("void enkf_state_kill_simulation(long)")
            ert.prototype("void enkf_state_resubmit_simulation(long, int)")
            ert.prototype("int enkf_state_get_run_status(long)")
            ert.prototype("long site_config_get_job_queue(long)")
            self.initialized = True

    def setModel(self, ert):
        """Set the reference to ERT (ertwrapper instance)"""
        self.initialize(ert)
        self.ert = ert

    def kill(self):
        """Kills the selected simulations."""
        for simulation in self.selectedSimulations:
            state = self.ert.enkf.enkf_main_iget_state(self.ert.main, simulation.name)
            status = self.ert.enkf.enkf_state_get_run_status(state)

            #if status == Simulation.RUNNING:
            self.ert.enkf.enkf_state_kill_simulation(state)

    def restart(self, resample):
        """Restarts the selected simulations. May also resample."""
        for simulation in self.selectedSimulations:
            state = self.ert.enkf.enkf_main_iget_state(self.ert.main, simulation.name)
            status = self.ert.enkf.enkf_state_get_run_status(state)

            #if status == Simulation.USER_KILLED:
            self.ert.enkf.enkf_state_resubmit_simulation(state , resample)

    def pause(self, pause):
        """Pause the job queue after the currently running jobs are finished."""
        job_queue = self.ert.enkf.site_config_get_job_queue(self.ert.site_config)

        if pause:
            self.statistics.stop()
            self.ert.job_queue.job_queue_set_pause_on(job_queue)
        else:
            self.statistics.startTiming()
            self.ert.job_queue.job_queue_set_pause_off(job_queue)

    def killAll(self):
        """Kills all simulations"""
        killAll = QtGui.QMessageBox.question(self.view, "Remove all jobs?", "Are you sure you want to remove all jobs from the queue?", QtGui.QMessageBox.Yes | QtGui.QMessageBox.No)

        if killAll == QtGui.QMessageBox.Yes:
            job_queue = self.ert.enkf.site_config_get_job_queue(self.ert.site_config)
            self.ert.job_queue.job_queue_user_exit(job_queue)

    def showSelectedSimulations(self):
        """Update information relating to a single job"""
        if len(self.selectedSimulations) >= 2:
            members = reduce(lambda a, b: str(a) + " " + str(b), sorted(self.selectedSimulations))
            self.view.selectedSimulationsLabel.setText(members)
        elif len(self.selectedSimulations) == 1:
            sim = self.selectedSimulations[0]
            self.view.jobLabel.setText(str(sim.name))
            self.view.submitLabel.setText(shortTime(sim.submitTime))
            self.view.startLabel.setText(shortTime(sim.startTime))
            self.view.finishLabel.setText(shortTime(sim.finishedTime))

            if sim.startTime == -1:
                runningTime = "-"
            elif sim.finishedTime > -1:
                runningTime = sim.finishedTime - sim.startTime
            else:
                runningTime = int(time.time()) - sim.startTime


            if sim.submitTime == -1:
                waitingTime = "-"
            elif sim.startTime > -1:
                waitingTime = sim.startTime - sim.submitTime
            else:
                waitingTime = int(time.time()) - sim.submitTime

            self.view.runningLabel.setText(str(waitingTime) + " secs")
            self.view.waitingLabel.setText(str(runningTime) + " secs")

            status = sim.status.name[10:]
            self.view.stateLabel.setText(status)


    def setSimulations(self, selection=None):
        """Change the view according to the selection. No, single or multiple jobs."""
        if selection is None: selection = []
        self.selectedSimulations = selection

        if len(selection) >= 2:
            self.view.setCurrentWidget(self.view.manySimulationsPanel)
        elif len(selection) == 1:
            self.view.setCurrentWidget(self.view.singleSimulationsPanel)
        else:
            self.view.setCurrentWidget(self.view.noSimulationsPanel)

        self.showSelectedSimulations()

    def setSimulationStatistics(self, statistics):
        """Set the associated statistics"""
        self.statistics = statistics


class Simulation:
    """Container for state information for a single simulation."""

    def __init__(self, name, statistics=None):
        self.name = name
        self.status = ert_job_status_type.NOT_ACTIVE
        self.statuslog = []
        self.statistics = statistics

        self.resetTime()

    def checkStatus(self, type):
        """Check the internal status against an ERT enum"""
        return self.status == type

    def isWaiting(self):
        """Is the job waiting?"""
        return self.checkStatus(ert_job_status_type.WAITING) or self.checkStatus(ert_job_status_type.SUBMITTED)

    def isPending(self):
        """Is the job PENDING in the LSF queue?"""
        return self.checkStatus(ert_job_status_type.PENDING)

    def isRunning(self):
        """Is the job running?"""
        return self.checkStatus(ert_job_status_type.RUNNING)

    def hasFailed(self):
        """Has the job failed?"""
        return self.checkStatus(ert_job_status_type.ALL_FAIL)

    def notActive(self):
        """Is the job active?"""
        return self.checkStatus(ert_job_status_type.NOT_ACTIVE)

    def finishedSuccessfully(self):
        """Has  the job finished?"""
        return self.checkStatus(ert_job_status_type.ALL_OK)

    def isUserKilled(self):
        """Has the job been killed by the user?"""
        return self.checkStatus(ert_job_status_type.USER_KILLED) or self.checkStatus(ert_job_status_type.USER_EXIT)


    def setStatus(self, status):
        """Update the status of this job"""
        if len(self.statuslog) == 0 or not self.statuslog[len(self.statuslog) - 1] == status:
            self.statuslog.append(status)

            if status == ert_job_status_type.ALL_OK:
                self.setFinishedTime(int(time.time()))

        self.status = status

    def setStartTime(self, secs):
        """Set the time the job started"""
        self.startTime = secs

    def setSubmitTime(self, secs):
        """Set the time the job was submitted to LSF"""
        self.submitTime = secs
        if self.submitTime > self.finishedTime:
            self.finishedTime = -1

    def setFinishedTime(self, secs):
        """Set the time the job finished"""
        self.finishedTime = secs
        
        if not self.statistics is None:
            self.statistics.addTime(self.submitTime, self.startTime, self.finishedTime)

    def printTime(self, secs):        
        if not secs == -1:
            print time.localtime(secs)

    def resetTime(self):
       """Reset job timing"""
       self.startTime = -1
       self.submitTime = -1
       self.finishedTime = -1

    def __str__(self):
        return str(self.name)

    def __ge__(self, other):
        return self.name >= other.name

    def __lt__(self, other):
        return not self >= other


class SimulationStatistics:
    """A class that tracks statistics for Simulations (running time, waiting time, estimates, etc...)"""


    def __init__(self, name="default"):
        """Create a new tracking object"""
        self.name = name
        self.clear()
        self.old_job_count = 0
        self.old_duration = 0

    def clear(self):
        """Reset all values and stop estimate calculation"""
        self.jobs = 0
        self.waiting = 0
        self.running = 0
        self.total = 0
        self.start = 0
        self.last = 0

        self.stopped = True

    def startTiming(self):
        """Starts estimate calculation"""
        self.stopped = False
        self.start = int(time.time())

    def jobsPerSecond(self):
        """Returns the number of jobs per second as a float"""
        #t = int(time.time()) - self.start
        t = self.last - self.start
        if t > 0:
            return self.jobs / t
        else:
            return 0

    def averageRunningTime(self):
        """Calculates the average running time"""
        return self.running / self.jobs

    def secondsPerJob(self):
        """Returns how long a job takes in seconds"""
        return 1.0 / self.jobsPerSecond()

    def averageConcurrentJobs(self):
        """Returns the average number of jobs performed in parallel"""
        return max(self.running / (self.last - self.start), 1)

    def estimate(self, jobs):
        """Returns an estimate on how long the rest of the job will take. Jobs = the total number of jobs"""
        if self.jobsPerSecond() > 0:
            avg_concurrent_jobs = self.averageConcurrentJobs()
            avg_running = self.averageRunningTime()

            jobs_left = jobs - self.jobs
            est_remaining_running = avg_running * (jobs_left) / avg_concurrent_jobs
            timeUsed = int(time.time()) - self.last
            return est_remaining_running - timeUsed 
        else:
            return -1


    def addTime(self, submit, start, finish):
        """Add new statistical data to the tracker"""
        if not self.stopped:
            self.jobs += 1
            self.waiting += start - submit
            self.running += finish - start
            self.total += finish - submit
            self.last = int(time.time())

    def stop(self):
        """Pause the tracker. Estimate data will be reset"""
        self.old_job_count += self.jobs
        self.old_duration += int(time.time()) - self.start
        self.clear()

