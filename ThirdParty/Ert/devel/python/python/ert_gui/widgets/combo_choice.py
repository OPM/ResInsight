#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'combo_choice.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QComboBox
from ert_gui.models.mixins import ChoiceModelMixin
from ert_gui.widgets.helped_widget import HelpedWidget


class ComboChoice(HelpedWidget):
    """
    A combo box widget for choices.
    List of objects retrieved from model
    str(item) is used for presentation
    getValue and setValue uses the same object as from the list
    """
    def __init__(self, model, combo_label="Choice", help_link=""):
        HelpedWidget.__init__(self, combo_label, help_link)

        assert model is not None and isinstance(model, ChoiceModelMixin)
        self.model = model


        model.observable().attach(ChoiceModelMixin.CURRENT_CHOICE_CHANGED_EVENT, self.getCurrentFromModel)
        model.observable().attach(ChoiceModelMixin.CHOICE_LIST_CHANGED_EVENT, self.updateChoicesFromModel)

        self.combo = QComboBox()

        self.combo.addItem("Fail!")

        self.addWidget(self.combo)
        self.addStretch()

        self.choice_list = None
        """ @type: list """

        self.connect(self.combo, SIGNAL('currentIndexChanged(int)'), self.selectionChanged)

        self.updateChoicesFromModel()
        self.getCurrentFromModel()


    def selectionChanged(self, index):
        assert 0 <= index < len(self.choice_list), "Should not happen! Index out of range: 0 <= %i < %i" % (index, len(self.choice_list))

        item = self.choice_list[index]
        self.model.setCurrentChoice(item)


    def getCurrentFromModel(self):
        new_value = self.model.getCurrentChoice()

        if self.choice_list is None:
            self.updateChoicesFromModel()

        if new_value in self.choice_list:
            index = self.choice_list.index(new_value)
            if not index == self.combo.currentIndex():
                self.combo.setCurrentIndex(index)
        else:
            self.combo.setCurrentIndex(0)

            #sys.stderr.write("AssertionError: ComboBox can not be set to: " + str(new_value) + "\n")
            # raise AssertionError("ComboBox can not be set to: " + str(new_value))


    def updateChoicesFromModel(self):
        block = self.combo.signalsBlocked()
        self.combo.blockSignals(True)

        self.choice_list = self.model.getChoices()

        self.combo.clear()
        for choice in self.choice_list:
            self.combo.addItem(str(choice))

        self.combo.blockSignals(block)
