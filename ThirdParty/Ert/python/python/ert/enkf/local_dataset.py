from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.ecl import EclRegion

class LocalDataset(BaseCClass):
    TYPE_NAME = "local_dataset"

    _alloc       = EnkfPrototype("void* local_dataset_alloc(char*)", bind = False)
    _size        = EnkfPrototype("void* local_dataset_get_size(local_dataset)")
    _has_key     = EnkfPrototype("bool  local_dataset_has_key(local_dataset, char*)")
    _free        = EnkfPrototype("void  local_dataset_free(local_dataset)")
    _name        = EnkfPrototype("char* local_dataset_get_name(local_dataset)")
    _add_node    = EnkfPrototype("void  local_dataset_add_node(local_dataset, char*)")
    _del_node    = EnkfPrototype("void  local_dataset_del_node(local_dataset, char*)")
    _active_list = EnkfPrototype("active_list_ref local_dataset_get_node_active_list(local_dataset, char*)")

    def __init__(self, name):
        raise NotImplementedError("Class can not be instantiated directly!")


    def initEnsembleConfig(self , config):
        self.ensemble_config = config


    def __len__(self):
        """ @rtype: int """
        return self._size()

    def __contains__(self , key):
        """ @rtype: bool """
        return self._has_key(key)

    def __delitem__(self, key):
        assert isinstance(key, str)
        if key in self:
            self._del_node(key)
        else:
            raise KeyError('Unknown key "%s"' % key)

    def name(self):
        return self._name()
    def getName(self):
        """ deprecated. @rtype: str """
        return self.name()

    def addNode(self, key):
        assert isinstance(key, str)
        if key in self.ensemble_config:
            if not self._has_key(key):
                self._add_node(key)
            else:
                raise KeyError('Tried to add existing data key "%s".' % key)
        else:
            raise KeyError('Tried to add data key "%s" not in ensemble.' % key)


    def addNodeWithIndex(self, key, index):
        assert isinstance(key, str)
        assert isinstance(index, int)

        self.addNode( key )
        active_list = self.getActiveList(key)
        active_list.addActiveIndex(index)


    def addField(self, key, ecl_region):
        assert isinstance(key, str)
        assert isinstance(ecl_region, EclRegion)

        self.addNode( key )
        active_list = self.getActiveList(key)
        active_region = ecl_region.getActiveList()
        for i in active_region:
            active_list.addActiveIndex(i)


    def getActiveList(self, key):
        """ @rtype: ActiveList """
        if key in self:
            return self._active_list(key)
        else:
            raise KeyError('Local key "%s" not recognized.' % key)

    def free(self):
        self._free()

    def __repr__(self):
        return 'LocalDataset(name = %s, len = %d) at 0x%x' % (self.name(), len(self), self._address())
