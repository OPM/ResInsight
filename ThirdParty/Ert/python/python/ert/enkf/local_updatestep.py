from cwrap import BaseCClass
from ert.enkf import EnkfPrototype, LocalMinistep

class LocalUpdateStep(BaseCClass):
    TYPE_NAME = "local_updatestep"

    _alloc           = EnkfPrototype("void  local_updatestep_alloc(char*)", bind = False)
    _size            = EnkfPrototype("int   local_updatestep_get_num_ministep(local_updatestep)")
    _iget_ministep   = EnkfPrototype("local_ministep_ref local_updatestep_iget_ministep(local_updatestep, int)")
    _free            = EnkfPrototype("void  local_updatestep_free(local_updatestep)")
    _attach_ministep = EnkfPrototype("void  local_updatestep_add_ministep(local_updatestep, local_ministep)")
    _name            = EnkfPrototype("char* local_updatestep_get_name(local_updatestep)")

    def __init__(self, updatestep_key):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __len__(self):
        """ @rtype: int """
        return self._size()

    def __getitem__(self, index):
        """ @rtype: LocalMinistep """
        if not isinstance(index, int):
            raise TypeError('Keys must be ints, not %s' % str(type(index)))
        if index < 0:
            index += len(self)
        if 0 <= index < len(self):
            return self._iget_ministep(index)
        else:
            raise IndexError('Invalid index, valid range: [0, %d)' % len(self))

    def attachMinistep(self, ministep):
        assert isinstance(ministep, LocalMinistep)
        self._attach_ministep(ministep)

    def name(self):
        return self._name()
    def getName(self):
        """ deprecated. @rtype: str """
        return self.name()

    def free(self):
        self._free(self)
