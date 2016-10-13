from cwrap import BaseCClass, CWrapper
from ert.well import ECL_WELL_LIB, WellState

class WellTimeLine(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")


    def __len__(self):
        """ @rtype: int """
        return WellTimeLine.cNamespace().size(self)


    def __getitem__(self, index):
        """
         @type index: int
         @rtype: WellState
        """

        if index < 0:
            index += len(self)

        if not 0 <= index < len(self):
            raise IndexError("Index must be in range 0 <= %d < %d" % (index, len(self)))

        return WellTimeLine.cNamespace().iget(self, index).setParent(self)


    def free(self):
        pass

CWrapper.registerObjectType("well_time_line", WellTimeLine)
cwrapper = CWrapper(ECL_WELL_LIB)


WellTimeLine.cNamespace().size = cwrapper.prototype("int well_ts_get_size(well_time_line)")
WellTimeLine.cNamespace().iget = cwrapper.prototype("well_state_ref well_ts_iget_state(well_time_line, int)")
