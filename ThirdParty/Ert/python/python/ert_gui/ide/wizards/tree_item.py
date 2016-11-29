class TreeItem(object):
    def __init__(self, name, data=None):
        super(TreeItem, self).__init__()
        self.__name = name
        self.__parent = None

        self.__children = []
        self.__data = data

    def child(self, row):
        return self.__children[row]

    def __len__(self):
        return len(self.__children)

    def data(self):
        return self.__data

    def name(self):
        return self.__name

    def addChild(self, child):
        assert isinstance(child, TreeItem)
        child.setParent(self)
        self.__children.append(child)
        return child

    def parent(self):
        return self.__parent

    def setParent(self, parent):
        self.__parent = parent

    def row(self):
        if self.__parent is not None:
            return self.__parent.__children.index(self)

        return 0