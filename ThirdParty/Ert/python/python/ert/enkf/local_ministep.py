import ert.util
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype, LocalObsdata, LocalObsdataNode, LocalDataset

class LocalMinistep(BaseCClass):
    TYPE_NAME = "local_ministep"

    _alloc               = EnkfPrototype("void* local_ministep_alloc(char*)", bind = False)
    _add_node            = EnkfPrototype("void local_ministep_add_obsdata_node(local_ministep, local_obsdata_node)")
    _get_local_obs_data  = EnkfPrototype("local_obsdata_ref local_ministep_get_obsdata(local_ministep)")
    _get_local_data      = EnkfPrototype("local_dataset_ref local_ministep_get_dataset(local_ministep , char*)")
    _has_local_data      = EnkfPrototype("bool              local_ministep_has_dataset(local_ministep , char*)")
    _free                = EnkfPrototype("void local_ministep_free(local_ministep)")
    _attach_obsdata      = EnkfPrototype("void local_ministep_add_obsdata(local_ministep, local_obsdata)")
    _attach_dataset      = EnkfPrototype("void local_ministep_add_dataset(local_ministep, local_dataset)")
    _name                = EnkfPrototype("char* local_ministep_get_name(local_ministep)")
    _data_size           = EnkfPrototype("int local_ministep_get_num_dataset(local_ministep)")

    def __init__(self, ministep_key):
        raise NotImplementedError("Class can not be instantiated directly!")

    # Will used the data keys; and ignore observation keys.
    def __getitem__(self, data_key):
        if isinstance(data_key, int):
            raise TypeError('Keys must be strings, not int!')
        if data_key in self:
            return self._get_local_data(data_key)
        else:
            raise KeyError('No such data key: "%s"' % data_key)

    def __len__(self):
        return self._data_size()

    def __contains__(self , data_key):
        return self._has_local_data(data_key)

    def addNode(self, node):
        assert isinstance(node, LocalObsdataNode)
        self._add_node(node)

    def attachObsset(self, obs_set):
        assert isinstance(obs_set, LocalObsdata)
        self._attach_obsdata(obs_set)


    def attachDataset(self, dataset):
        assert isinstance(dataset, LocalDataset)
        self._attach_dataset(dataset)

    def getLocalObsData(self):
        """ @rtype: LocalObsdata """
        return self._get_local_obs_data()

    def name(self):
        return self._name()
    def getName(self):
        """ deprecated. @rtype: str """
        return self.name()

    def free(self):
        self._free()

    def __repr__(self):
        return 'LocalMinistep(name = %s, len = %d) at 0x%x' % (self.name(), len(self), self._address())
