#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'parametermodels.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from ert.ert.enums import enkf_impl_type, field_type
from PyQt4.QtCore import QObject
from PyQt4.Qt import SIGNAL

class Model(QObject):

    def __init__(self, name):
        QObject.__init__(self)
        self.name = name
        self.data = {}
        self.valid = True

    def set(self, attr, value):
        self[attr] = value

    def __setitem__(self, attr, value):        
        self.data[attr] = value
        self.emitUpdate()
        

    def __getitem__(self, item):
        return self.data[item]

    def isValid(self):
        return self.valid

    def setValid(self, valid):
        if not self.valid == valid:
            self.valid = valid
            self.emitUpdate()

    def emitUpdate(self):
        self.emit(SIGNAL("modelChanged(Model)"), self)


    def getName(self):
        return self.name


class FieldModel(Model):
    TYPE = enkf_impl_type.FIELD

    def __init__(self, name):
        Model.__init__(self, name)
        self.name = name

        self["type"] = field_type.GENERAL
        self["min"] = ""
        self["max"] = ""
        self["init"] = "None"
        self["output"] = "None"
        self["init_files"] = ""
        self["enkf_outfile"] = ""
        self["enkf_infile"] = ""
        self["min_std"] = ""

class KeywordModel(Model):
    TYPE = enkf_impl_type.GEN_KW

    def __init__(self, name):
        Model.__init__(self, name)
        self.name = name

        self["min_std"] = ""
        self["enkf_outfile"] = ""
        self["template"] = ""
        self["init_files"] = ""
        self["parameter_file"] = ""

class DataModel(Model):
    TYPE = enkf_impl_type.GEN_DATA
    
    def __init__(self, name):
        Model.__init__(self, name)
        self.name = name

        self["input_format"] = ""
        self["output_format"] = ""
        self["template_file"] = ""
        self["template_key"] = ""
        self["init_file_fmt"] = ""
        self["enkf_outfile"] = ""
        self["enkf_infile"] = ""
        self["min_std"] = ""

class SummaryModel(Model):
    TYPE = enkf_impl_type.SUMMARY

    def __init__(self, name):
        Model.__init__(self, name)
        self.name = name
