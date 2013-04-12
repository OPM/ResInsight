#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'helpedwidget.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4 import QtGui, QtCore
import help
import sys
from   util import resourceIcon, resourceImage
import inspect
import ert.ert.enums as enums

def abstract():
    """Abstract keyword that indicate an abstract function"""
    import inspect
    caller = inspect.getouterframes(inspect.currentframe())[1][3]
    raise NotImplementedError(caller + ' must be implemented in subclass')

class UpdateOperations(enums.enum):
    INSERT = None
    REMOVE = None
    UPDATE = None
UpdateOperations.INSERT = UpdateOperations("Insert", 1)
UpdateOperations.REMOVE = UpdateOperations("Remove", 2)
UpdateOperations.UPDATE = UpdateOperations("Update", 3)


class ContentModel:
    """This class is a wrapper for communication between the model and the view."""
    contentModel = None
    signalManager = QtCore.QObject()
    observers = []

    INSERT = UpdateOperations.INSERT
    REMOVE = UpdateOperations.REMOVE
    UPDATE = UpdateOperations.UPDATE

    def __init__(self):
        """Constructs a ContentModel. All inheritors are registered"""
        ContentModel.observers.append(self)

    def initialize(self, model):
        """Should be implemented by inheriting classes that wants to do some one time initialization before getting and setting."""
        abstract()

    def getter(self, model):
        """MUST be implemented to get data from a source. Should not be called directly."""
        abstract()

    def setter(self, model, value):
        """MUST be implemented to update the source with new data. Should not be called directly."""
        abstract()

    def insert(self, model, value):
        """Can be implemented to insert new data to the source. Should not be called directly."""
        abstract()

    def remove(self, model, value):
        """Can be implemented to remove data from the source. Should not be called directly."""
        abstract()

    def fetchContent(self):
        """MUST be implemented. This function is called to tell all inheriting classes to retrieve data from the model. """
        abstract()

    def getFromModel(self):
        """Retrieves the data from the model. Calls the getter function with an appropriate model."""
        gargs = inspect.getargspec(self.getter)

        if inspect.isfunction(self.getter) and "self" in gargs[0]:
            data = self.getter.__call__(self, ContentModel.contentModel)
        else:
            data = self.getter(ContentModel.contentModel)

        return data

    def updateContent(self, value, operation = UPDATE):
        """
        Sends updated data to the model.
        Calls the a function with an appropriate model.
        The function is one of INSERT, REMOVE, UPDATE which corresponds to insert, remove and setter
        Any value returned by insert, remove or setter is also returned to the caller of this function

        Emits a SIGNAL 'contentsChanged()' after the function has been called.
        Emits a SIGNAL 'contentsChanged(int)' after the function has been called, where int is the operation performed.
        """
        if not ContentModel.contentModel is None :
            if ContentModel.INSERT == operation:
                result = self.insert(ContentModel.contentModel, value)
            elif ContentModel.REMOVE == operation:
                result = self.remove(ContentModel.contentModel, value)
            elif ContentModel.UPDATE == operation:
                result = self.setter(ContentModel.contentModel, value)
            else:
                sys.stderr.write("Unknown operation: %d\n" % (operation))
                return

            self.emit(QtCore.SIGNAL('contentsChanged()'))
            self.emit(QtCore.SIGNAL('contentsChanged(int)'), operation)

            return result

    def getModel(self):
        """Returns the contentModel associated with this session"""
        return ContentModel.contentModel

    # The modelConnect, modelDisconnect and modelEmit uses Qt signal handling to enable communication between
    # separate parts of the model that needs to know about changes.
    @classmethod
    def modelConnect(cls, signal, callable):
        """Connect to a custom signal available to all ContentModel objects."""
        QtCore.QObject.connect(ContentModel.signalManager, QtCore.SIGNAL(signal), callable)

    @classmethod
    def modelDisconnect(cls, signal, callable):
        """Disconnect from a custom signal available to all ContentModel objects."""
        QtCore.QObject.disconnect(ContentModel.signalManager, QtCore.SIGNAL(signal), callable)

    @classmethod
    def modelEmit(cls, signal, *args):
        """Emit a custom signal available to all ContentModel objects."""
        ContentModel.signalManager.emit(QtCore.SIGNAL(signal), *args)

    @classmethod
    def updateObservers(cls):
        """
        Calls all ContentModel inheritors to initialize (if implemented) and perform initial fetch of data.
        The signal 'initialized()' is emitted after initialization and fetching is completed.
        """
        for o in ContentModel.observers:
            try:
                o.initialize(ContentModel.contentModel)
            except NotImplementedError:
                sys.stderr.write("Missing initializer: " + o.helpLabel + "\n")
            except Exception:
                sys.stderr.write("Caught an exception during initialization!\n")

            #try:
            o.fetchContent()
            #except Exception:
            #    sys.stderr.write("Caught an exception while fetching!\n")

        ContentModel.modelEmit('initialized()')

    @classmethod
    def printObservers(cls):
        """Convenience method for printing the registered inheritors."""
        for o in ContentModel.observers:
            print o

    @classmethod
    def emptyInitializer(cls, model):
        """An empty initializer. Provided for convenience."""
        pass

