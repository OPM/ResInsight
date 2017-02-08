from cwrap import BaseCClass
from ert.enkf import EnkfPrototype

class LocalObsdataNode(BaseCClass):
    TYPE_NAME = "local_obsdata_node"

    _alloc                   = EnkfPrototype("void* local_obsdata_node_alloc(char* , bool)", bind = False)
    _free                    = EnkfPrototype("void  local_obsdata_node_free(local_obsdata_node)")
    _get_key                 = EnkfPrototype("char* local_obsdata_node_get_key(local_obsdata_node)")
    _add_range               = EnkfPrototype("void  local_obsdata_node_add_range(local_obsdata_node, int, int)")
    _add_step                = EnkfPrototype("void  local_obsdata_node_add_tstep(local_obsdata_node, int)")
    _tstep_active            = EnkfPrototype("bool  local_obsdata_node_tstep_active(local_obsdata_node, int)")
    _all_timestep_active     = EnkfPrototype("bool  local_obsdata_node_all_timestep_active(local_obsdata_node)")
    _set_all_timestep_active = EnkfPrototype("void  local_obsdata_node_set_all_timestep_active(local_obsdata_node, bool)")
    _get_active_list         = EnkfPrototype("active_list_ref local_obsdata_node_get_active_list(local_obsdata_node)")

    def __init__(self, obs_key , all_timestep_active = True):
        if isinstance(obs_key, str):
            c_ptr = self._alloc(obs_key , all_timestep_active)
            if c_ptr:
                super(LocalObsdataNode, self).__init__(c_ptr)
            else:
                raise ArgumentError('Unable to construct LocalObsdataNode with key = "%s".' % obs_key)
        else:
            raise TypeError('LocalObsdataNode needs string, not %s.' % str(type(obs_key)))

    def key(self):
        return self._get_key()
    def getKey(self):
        return self.key()

    def addRange(self, step_1, step_2):
        assert isinstance(step_1, int)
        assert isinstance(step_2, int)
        self._add_range(step_1, step_2)


    def addTimeStep(self , step):
        self._add_step( step )


    def free(self):
        self._free()

    def __repr__(self):
        return 'LocalObsdataNode(key = %s) %s' % (self.key(), self._ad_str())

    def tstepActive(self , tstep):
        return self._tstep_active(tstep)


    def getActiveList(self):
        return self._get_active_list()

    def allTimeStepActive(self):
        return self._all_timestep_active()

    def setAllTimeStepActive(self, flag):
        return self._set_all_timestep_active( flag )
