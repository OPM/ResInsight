from .PdmObject import PdmObject

class View (PdmObject):
    """ResInsight view class

    Attributes:
        id(int): View Id corresponding to the View Id in ResInsight project.

    """
    def __init__(self, pbmObject):
        self.id = pbmObject.getValue("ViewId")

        PdmObject.__init__(self, pbmObject.pb2Object, pbmObject.channel)

    def showGridBox(self):
        """Check if the grid box is meant to be shown in the view"""
        return self.getValue("ShowGridBox")

    def setShowGridBox(self, value):
        """Set if the grid box is meant to be shown in the view"""
        self.setValue("ShowGridBox", value)

    def backgroundColor(self):
        """Get the current background color in the view"""
        return self.getValue("ViewBackgroundColor")

    def setBackgroundColor(self, bgColor):
        """Set the background color in the view"""
        self.setValue("ViewBackgroundColor", bgColor)

    def cellResult(self):
        """Retrieve the current cell results"""
        return self.children("GridCellResult")[0]