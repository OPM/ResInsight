#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ert_templates.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, ErtTemplate
from ert.util import StringList



class ErtTemplates(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getTemplateNames(self):
        """ @rtype: StringList """
        return ErtTemplates.cNamespace().alloc_list(self).setParent(self)

    def clear(self):
        ErtTemplates.cNamespace().clear(self)

    def get_template(self, key):
        """ @rtype: ErtTemplate """
        return ErtTemplates.cNamespace().get_template(self, key).setParent(self)

    def add_template(self, key, template_file, target_file, arg_string):
        """ @rtype: ErtTemplate """
        return ErtTemplates.cNamespace().add_template(self, key, template_file, target_file, arg_string).setParent(self)

    def free(self):
        ErtTemplates.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ert_templates", ErtTemplates)
cwrapper.registerType("ert_templates_obj", ErtTemplates.createPythonObject)
cwrapper.registerType("ert_templates_ref", ErtTemplates.createCReference)

ErtTemplates.cNamespace().free = cwrapper.prototype("void ert_templates_free( ert_templates )")
ErtTemplates.cNamespace().alloc_list = cwrapper.prototype("stringlist_ref ert_templates_alloc_list(ert_templates)")
ErtTemplates.cNamespace().get_template = cwrapper.prototype("ert_template_ref ert_templates_get_template(ert_templates, char*)")
ErtTemplates.cNamespace().clear = cwrapper.prototype("void ert_templates_clear(ert_templates)")
ErtTemplates.cNamespace().add_template = cwrapper.prototype("ert_template_ref ert_templates_add_template(ert_templates, char*, char*, char*, char*)")
