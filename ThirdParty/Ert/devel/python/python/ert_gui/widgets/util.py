#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'util.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import os
import time

# The variable @img_prefix should be set to point to a directory
# containing icons and images. In the current implementation this
# variable is set from the gert_main.py script.
img_prefix = ""

def resourceIcon(name):
    """Load an image as an icon"""
    # print("Icon used: %s" % name)
    return QtGui.QIcon(img_prefix + name)

def resourceStateIcon(on, off):
    """Load two images as an icon with on and off states"""
    icon = QtGui.QIcon()
    icon.addPixmap(resourceImage(on), state=QtGui.QIcon.On)
    icon.addPixmap(resourceImage(off), state=QtGui.QIcon.Off)
    return icon

def resourceImage(name):
    """Load an image as a Pixmap"""
    return QtGui.QPixmap(img_prefix + name)


def resourceMovie(name):
    """ @rtype: QMovie """
    movie = QtGui.QMovie(img_prefix + name)
    movie.start()
    return movie




class ValidationInfo(QtGui.QWidget):
    """A message with an icon that present information about validation."""

    WARNING = "warning"
    EXCLAMATION = "exclamation"

    def __init__(self, type=WARNING, parent = None):
        QtGui.QWidget.__init__(self, parent)

        layout = QtGui.QHBoxLayout()
        self.iconLabel = QtGui.QLabel()
        self.iconLabel.setMaximumSize(QtCore.QSize(16, 16))
        self.iconLabel.setPixmap(resourceImage(type))
        layout.addWidget(self.iconLabel)

        self.messageLabel = QtGui.QLabel()
        layout.addWidget(self.messageLabel)
        self.setLayout(layout)
        self.setHidden(True)

    def setMessage(self, msg):
        """Set this message to an empty string to hide the validation widget"""
        if msg.strip() == "":
            self.setHidden(True)
        else:
            self.setHidden(False)

        self.messageLabel.setText(msg)



def createSeparator():
        """Creates a widget that can be used as a separator line on a panel."""
        qw = QtGui.QWidget()
        qwl = QtGui.QVBoxLayout()
        qw.setLayout(qwl)

        qf = QtGui.QFrame()
        qf.setFrameShape(QtGui.QFrame.HLine)
        qf.setFrameShadow(QtGui.QFrame.Sunken)

        qwl.addSpacing(5)
        qwl.addWidget(qf)
        qwl.addSpacing(5)

        return qw

def createSpace(size = 5):
    """Creates a widget that can be used as spacing on  a panel."""
    qw = QtGui.QWidget()
    qw.setMinimumSize(QtCore.QSize(size, size))

    return qw

def createEmptyPanel():
    """An empty expanding bordered panel"""
    emptyPanel = QtGui.QFrame()
    emptyPanel.setFrameShape(QtGui.QFrame.StyledPanel)
    emptyPanel.setFrameShadow(QtGui.QFrame.Plain)
    emptyPanel.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
    return emptyPanel

def centeredWidget(widget):
    """Returns a layout with the widget centered"""
    layout = QtGui.QHBoxLayout()
    layout.addStretch(1)
    layout.addWidget(widget)
    layout.addStretch(1)
    return layout


def getItemsFromList(list, func = lambda item : str(item.text()), selected = True) :
    """Creates a list of strings from the selected items of a ListWidget or all items if selected is False"""
    if selected:
        selectedItemsList = list.selectedItems()
    else:
        selectedItemsList = []
        for index in range(list.count()):
            selectedItemsList.append(list.item(index))

    selectedItems = []
    for item in selectedItemsList:
        selectedItems.append(func(item))

    return selectedItems

    
def frange(*args):
    """
    A float range generator.
    Found here: http://code.activestate.com/recipes/66472/
    """
    start = 0.0
    step = 1.0

    l = len(args)
    if l == 1:
        end = args[0]
    elif l == 2:
        start, end = args
    elif l == 3:
        start, end, step = args
        if step == 0.0:
            raise ValueError, "step must not be zero"
    else:
        raise TypeError, "frange expects 1-3 arguments, got %d" % l

    v = start
    while True:
        if (step > 0 and v >= end) or (step < 0 and v <= end):
            raise StopIteration
        yield v
        v += step

def shortTime(secs):
    """Converts seconds into hours:minutes:seconds. -1 returns a '-'"""
    if secs == -1:
        return "-"
    else:
        t = time.localtime(secs)
        return time.strftime("%H:%M:%S", t)


#-------------------------------------------------------------
# Function decorators
#-------------------------------------------------------------

def print_timing(func):
    """A function decorator that performs timing of the applied function"""
    def wrapper(*arg):
        t1 = time.time()
        res = func(*arg)
        t2 = time.time()
        print '%s took %0.3f ms' % (func.func_name, (t2-t1)*1000.0)
        return res
    return wrapper

    
def may_take_a_long_time(func):
    """A function decorator to show the wait cursor while the function is working."""
    def wrapper(*arg):
        QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.WaitCursor))
        res = func(*arg)
        QtGui.QApplication.restoreOverrideCursor()
        return res
    return wrapper
