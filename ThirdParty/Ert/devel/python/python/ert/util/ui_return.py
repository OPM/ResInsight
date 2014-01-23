#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'ui_return.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ert.cwrap import BaseCClass , CWrapper , BaseCEnum
from ert.util import UTIL_LIB
from enums import UIReturnStatusEnum
    

class UIReturn(BaseCClass):
    
    def __init__(self , status):
        c_ptr = UIReturn.cNamespace().alloc(status)
        super(UIReturn, self).__init__(c_ptr)


    def __nonzero__(self):
        if self.status() == UIReturnStatusEnum.UI_RETURN_OK:
            return True
        else:
            return False
        
        
    def __len__(self):
        return UIReturn.cNamespace().num_error(self)
    

    def __getitem__(self , index):
        if isinstance(index , int):
            if index >= 0 and index < len(self):
                return UIReturn.cNamespace().iget_error(self , index)
            else:
                raise IndexError
        else:
            raise TypeError("Lookup type must be integer")
  
        
    def iget_error(self , index):
        return self.__getitem__(index)
  
        
    def help_text(self):
        help_text = UIReturn.cNamespace().get_help(self)
        if help_text:
            return help_text
        else:
            return ""
        
    def add_help(self, help_text):
        UIReturn.cNamespace().add_help(self , help_text)
        
        
    def status(self):
        return UIReturn.cNamespace().get_status(self)
        
    
    def __assert_error(self):
        if self.status() == UIReturnStatusEnum.UI_RETURN_OK:
            raise ValueError("Can not add error messages to object in state RETURN_OK")
    
    
    def add_error(self, error):
        self.__assert_error()
        UIReturn.cNamespace().add_error(self , error)

    
    def last_error(self):
        self.__assert_error()
        return UIReturn.cNamespace().last_error(self)
    
    
    def first_error(self):
        self.__assert_error()
        return UIReturn.cNamespace().first_error(self)
    

    def free(self):
        UIReturn.cNamespace().free(self)




CWrapper.registerType("ui_return_status" , UIReturnStatusEnum)
CWrapper.registerType("ui_return", UIReturn)
CWrapper.registerType("ui_return_obj", UIReturn.createPythonObject)
CWrapper.registerType("ui_return_ref", UIReturn.createCReference)
cwrapper = CWrapper(UTIL_LIB)

UIReturn.cNamespace().alloc = cwrapper.prototype("c_void_p ui_return_alloc( ui_return_status )")
UIReturn.cNamespace().free = cwrapper.prototype("void ui_return_free(ui_return)")
UIReturn.cNamespace().get_status = cwrapper.prototype("ui_return_status ui_return_get_status(ui_return)")
UIReturn.cNamespace().get_help = cwrapper.prototype("char* ui_return_get_help(ui_return)")
UIReturn.cNamespace().add_help = cwrapper.prototype("void ui_return_add_help(ui_return)")
UIReturn.cNamespace().add_error = cwrapper.prototype("void ui_return_add_error(ui_return)")
UIReturn.cNamespace().num_error = cwrapper.prototype("int ui_return_get_error_count(ui_return)")
UIReturn.cNamespace().last_error = cwrapper.prototype("char* ui_return_get_last_error(ui_return)")
UIReturn.cNamespace().first_error = cwrapper.prototype("char* ui_return_get_first_error(ui_return)")
UIReturn.cNamespace().iget_error = cwrapper.prototype("char* ui_return_iget_error(ui_return , int)")
