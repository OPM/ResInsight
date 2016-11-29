#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'local_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, LocalUpdateStep
from ert.enkf.local_ministep import LocalMinistep
from ert.analysis import AnalysisModule


class LocalConfig(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")
    

    # The LocalConfig class is created as a reference to an existing
    # underlying C structure by the method
    # EnkFMain.local_config(). When the pointer to the C
    # local_config_type object has been properly wrapped we 'decorate'
    # the Python object with references to the ensemble_config ,
    # observations and grid. 
    #
    # This implies that the Python object LocalConfig is richer than
    # the underlying C object local_config_type; the extra attributes
    # are only used for validation.
    
    def initAttributes(self , ensemble_config , obs , grid):
        self.ensemble_config = ensemble_config
        self.obs = obs
        self.grid = grid


    def __getObservations(self):
        return self.obs

    def __getEnsembleConfig(self):
        return self.ensemble_config

    def getGrid(self):
        # The grid can be None
        return self.grid

    
    def free(self):
        LocalConfig.cNamespace().free(self)

    def clear(self):
        LocalConfig.cNamespace().clear(self)

    def createMinistep(self, mini_step_key, analysis_module = None):
        """ @rtype: Ministep """
        assert isinstance(mini_step_key, str)
        if analysis_module:
            assert isinstance(analysis_module, AnalysisModule)
        LocalConfig.cNamespace().create_ministep(self, mini_step_key, analysis_module)         
        return self.getMinistep(mini_step_key)  
    
    def createObsdata(self, obsdata_key):
        """ @rtype: Obsdata """
        assert isinstance(obsdata_key, str)
        if LocalConfig.cNamespace().has_obsdata(self, obsdata_key):
            raise ValueError("Tried to add existing observation key:%s " % obsdata_key)
        
        LocalConfig.cNamespace().create_obsdata(self, obsdata_key)  
        obsdata = self.getObsdata(obsdata_key)
        obsdata.initObservations( self.__getObservations() )
        return obsdata

    
    def copyObsdata(self, src_key, target_key):
        """ @rtype: Obsdata """
        assert isinstance(src_key, str)
        assert isinstance(target_key, str)
        obsdata = LocalConfig.cNamespace().copy_obsdata(self, src_key, target_key)
        obsdata.initObservations( self.__getObservations() )
        return obsdata

        
    def createDataset(self, dataset_key):
        """ @rtype: Dataset """
        assert isinstance(dataset_key, str)
        if LocalConfig.cNamespace().has_dataset(self, dataset_key):
            raise ValueError("Tried to add existing data key:%s " % dataset_key)
        
        LocalConfig.cNamespace().create_dataset(self, dataset_key)  
        data = self.getDataset(dataset_key)
        data.initEnsembleConfig( self.__getEnsembleConfig() )
        return data
    
                                 
    def copyDataset(self, src_key, target_key):
        """ @rtype: Dataset """
        assert isinstance(src_key, str)
        assert isinstance(target_key, str)
        data = LocalConfig.cNamespace().copy_dataset(self, src_key, target_key)  
        data.initEnsembleConfig( self.__getEnsembleConfig() )
        return data

    
    def getUpdatestep(self):
        """ @rtype: UpdateStep """
        return LocalConfig.cNamespace().get_updatestep(self)  


    def getMinistep(self, mini_step_key):
        """ @rtype: Ministep """
        assert isinstance(mini_step_key, str)                
        return LocalConfig.cNamespace().get_ministep(self, mini_step_key)  
    
    def getObsdata(self, obsdata_key):
        """ @rtype: Obsdata """
        assert isinstance(obsdata_key, str)          
        return LocalConfig.cNamespace().get_obsdata(self, obsdata_key)    
    
    def getDataset(self, dataset_key):
        """ @rtype: Dataset """
        assert isinstance(dataset_key, str)
        return LocalConfig.cNamespace().get_dataset(self, dataset_key)
    
        
    def attachMinistep(self, update_step, mini_step):
        assert isinstance(mini_step, LocalMinistep)
        assert isinstance(update_step, LocalUpdateStep)
        LocalConfig.cNamespace().attach_ministep(update_step, mini_step)
        

    def writeSummaryFile(self, filename):                                                                                                                          
        """                                                                                                                                                    
        Writes a summary of the local config object                                                                                                            
        The summary contains the Obsset with their respective                                                                                                  
        number of observations and the Datasets with the number of active indices                                                                              
        """                                                                                                                                                    
        assert isinstance(filename, str)                                                                                                                       
        LocalConfig.cNamespace().write_local_config_summary_file(self, filename)                    
        

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("local_config", LocalConfig)

LocalConfig.cNamespace().free                            = cwrapper.prototype("void local_config_free( local_config )")
LocalConfig.cNamespace().clear                           = cwrapper.prototype("void local_config_clear( local_config )")
LocalConfig.cNamespace().get_updatestep                  = cwrapper.prototype("local_updatestep_ref local_config_get_updatestep( local_config )")
LocalConfig.cNamespace().get_ministep                    = cwrapper.prototype("local_ministep_ref local_config_get_ministep( local_config, char*)")
LocalConfig.cNamespace().create_ministep                 = cwrapper.prototype("void local_config_alloc_ministep( local_config, char*, analysis_module)")
LocalConfig.cNamespace().attach_ministep                 = cwrapper.prototype("void local_updatestep_add_ministep( local_updatestep, local_ministep)")
LocalConfig.cNamespace().get_obsdata                     = cwrapper.prototype("local_obsdata_ref local_config_get_obsdata( local_config, char*)")
LocalConfig.cNamespace().create_obsdata                  = cwrapper.prototype("void local_config_alloc_obsdata( local_config, char*)")
LocalConfig.cNamespace().copy_obsdata                    = cwrapper.prototype("local_obsdata_ref local_config_alloc_obsdata_copy( local_config, char*, char*)")
LocalConfig.cNamespace().has_obsdata                     = cwrapper.prototype("bool local_config_has_obsdata( local_config, char*)")
LocalConfig.cNamespace().get_dataset                     = cwrapper.prototype("local_dataset_ref local_config_get_dataset( local_config, char*)")
LocalConfig.cNamespace().create_dataset                  = cwrapper.prototype("void local_config_alloc_dataset( local_config, char*)")
LocalConfig.cNamespace().copy_dataset                    = cwrapper.prototype("local_dataset_ref local_config_alloc_dataset_copy( local_config, char*, char*)")
LocalConfig.cNamespace().has_dataset                     = cwrapper.prototype("bool local_config_has_dataset( local_config, char*)")

LocalConfig.cNamespace().write_local_config_summary_file = cwrapper.prototype("void local_config_summary_fprintf( local_config, char*)")




