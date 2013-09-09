#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ert_template.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB


class ErtTemplate(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_template_file(self):
        """ @rtype: str """
        return ErtTemplate.cNamespace().get_template_file(self)

    def get_target_file(self):
        """ @rtype: str """
        return ErtTemplate.cNamespace().get_target_file(self)

    def get_args_as_string(self):
        """ @rtype: str """
        return ErtTemplate.cNamespace().get_args_as_string(self)

    def free(self):
        ErtTemplate.cNamespace().free(self)

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ert_template", ErtTemplate)
cwrapper.registerType("ert_template_obj", ErtTemplate.createPythonObject)
cwrapper.registerType("ert_template_ref", ErtTemplate.createCReference)

ErtTemplate.cNamespace().free = cwrapper.prototype("void ert_template_free( ert_template )")
ErtTemplate.cNamespace().get_template_file = cwrapper.prototype("char* ert_template_get_template_file(ert_template)")
ErtTemplate.cNamespace().get_target_file = cwrapper.prototype("char* ert_template_get_target_file(ert_template)")
ErtTemplate.cNamespace().get_args_as_string = cwrapper.prototype("char* ert_template_get_args_as_string(ert_template)")
