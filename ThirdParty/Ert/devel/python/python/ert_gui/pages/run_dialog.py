from PyQt4.QtCore import Qt, pyqtSignal, QTimer, pyqtSlot
from PyQt4.QtGui import QDialog, QVBoxLayout, QLayout, QMessageBox, QPushButton, QHBoxLayout, QColor, QLabel
from ert_gui.models.connectors.run import SimulationRunner, SimulationsTracker
from ert_gui.widgets.legend import Legend
from ert_gui.widgets.progress import Progress


class RunDialog(QDialog):

    simulationFinished = pyqtSignal()

    def __init__(self, run_model):
        QDialog.__init__(self)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowCloseButtonHint)
        self.setModal(True)
        self.setWindowTitle("Simulations")

        assert isinstance(run_model, SimulationRunner)
        self.run_model = run_model
        self.run_model.observable().attach(SimulationRunner.SIMULATION_FINISHED_EVENT, self.simulationFinished.emit)


        layout = QVBoxLayout()
        layout.setSizeConstraint(QLayout.SetFixedSize)

        self.simulations_tracker = SimulationsTracker()
        self.simulations_tracker.observable().attach(SimulationsTracker.LIST_CHANGED_EVENT, self.statusChanged)
        states = self.simulations_tracker.getList()

        self.progress = Progress()

        for state in states:
            self.progress.addState(state.state, QColor(*state.color), 100.0 * state.count / state.total_count)

        layout.addWidget(self.progress)

        legend_layout = QHBoxLayout()

        for state in states:
            legend_layout.addWidget(Legend(state.name, QColor(*state.color)))

        layout.addLayout(legend_layout)

        self.running_time = QLabel("")

        self.kill_button = QPushButton("Kill simulations")
        self.done_button = QPushButton("Done")
        self.done_button.setHidden(True)

        button_layout = QHBoxLayout()
        button_layout.addWidget(self.running_time)
        button_layout.addStretch()
        button_layout.addWidget(self.kill_button)
        button_layout.addWidget(self.done_button)

        layout.addStretch()
        layout.addLayout(button_layout)

        self.setLayout(layout)

        self.kill_button.clicked.connect(self.killJobs)
        self.done_button.clicked.connect(self.accept)
        self.simulationFinished.connect(self.hideKillAndShowDone)

        timer = QTimer(self)
        timer.setInterval(500)
        timer.timeout.connect(self.setRunningTime)
        timer.start()

    def setRunningTime(self):
        self.running_time.setText("Running time: %d seconds" % self.run_model.getRunningTime())

    def statusChanged(self):
        states = self.simulations_tracker.getList()

        for state in states:
            self.progress.updateState(state.state, 100.0 * state.count / state.total_count)


    def killJobs(self):
        kill_job = QMessageBox.question(self, "Kill simulations?", "Are you sure you want to kill the currently running simulations?", QMessageBox.Yes | QMessageBox.No )

        if kill_job == QMessageBox.Yes:
            self.run_model.killAllSimulations()
            QDialog.reject(self)

    def hideKillAndShowDone(self):
        self.kill_button.setHidden(True)
        self.done_button.setHidden(False)