class ContentModelProxy:
    """
    A ContentModelProxy adds an apply mode to a ContentModel
    The proxy sits between the widget and the updateContent function call
    and delays the updateContent call until the apply function of the proxy is called.
    It only relays the last updateContent call (for a single instance) so it should only
    be used in situations when this is the desired behaviour.
    """
    def __init__(self):
        self.objectFunctions = {}
        self.objectContent = {}

    def proxifyObject(self, object):
        """
        This function is here because lambdas loose context in loops.
        Lambdas point to the variable and not the content.
        """
        self.objectFunctions[object] = object.updateContent
        object.updateContent = lambda value, operation = ContentModel.UPDATE : self._proxyUpdateContent(object, value,
                                                                                                       operation)

    def proxify(self, *objects):
        """Add a ContenModel instance to this proxy group"""
        for object in objects:
            self.proxifyObject(object)

    def _proxyUpdateContent(self, object, value, operation):
        self.objectContent[object] = (value, operation)

    def apply(self):
        """Perform all delayed updateContent calls"""
        for key in self.objectFunctions.keys():
            function = self.objectFunctions[key]
            if self.objectContent.has_key(key):
                data = self.objectContent[key]
                function(data[0], data[1])
            else:
                #This usually means that no value has been sent to the setter
                #print "Unknown key: %s" % (str(key))
                pass



class HelpedWidget(QtGui.QWidget, ContentModel):
    """
    HelpedWidget is a class that enables embedded help messages in widgets.
    The help button must manually be added to the containing layout with addHelpButton().
    """

    STRONG_ERROR_COLOR = QtGui.QColor(255, 215, 215)
    ERROR_COLOR = QtGui.QColor(255, 235, 235)
    INVALID_COLOR = QtGui.QColor(235, 235, 255)

    WARNING = "warning"
    EXCLAMATION = "exclamation"

    def __init__(self, parent=None, widgetLabel="", helpLabel=""):
        """Creates a widget that can have a help button"""
        QtGui.QWidget.__init__(self, parent)
        ContentModel.__init__(self)


        self.validationLabel = QtGui.QLabel()
        self.validationLabel.setMaximumSize(QtCore.QSize(16, 16))
        self.validationLabel.setPixmap(resourceImage("warning"))
        self.validationLabel.setHidden(True)

        if not widgetLabel == "":
            self.label = widgetLabel + ":"
        else:
            self.label = ""

        self.helpMessage = help.resolveHelpLabel(helpLabel)
        self.helpLabel = helpLabel

        self.widgetLayout = QtGui.QHBoxLayout()
        #self.setStyleSheet("padding: 2px")
        self.widgetLayout.setMargin(0)
        self.widgetLayout.addWidget(self.validationLabel)
        self.setLayout(self.widgetLayout)


    def getHelpButton(self):
        """Returns the help button or None"""
        try:
            self.helpButton
        except AttributeError:
            self.helpButton = None

        return self.helpButton

    def showHelp(self):
        """Pops up the tooltip associated to the button"""
        QtGui.QToolTip.showText(QtGui.QCursor.pos(), self.helpMessage, self)

    def addHelpButton(self):
        """Adds the help button to the provided layout."""

        self.helpButton = QtGui.QToolButton(self)

        self.helpButton.setIcon(resourceIcon("help"))
        self.helpButton.setIconSize(QtCore.QSize(16, 16))
        self.helpButton.setToolButtonStyle(QtCore.Qt.ToolButtonIconOnly)
        self.helpButton.setAutoRaise(True)

        self.connect(self.helpButton, QtCore.SIGNAL('clicked()'), self.showHelp)

        if self.helpMessage == "":
            self.helpButton.setEnabled(False)

        if not self.getHelpButton() is None :
            self.addWidget(self.getHelpButton())

    def getLabel(self):
        """Returns the label of this widget if set or empty string."""
        return self.label

    def addLayout(self, layout):
        """Add a layout to the layout of this widget."""
        self.widgetLayout.addLayout(layout)


    def addWidget(self, widget):
        """Add a widget to the layout of this widget."""
        self.widgetLayout.addWidget(widget)

    def addStretch(self):
        """Add stretch between widgets. Usually added between a widget and the help button."""
        self.widgetLayout.addStretch(1)

    def setValidationMessage(self, message, validationType=WARNING):
        """Add a warning or information icon to the widget with a tooltip"""
        if message == "":
            self.validationLabel.setHidden(True)
            self.validationLabel.setToolTip("")
        else:
            self.validationLabel.setHidden(False)
            self.validationLabel.setToolTip(message)
            self.validationLabel.setPixmap(resourceImage(validationType))

