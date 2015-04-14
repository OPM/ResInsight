from PyQt4.QtGui import QAction
from ert_gui.tools import HelpCenter


class Tool(object):
    def __init__(self, name, help_link="", icon=None, enabled=True, checkable=False, popup_menu=False):
        super(Tool, self).__init__()
        self.__icon = icon
        self.__name = name
        self.__parent = None
        self.__enabled = enabled
        self.__checkable = checkable
        self.__help_link = help_link
        self.__is_popup_menu = popup_menu

        self.__action = QAction(self.getIcon(), self.getName(), None)
        self.__action.setIconText(self.getName())
        self.__action.setEnabled(self.isEnabled())
        self.__action.setCheckable(checkable)
        self.__action.triggered.connect(self.trigger)

        HelpCenter.addHelpToAction(self.__action, self.getHelpLink())

    def getIcon(self):
        return self.__icon

    def getName(self):
        return self.__name

    def trigger(self):
        raise NotImplementedError()

    def setParent(self, parent):
        self.__parent = parent
        self.__action.setParent(parent)

    def parent(self):
        return self.__parent

    def isEnabled(self):
        return self.__enabled

    def getHelpLink(self):
        return self.__help_link


    def getAction(self):
        return self.__action

    def setVisible(self, visible):
        self.__action.setVisible(visible)

    def setEnabled(self, enabled):
        self.__action.setEnabled(enabled)

    def isPopupMenu(self):
        return self.__is_popup_menu
