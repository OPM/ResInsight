from threading import Thread

from PyQt4.QtCore import Qt, QTimer, QSize
from PyQt4.QtGui import QDialog, QVBoxLayout, QLayout, QMessageBox, QPushButton, QHBoxLayout, QColor, QLabel

from ert_gui.ertwidgets import resourceMovie, Legend
from ert_gui.simulation import Progress, SimpleProgress
from ert_gui.simulation.models import BaseRunModel, SimulationsTracker
from ert_gui.tools.plot.plot_tool import PlotTool


class RunDialog(QDialog):

    def __init__(self, run_model, run_arguments, parent):
        QDialog.__init__(self, parent)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowCloseButtonHint)
        self.setModal(True)
        self.setWindowModality(Qt.WindowModal)
        self.setWindowTitle("Simulations")

        assert isinstance(run_model, BaseRunModel)
        self._run_model = run_model
        self._run_arguments = run_arguments

        layout = QVBoxLayout()
        layout.setSizeConstraint(QLayout.SetFixedSize)

        self.simulations_tracker = SimulationsTracker()
        states = self.simulations_tracker.getStates()

        self.total_progress = SimpleProgress()
        layout.addWidget(self.total_progress)


        status_layout = QHBoxLayout()
        status_layout.addStretch()
        self.__status_label = QLabel()
        status_layout.addWidget(self.__status_label)
        status_layout.addStretch()
        layout.addLayout(status_layout)

        self.progress = Progress()
        self.progress.setIndeterminateColor(self.total_progress.color)
        for state in states:
            self.progress.addState(state.state, QColor(*state.color), 100.0 * state.count / state.total_count)

        layout.addWidget(self.progress)

        legend_layout = QHBoxLayout()
        self.legends = {}
        for state in states:
            self.legends[state] = Legend("%s (%d/%d)", QColor(*state.color))
            self.legends[state].updateLegend(state.name, 0, 0)
            legend_layout.addWidget(self.legends[state])

        layout.addLayout(legend_layout)

        self.running_time = QLabel("")

        ert = None
        if isinstance(run_model, BaseRunModel):
            ert = run_model.ert()

        self.plot_tool = PlotTool()
        self.plot_tool.setParent(None)
        self.plot_button = QPushButton(self.plot_tool.getName())
        self.plot_button.clicked.connect(self.plot_tool.trigger)
        self.plot_button.setEnabled(ert is not None)
        
        self.kill_button = QPushButton("Kill simulations")
        self.done_button = QPushButton("Done")
        self.done_button.setHidden(True)

        button_layout = QHBoxLayout()

        size = 20
        spin_movie = resourceMovie("ide/loading.gif")
        spin_movie.setSpeed(60)
        spin_movie.setScaledSize(QSize(size, size))
        spin_movie.start()

        self.processing_animation = QLabel()
        self.processing_animation.setMaximumSize(QSize(size, size))
        self.processing_animation.setMinimumSize(QSize(size, size))
        self.processing_animation.setMovie(spin_movie)

        button_layout.addWidget(self.processing_animation)
        button_layout.addWidget(self.running_time)
        button_layout.addStretch()
        button_layout.addWidget(self.plot_button)
        button_layout.addWidget(self.kill_button)
        button_layout.addWidget(self.done_button)

        layout.addStretch()
        layout.addLayout(button_layout)

        self.setLayout(layout)

        self.kill_button.clicked.connect(self.killJobs)
        self.done_button.clicked.connect(self.accept)

        self.__updating = False
        self.__update_queued = False
        self.__simulation_started = False

        self.__update_timer = QTimer(self)
        self.__update_timer.setInterval(500)
        self.__update_timer.timeout.connect(self.updateRunStatus)


    def startSimulation(self):
        self._run_model.reset()

        def run():
            self._run_model.startSimulations(self._run_arguments)

        simulation_thread = Thread(name="ert_gui_simulation_thread")
        simulation_thread.setDaemon(True)
        simulation_thread.run = run
        simulation_thread.start()

        self.__update_timer.start()


    def checkIfRunFinished(self):
        if self._run_model.isFinished():
            self.hideKillAndShowDone()

            if self._run_model.hasRunFailed():
                error = self._run_model.getFailMessage()
                QMessageBox.critical(self, "Simulations failed!", "The simulation failed with the following error:\n\n%s" % error)
                self.reject()


    def updateRunStatus(self):
        self.checkIfRunFinished()

        self.total_progress.setProgress(self._run_model.getProgress())

        self.__status_label.setText(self._run_model.getPhaseName())

        states = self.simulations_tracker.getStates()

        if self._run_model.isIndeterminate():
            self.progress.setIndeterminate(True)

            for state in states:
                self.legends[state].updateLegend(state.name, 0, 0)

        else:
            self.progress.setIndeterminate(False)
            total_count = self._run_model.getQueueSize()
            queue_status = self._run_model.getQueueStatus()

            for state in states:
                state.count = 0
                state.total_count = total_count

            for state in states:
                for queue_state in queue_status:
                    if queue_state in state.state:
                        state.count += queue_status[queue_state]

                self.progress.updateState(state.state, 100.0 * state.count / state.total_count)
                self.legends[state].updateLegend(state.name, state.count, state.total_count)

        self.setRunningTime()


    def setRunningTime(self):
        days = 0
        hours = 0
        minutes = 0
        seconds = self._run_model.getRunningTime()

        if seconds >= 60:
            minutes, seconds = divmod(seconds, 60)

        if minutes >= 60:
            hours, minutes = divmod(minutes, 60)

        if hours >= 24:
            days, hours = divmod(hours, 24)

        if days > 0:
            self.running_time.setText("Running time: %d days %d hours %d minutes %d seconds" % (days, hours, minutes, seconds))
        elif hours > 0:
            self.running_time.setText("Running time: %d hours %d minutes %d seconds" % (hours, minutes, seconds))
        elif minutes > 0:
            self.running_time.setText("Running time: %d minutes %d seconds" % (minutes, seconds))
        else:
            self.running_time.setText("Running time: %d seconds" % seconds)


    def killJobs(self):
        kill_job = QMessageBox.question(self, "Kill simulations?", "Are you sure you want to kill the currently running simulations?", QMessageBox.Yes | QMessageBox.No )

        if kill_job == QMessageBox.Yes:
            if self._run_model.killAllSimulations():
                self.reject()


    def hideKillAndShowDone(self):
        self.__update_timer.stop()
        self.processing_animation.hide()
        self.kill_button.setHidden(True)
        self.done_button.setHidden(False)
