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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf import LocalUpdateStep
from ert.enkf.local_ministep import LocalMinistep
from ert.analysis import AnalysisModule


class LocalConfig(BaseCClass):
    TYPE_NAME = "local_config"

    _free            = EnkfPrototype("void local_config_free( local_config )")
    _clear           = EnkfPrototype("void local_config_clear( local_config )")
    _get_updatestep  = EnkfPrototype("local_updatestep_ref local_config_get_updatestep( local_config )")
    _get_ministep    = EnkfPrototype("local_ministep_ref local_config_get_ministep( local_config, char*)")
    _create_ministep = EnkfPrototype("void local_config_alloc_ministep( local_config, char*, analysis_module)")
    _attach_ministep = EnkfPrototype("void local_updatestep_add_ministep( local_updatestep, local_ministep)", bind = False)
    _get_obsdata     = EnkfPrototype("local_obsdata_ref local_config_get_obsdata( local_config, char*)")
    _create_obsdata  = EnkfPrototype("void local_config_alloc_obsdata( local_config, char*)")
    _copy_obsdata    = EnkfPrototype("local_obsdata_ref local_config_alloc_obsdata_copy( local_config, char*, char*)")
    _has_obsdata     = EnkfPrototype("bool local_config_has_obsdata( local_config, char*)")
    _get_dataset     = EnkfPrototype("local_dataset_ref local_config_get_dataset( local_config, char*)")
    _create_dataset  = EnkfPrototype("void local_config_alloc_dataset( local_config, char*)")
    _copy_dataset    = EnkfPrototype("local_dataset_ref local_config_alloc_dataset_copy( local_config, char*, char*)")
    _has_dataset     = EnkfPrototype("bool local_config_has_dataset( local_config, char*)")
    _write_local_config_summary_file = EnkfPrototype("void local_config_summary_fprintf( local_config, char*)")


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
        self._free()

    def clear(self):
        self._clear()

    def createMinistep(self, mini_step_key, analysis_module = None):
        """ @rtype: Ministep """
        assert isinstance(mini_step_key, str)
        if analysis_module:
            assert isinstance(analysis_module, AnalysisModule)
        self._create_ministep(mini_step_key, analysis_module)
        return self.getMinistep(mini_step_key)

    def createObsdata(self, obsdata_key):
        """ @rtype: Obsdata """
        assert isinstance(obsdata_key, str)
        if self._has_obsdata(obsdata_key):
            raise ValueError("Tried to add existing observation key:%s " % obsdata_key)

        self._create_obsdata(obsdata_key)
        obsdata = self.getObsdata(obsdata_key)
        obsdata.initObservations( self.__getObservations() )
        return obsdata


    def copyObsdata(self, src_key, target_key):
        """ @rtype: Obsdata """
        assert isinstance(src_key, str)
        assert isinstance(target_key, str)
        obsdata = self._copy_obsdata(src_key, target_key)
        obsdata.initObservations( self.__getObservations() )
        return obsdata


    def createDataset(self, dataset_key):
        """ @rtype: Dataset """
        assert isinstance(dataset_key, str)
        if self._has_dataset(dataset_key):
            raise ValueError("Tried to add existing data key:%s " % dataset_key)

        self._create_dataset(dataset_key)
        data = self.getDataset(dataset_key)
        data.initEnsembleConfig( self.__getEnsembleConfig() )
        return data


    def copyDataset(self, src_key, target_key):
        """ @rtype: Dataset """
        assert isinstance(src_key, str)
        assert isinstance(target_key, str)
        data = self._copy_dataset(src_key, target_key)
        data.initEnsembleConfig( self.__getEnsembleConfig() )
        return data


    def getUpdatestep(self):
        """ @rtype: UpdateStep """
        return self._get_updatestep()


    def getMinistep(self, mini_step_key):
        """ @rtype: Ministep """
        assert isinstance(mini_step_key, str)
        return self._get_ministep(mini_step_key)

    def getObsdata(self, obsdata_key):
        """ @rtype: Obsdata """
        assert isinstance(obsdata_key, str)
        return self._get_obsdata(obsdata_key)

    def getDataset(self, dataset_key):
        """ @rtype: Dataset """
        assert isinstance(dataset_key, str)
        return self._get_dataset(dataset_key)


    def attachMinistep(self, update_step, mini_step):
        assert isinstance(mini_step, LocalMinistep)
        assert isinstance(update_step, LocalUpdateStep)
        self._attach_ministep(update_step, mini_step)


    def writeSummaryFile(self, filename):
        """
        Writes a summary of the local config object
        The summary contains the Obsset with their respective
        number of observations and the Datasets with the number of active indices
        """
        assert isinstance(filename, str)
        self._write_local_config_summary_file(filename)
