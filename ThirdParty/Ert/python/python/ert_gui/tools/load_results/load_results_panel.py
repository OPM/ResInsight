#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'load_results_panel.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtGui import QWidget, QFormLayout, QComboBox, QTextEdit

from ert_gui.ertwidgets.models.activerealizationsmodel import ActiveRealizationsModel
from ert_gui.ertwidgets.models.all_cases_model import AllCasesModel
from ert_gui.ertwidgets.models.ertmodel import getCurrentCaseName
from ert_gui.ertwidgets.models.valuemodel import ValueModel
from ert_gui.ertwidgets.stringbox import StringBox
from ert_gui.ide.keywords.definitions import RangeStringArgument, IntegerArgument
from ert_gui.tools.load_results import LoadResultsModel


class LoadResultsPanel(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setMinimumWidth(500)
        self.setMinimumHeight(200)
        self._dynamic = False

        self.setWindowTitle("Load results manually")
        self.activateWindow()

        layout = QFormLayout()
        current_case = getCurrentCaseName()

        run_path_text = QTextEdit()
        run_path_text.setText(self.readCurrentRunPath())
        run_path_text.setDisabled(True)
        run_path_text.setFixedHeight(80)

        layout.addRow("Load data from current run path: ",run_path_text)

        self._case_model = AllCasesModel()
        self._case_combo = QComboBox()
        self._case_combo.setSizeAdjustPolicy(QComboBox.AdjustToMinimumContentsLength)
        self._case_combo.setMinimumContentsLength(20)
        self._case_combo.setModel(self._case_model)
        self._case_combo.setCurrentIndex(self._case_model.indexOf(current_case))
        layout.addRow("Load into case:", self._case_combo)


        self._active_realizations_model = ActiveRealizationsModel()
        self._active_realizations_field = StringBox(self._active_realizations_model, "load_results_manually/Realizations")
        self._active_realizations_field.setValidator(RangeStringArgument())
        layout.addRow("Realizations to load:", self._active_realizations_field)

        iterations_count = LoadResultsModel.getIterationCount()

        self._iterations_model = ValueModel(iterations_count)
        self._iterations_field = StringBox(self._iterations_model, "load_results_manually/iterations")
        self._iterations_field.setValidator(IntegerArgument())
        layout.addRow("Iteration to load:", self._iterations_field)

        self.setLayout(layout)

    def readCurrentRunPath(self):
        current_case = getCurrentCaseName()
        run_path = LoadResultsModel.getCurrentRunPath()
        run_path = run_path.replace("<ERTCASE>",current_case)
        run_path = run_path.replace("<ERT-CASE>",current_case)
        return run_path


    def load(self):
        all_cases = self._case_model.getAllItems()
        selected_case  = all_cases[self._case_combo.currentIndex()]
        realizations = self._active_realizations_model.getActiveRealizationsMask()
        iteration = self._iterations_model.getValue()
        try:
            if iteration is None:
                iteration = ''
            iteration = int(iteration)
        except ValueError as e:
            print('Expected a (whole) number in iteration field, got "%s". Error message: %s.'  % (iteration, e))
            return False
        loaded = LoadResultsModel.loadResults(selected_case, realizations, iteration)
        if loaded > 0:
            print('Successfully loaded %d realisations.' % loaded)
        else:
            print('No realisations loaded.')
        return loaded

    def setCurrectCase(self):
        current_case = getCurrentCaseName()
        self._case_combo.setCurrentIndex(self._case_model.indexOf(current_case))