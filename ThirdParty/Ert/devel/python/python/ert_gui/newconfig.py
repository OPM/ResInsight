#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'newconfig.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4.QtGui import QDialog, QFormLayout, QLabel, QDialogButtonBox, QComboBox, QCheckBox , QSpinBox , QLineEdit
from PyQt4.QtCore import Qt, SIGNAL
from widgets.util import createSpace
import os

class NewConfigurationDialog(QDialog):
    """A dialog for selecting defaults for a new configuration."""
    def __init__(self, configuration_path, parent = None):
        QDialog.__init__(self, parent)

        self.setModal(True)
        self.setWindowTitle("New configuration file")
        self.setMinimumWidth(250)
        self.setMinimumHeight(150)

        layout = QFormLayout()

        directory, filename = os.path.split(configuration_path)

        if directory.strip() == "":
            directory = os.path.abspath(os.curdir)
            self.configuration_path = "%s/%s" % (directory, filename)
        else:
            self.configuration_path = configuration_path


        configuration_location = QLabel()
        configuration_location.setText(directory)

        configuration_name = QLabel()
        configuration_name.setText(filename)

        self.db_type = QComboBox()
        self.db_type.addItem("BLOCK_FS")
        self.db_type.addItem("PLAIN")
        
        self.first_case_name = QLineEdit()
        self.first_case_name.setText("default")
        self.connect(self.first_case_name, SIGNAL('textChanged(QString)'), self._validateName)

        self.num_realizations = QSpinBox()
        self.num_realizations.setMinimum( 1 )
        self.num_realizations.setMaximum( 1000 )
        self.num_realizations.setValue( 10 )

        self.storage_path = QLineEdit()
        self.storage_path.setText("Storage")
        self.connect(self.storage_path, SIGNAL('textChanged(QString)'), self._validateName)


        layout.addRow(createSpace(10))
        layout.addRow("Configuration name:", configuration_name)
        layout.addRow("Configuration location:", configuration_location)
        layout.addRow("Path to store DBase:",self.storage_path)
        layout.addRow("DBase type:", self.db_type)
        layout.addRow("Name of first case:", self.first_case_name)
        layout.addRow("Number of realizations" , self.num_realizations)
        layout.addRow(createSpace(10))

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, Qt.Horizontal, self)
        self.ok_button = buttons.button(QDialogButtonBox.Ok)

        layout.addRow(buttons)

        self.connect(buttons, SIGNAL('accepted()'), self.accept)
        self.connect(buttons, SIGNAL('rejected()'), self.reject)


        self.setLayout(layout)


    def getNumberOfRealizations(self):
        return self.num_realizations.value()
    
    def getConfigurationPath(self):
        return self.configuration_path

    def getCaseName(self):
        """Return the name of the first case."""
        return str(self.first_case_name.text()).strip()

    def getDBaseType(self):
        """Return the DBase type"""
        return str(self.db_type.currentText())

    def getStoragePath(self):
        """Return the DBase storage path"""
        return str(self.storage_path.text()).strip()

    def _validateName(self, name):
        name = str(name)
        enabled = len(name) > 0 and name.find(" ") == -1
        self.ok_button.setEnabled(enabled)
