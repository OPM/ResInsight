from PyQt4.QtGui import QComboBox

from ert_gui import ERT
from ert_gui.ertwidgets import addHelpToWidget
from ert_gui.ertwidgets.models.ertmodel import getAllCases, selectOrCreateNewCase, getCurrentCaseName, getAllInitializedCases


class CaseSelector(QComboBox):
    def __init__(self, update_ert=True, show_only_initialized=False, ignore_current=False, help_link="init/current_case_selection"):
        QComboBox.__init__(self)
        self._update_ert = update_ert # If true current case of ert will be change
        self._show_only_initialized = show_only_initialized # only show initialized cases
        self._ignore_current = ignore_current # ignore the currently selected case if it changes


        addHelpToWidget(self, help_link)
        self.setSizeAdjustPolicy(QComboBox.AdjustToContents)

        self.populate()

        self.currentIndexChanged[int].connect(self.selectionChanged)
        ERT.ertChanged.connect(self.populate)

    def _getAllCases(self):
        if self._show_only_initialized:
            return getAllInitializedCases()
        else:
            return getAllCases()

    def selectionChanged(self, index):
        if self._update_ert:
            assert 0 <= index < self.count(), "Should not happen! Index out of range: 0 <= %i < %i" % (index, self.count())

            item = self._getAllCases()[index]
            selectOrCreateNewCase(item)

    def populate(self):
        block = self.signalsBlocked()
        self.blockSignals(True)

        case_list = self._getAllCases()
        self.clear()

        for case in case_list:
            self.addItem(case)

        current_index = 0
        current_case = getCurrentCaseName()
        if current_case in case_list:
            current_index = case_list.index(current_case)

        if current_index != self.currentIndex() and not self._ignore_current:
            self.setCurrentIndex(current_index)

        self.blockSignals(block)
