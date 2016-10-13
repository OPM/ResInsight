from PyQt4.QtGui import QTreeView
from ert_gui.ide.wizards import TreeItem
from ert_gui.ide.wizards.tree_model import TreeModel


class WizardView(QTreeView):
    def __init__(self, parent=None):
        QTreeView.__init__(self, parent)

        self.__root = TreeItem("ERT")

        self.__tree_model = TreeModel(self.__root)

        self.setModel(self.__tree_model)

        #: :type: dict of (str, TreeItem)
        self.__groups = {}

        self.header().hide()


    def addGroup(self, group_name):
        if group_name in self.__groups:
            raise ValueError("A group with name: %s already exists!" % group_name)

        group = TreeItem(group_name)

        self.__groups[group_name] = group
        self.__root.addChild(group)

        self.__tree_model.emitChange()


    def addItemToGroup(self, group, item):
        group_item = self.__groups[group]

        child_item = TreeItem(item)
        group_item.addChild(child_item)

        self.__tree_model.emitChange()


