#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_state.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf import ENKF_LIB
from ert.job_queue import JobStatusType


class EnKFState(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")
        
    
    def __getitem__(self , kw):
        """ @rtype: ert.enkf.data.enkf_node.EnkfNode """
        if isinstance(kw , str):
            if kw in self:
                node = EnKFState.cNamespace().get_node( self , kw )
                node.setParent( self )
                return node
            else:
                raise KeyError("The state object does not have node:%s" % kw)
        else:
            raise TypeError("The kw type must be string. Input:%s" % kw)


    def __contains__(self , kw):
        return EnKFState.cNamespace().has_key( self , kw )


    def hasKey(self , kw):
        """ @rtype: bool """
        return kw in self


    def getNode(self , kw):
        """ @rtype: ert.enkf.data.enkf_node.EnkfNode """
        return self[kw]


    def free(self):
        EnKFState.cNamespace().free(self)


    def addSubstKeyword(self , key , value):
        """
        Will add a key -> value pair which can be used for search replace
        operations in the data file. Observe that the key will be
        surrounded by \'<\' and \'>\'.
        """
        doc_string = None
        if isinstance(value , str):
            EnKFState.cNamespace().add_subst_kw( self , key , value , doc_string )
        else:
            raise TypeError("The value argument must be a string")

    def getDataKW(self):
        """
        Will return the substitution map for this realisation.
        """
        return EnKFState.cNamespace().get_subst_list( self )


        

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_state", EnKFState)
cwrapper.registerType("enkf_state_obj", EnKFState.createPythonObject)
cwrapper.registerType("enkf_state_ref", EnKFState.createCReference)


EnKFState.cNamespace().free     = cwrapper.prototype("void enkf_state_free( enkf_state )")
EnKFState.cNamespace().has_key  = cwrapper.prototype("bool enkf_state_has_node( enkf_state , char* )")
EnKFState.cNamespace().get_node = cwrapper.prototype("enkf_node_ref enkf_state_get_node( enkf_state , char* )")
EnKFState.cNamespace().add_subst_kw = cwrapper.prototype("void enkf_state_add_subst_kw( enkf_state , char* , char* , char*)")
EnKFState.cNamespace().get_subst_list  = cwrapper.prototype("subst_list_ref enkf_state_get_subst_list( enkf_state )")
