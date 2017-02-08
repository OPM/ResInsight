from cwrap import BaseCClass
from ert.enkf import EnkfPrototype

class ESUpdate(BaseCClass):
    TYPE_NAME="es_update"
    _smoother_update = EnkfPrototype("bool enkf_main_smoother_update(es_update, enkf_fs, enkf_fs)")

    def __init__(self , ert):
        assert isinstance(ert , BaseCClass)
        super(ESUpdate, self).__init__(ert.from_param(ert).value , parent=ert , is_reference=True)
        self.ert = ert
        self.analysis_config = self.ert.analysisConfig( )


    def hasModule(self, name):
        """
        Will check if we have analysis module @name.
        """
        return self.analysis_config.hasModule( name )


    def getModule(self,name):
        if self.hasModule( name ):
            self.analysis_config.getModule( name )
        else:
            raise KeyError("No such module:%s " % name)


    def setGlobalStdScaling(self , weight):
        self.analysis_config.setGlobalStdScaling( weight )



    def smootherUpdate( self , data_fs , target_fs):
        return self._smoother_update(data_fs , target_fs )
