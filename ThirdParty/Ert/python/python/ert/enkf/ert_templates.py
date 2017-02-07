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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype, ErtTemplate
from ert.util import StringList


class ErtTemplates(BaseCClass):
    TYPE_NAME = "ert_templates"

    _free         = EnkfPrototype("void ert_templates_free( ert_templates )")
    _alloc_list   = EnkfPrototype("stringlist_ref ert_templates_alloc_list(ert_templates)")
    _get_template = EnkfPrototype("ert_template_ref ert_templates_get_template(ert_templates, char*)")
    _clear        = EnkfPrototype("void ert_templates_clear(ert_templates)")
    _add_template = EnkfPrototype("ert_template_ref ert_templates_add_template(ert_templates, char*, char*, char*, char*)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getTemplateNames(self):
        """ @rtype: StringList """
        return self._alloc_list().setParent(self)

    def clear(self):
        self._clear()

    def get_template(self, key):
        """ @rtype: ErtTemplate """
        return self._get_template(key).setParent(self)

    def add_template(self, key, template_file, target_file, arg_string):
        """ @rtype: ErtTemplate """
        return self._add_template(key, template_file, target_file, arg_string).setParent(self)

    def free(self):
        self._free()
