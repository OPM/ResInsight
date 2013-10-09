from PyQt4.QtGui import QToolButton, QMenu, QAction
from ert_gui.models.mixins.button_model import ButtonModelMixin
from ert_gui.widgets.helped_widget import HelpedWidget


class Button(HelpedWidget):

    def __init__(self, model, label="", help_link=""):
        HelpedWidget.__init__(self, label, help_link)

        self.models = {}
        assert isinstance(model, ButtonModelMixin)

        self.button = QToolButton()
        self.button.setMinimumWidth(75)
        self.button.setDefaultAction(self.createAction(model))

        self.addWidget(self.button)


    def addOption(self, model):
        """ @type model: ButtonModelMixin"""
        if self.button.popupMode() != QToolButton.MenuButtonPopup:
            self.button.setPopupMode(QToolButton.MenuButtonPopup)
            self.button.setMenu(QMenu())
            menu = self.button.menu()
            menu.addAction(self.button.defaultAction())

        menu = self.button.menu()
        action  = self.createAction(model)
        menu.addAction(action)

    def createAction(self, model):
        """ @type model: ButtonModelMixin"""
        action = QAction(self.button)
        action.setText(model.getButtonName())
        action.setEnabled(model.buttonIsEnabled())
        action.triggered.connect(model.buttonTriggered)
        model.observable().attach(ButtonModelMixin.BUTTON_STATE_CHANGED_EVENT, self.modelChanged)
        self.models[model] = action
        return action

    def modelChanged(self):
        for model in self.models:
            assert isinstance(model, ButtonModelMixin)
            action = self.models[model]
            action.setText(model.getButtonName())
            action.setEnabled(model.buttonIsEnabled())
