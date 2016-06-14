#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'field.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import sys

from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.config import FieldConfig


class Field(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    
    def ijk_get_double(self, i, j, k):
        return Field.cNamespace().ijk_get_double(self, i, j, k)

    
    def export(self , filename , file_type = None , init_file = None):
        output_transform = False
        if file_type is None:
            try:
                file_type = FieldConfig.exportFormat( filename )
            except ValueError:
                sys.stderr.write("Sorry - could not infer output format from filename:%s\n" % filename)
                return False

        Field.cNamespace().export(self , filename , None , file_type , output_transform , init_file )
        return True

    
    def free(self):
        Field.cNamespace().free(self)


##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("field", Field)

Field.cNamespace().free = cwrapper.prototype("void field_free( field )")
Field.cNamespace().ijk_get_double = cwrapper.prototype("double field_ijk_get_double(field, int, int, int)")
Field.cNamespace().export = cwrapper.prototype("void field_export(field, char* , fortio , enkf_field_file_format_enum , bool , char*)")
