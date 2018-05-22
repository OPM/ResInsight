from cwrap import BaseCClass
from ecl import EclPrototype
from ecl.well import WellState

class WellTimeLine(BaseCClass):
    TYPE_NAME = "well_time_line"
    _size = EclPrototype("int well_ts_get_size(well_time_line)")
    _name = EclPrototype("char* well_ts_get_name(well_time_line)")
    _iget = EclPrototype("well_state_ref well_ts_iget_state(well_time_line, int)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")

    def getName(self):
        return self._name()

    def __len__(self):
        """ @rtype: int """
        return self._size()


    def __getitem__(self, index):
        """
         @type index: int
         @rtype: WellState
        """

        if index < 0:
            index += len(self)

        if not 0 <= index < len(self):
            raise IndexError("Index must be in range 0 <= %d < %d" % (index, len(self)))

        return self._iget(index).setParent(self)

    def free(self):
        pass

    def __repr__(self):
        n = self.getName()
        l = len(self)
        cnt = 'name = %s, size = %d' % (n,l)
        return self._create_repr(cnt)
