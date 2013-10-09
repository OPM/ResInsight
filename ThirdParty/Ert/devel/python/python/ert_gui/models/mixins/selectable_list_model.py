from ert_gui.models.mixins.list_model import ListModelMixin
from ert_gui.models.mixins.selectable_model import SelectableModelMixin


class SelectableListModelMixin(ListModelMixin, SelectableModelMixin):

    def __init__(self):
        self.__selection = {}
        super(SelectableListModelMixin, self).__init__()

    def isValueSelected(self, value):
        if self.__selection.has_key(value):
            return self.__selection[value]
        else:
            return True

    def selectValue(self, value):
        self.__setSelectState(value, True)
        self.observable().notify(self.SELECTION_CHANGED_EVENT)

    def unselectValue(self, value):
        self.__setSelectState(value, False)
        self.observable().notify(self.SELECTION_CHANGED_EVENT)

    def unselectAll(self):
        for item in self.getList():
            self.__setSelectState(item, False)

        self.observable().notify(self.SELECTION_CHANGED_EVENT)

    def selectAll(self):
        for item in self.getList():
            self.__setSelectState(item, True)

        self.observable().notify(self.SELECTION_CHANGED_EVENT)

    def getSelectedItems(self):
        result = []
        items = self.getList()

        for item in items:
            if self.isValueSelected(item):
                result.append(item)

        return result

    def __setSelectState(self, key, state):
        self.__selection[key] = state












