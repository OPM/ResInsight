#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'pathchooser.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtCore import Qt, QSize
from PyQt4.QtGui import QHBoxLayout, QLineEdit, QCompleter, QToolButton, QInputDialog, QMessageBox, QWidget

from ert_gui.ertwidgets import resourceIcon
from ert_gui.ertwidgets.validationsupport import ValidationSupport


class AutoCompleteLineEdit(QLineEdit):
    # http://blog.elentok.com/2011/08/autocomplete-textbox-for-multiple.html
    def __init__(self, items, parent=None):
        super(AutoCompleteLineEdit, self).__init__(parent)

        self._separators = [",", " "]

        self._completer = QCompleter(items, self)
        self._completer.setWidget(self)
        self._completer.activated[str].connect(self.__insertCompletion)
        self._completer.setCaseSensitivity(Qt.CaseInsensitive)

        self.__keysToIgnore = [Qt.Key_Enter, Qt.Key_Return, Qt.Key_Escape, Qt.Key_Tab]

    def __insertCompletion(self, completion):
        extra = len(completion) - len(self._completer.completionPrefix())
        extra_text = completion[-extra:]
        extra_text += ', '
        self.setText(self.text() + extra_text)

    def textUnderCursor(self):
        text = self.text()
        text_under_cursor = ''
        i = self.cursorPosition() - 1
        while i >= 0 and text[i] not in self._separators:
            text_under_cursor = text[i] + text_under_cursor
            i -= 1
        return text_under_cursor

    def keyPressEvent(self, event):
        if self._completer.popup().isVisible():
            if event.key() in self.__keysToIgnore:
                event.ignore()
                return

        super(AutoCompleteLineEdit, self).keyPressEvent(event)

        completion_prefix = self.textUnderCursor()
        if completion_prefix != self._completer.completionPrefix():
            self.__updateCompleterPopupItems(completion_prefix)
        if len(event.text()) > 0 and len(completion_prefix) > 0:
            self._completer.complete()
        if len(completion_prefix) == 0:
            self._completer.popup().hide()

    def __updateCompleterPopupItems(self, completionPrefix):
        self._completer.setCompletionPrefix(completionPrefix)
        self._completer.popup().setCurrentIndex(self._completer.completionModel().index(0, 0))


class ListEditBox(QWidget):
    ITEM_DOES_NOT_EXIST_MSG = "The item: '%s' is not a possible choice."
    NO_ITEMS_SPECIFIED_MSG = "The list must contain at least one item or * (for all)."
    DEFAULT_MSG = "A list of comma separated case names or * for all."

    def __init__(self, possible_items):
        QWidget.__init__(self)

        self._editing = True
        self._possible_items = possible_items

        self._list_edit_line = AutoCompleteLineEdit(possible_items, self)
        self._list_edit_line.setMinimumWidth(350)

        layout = QHBoxLayout()
        layout.setMargin(0)

        layout.addWidget(self._list_edit_line)

        dialog_button = QToolButton(self)
        dialog_button.setIcon(resourceIcon("ide/small/add"))
        dialog_button.setIconSize(QSize(16, 16))
        dialog_button.clicked.connect(self.addChoice)

        layout.addWidget(dialog_button)

        self.setLayout(layout)

        self._validation_support = ValidationSupport(self)
        self._valid_color = self._list_edit_line.palette().color(self._list_edit_line.backgroundRole())

        self._list_edit_line.setText("")
        self._editing = False

        self._list_edit_line.editingFinished.connect(self.validateList)
        self._list_edit_line.textChanged.connect(self.validateList)

        self.validateList()

    def getListText(self):
        text = str(self._list_edit_line.text())
        text = "".join(text.split())
        return text

    def getItems(self):
        text = self.getListText()
        items = text.split(",")

        if len(items) == 1 and items[0] == "*":
            items = self._possible_items

        return [item for item in items if len(item) > 0]

    def validateList(self):
        """Called whenever the list is modified"""
        palette = self._list_edit_line.palette()

        items = self.getItems()

        valid = True
        message = ""

        if len(items) == 0:
            valid = False
            message = ListEditBox.NO_ITEMS_SPECIFIED_MSG
        else:
            for item in items:
                if item not in self._possible_items:
                    valid = False
                    message = ListEditBox.ITEM_DOES_NOT_EXIST_MSG % item

        validity_type = ValidationSupport.WARNING

        if not valid:
            color = ValidationSupport.ERROR_COLOR
        else:
            color = self._valid_color

        self._validation_support.setValidationMessage(message, validity_type)
        self._list_edit_line.setToolTip(message)
        palette.setColor(self._list_edit_line.backgroundRole(), color)

        self._list_edit_line.setPalette(palette)

        if valid:
            self._list_edit_line.setToolTip(ListEditBox.DEFAULT_MSG)

    def addChoice(self):
        if len(self._possible_items) == 0:
            QMessageBox.information(self, "No items", "No items available for selection!")
        else:
            item, ok = QInputDialog.getItem(self,
                                            "Select a case",
                                            "Select a case to add to the case list:",
                                            self._possible_items)

            if ok:
                item = str(item).strip()
                text = str(self._list_edit_line.text()).rstrip()

                if len(text) == 0:
                    text = item + ", "
                elif text.endswith(","):
                    text += " " + item
                else:
                    text += ", " + item

                self._list_edit_line.setText(text)

    def getValidationSupport(self):
        return self._validation_support

    def isValid(self):
        return self._validation_support.isValid()
