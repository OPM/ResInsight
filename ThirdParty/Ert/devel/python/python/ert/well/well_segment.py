from ert.cwrap import BaseCClass, CWrapper
from ert.well import ECL_WELL_LIB

class WellSegment(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")

    def free(self):
        pass

    def __str__(self):
        return "{Segment ID:%d   BranchID:%d  Length:%g}" % (self.id() , self.branchId() , self.length())

    def id(self):
        """ @rtype: int """
        return WellSegment.cNamespace().id(self)

    def linkCount(self):
        """ @rtype: int """
        return WellSegment.cNamespace().link_count(self)

    def branchId(self):
        """ @rtype: int """
        return WellSegment.cNamespace().branch_id(self)

    def outletId(self):
        """ @rtype: int """
        return WellSegment.cNamespace().outlet_id(self)

    def isActive(self):
        """ @rtype: bool """
        return WellSegment.cNamespace().active(self)

    def isMainStem(self):
        """ @rtype: bool """
        return WellSegment.cNamespace().main_stem(self)

    def isNearestWellHead(self):
        """ @rtype: bool """
        return WellSegment.cNamespace().nearest_wellhead(self)

    def depth(self):
        """ @rtype: float """
        return WellSegment.cNamespace().depth(self)

    def length(self):
        """ @rtype: float """
        return WellSegment.cNamespace().length(self)

    def totalLength(self):
        """ @rtype: float """
        return WellSegment.cNamespace().total_length(self)

    def diameter(self):
        """ @rtype: float """
        return WellSegment.cNamespace().diameter(self)


CWrapper.registerObjectType("well_segment", WellSegment)
cwrapper = CWrapper(ECL_WELL_LIB)


WellSegment.cNamespace().active = cwrapper.prototype("bool well_segment_active(well_segment)")
WellSegment.cNamespace().main_stem = cwrapper.prototype("bool well_segment_main_stem(well_segment)")
WellSegment.cNamespace().nearest_wellhead = cwrapper.prototype("bool well_segment_nearest_wellhead(well_segment)")

WellSegment.cNamespace().id = cwrapper.prototype("int well_segment_get_id(well_segment)")
WellSegment.cNamespace().link_count = cwrapper.prototype("int well_segment_get_link_count(well_segment)")
WellSegment.cNamespace().branch_id = cwrapper.prototype("int well_segment_get_branch_id(well_segment)")
WellSegment.cNamespace().outlet_id = cwrapper.prototype("int well_segment_get_outlet_id(well_segment)")

WellSegment.cNamespace().depth = cwrapper.prototype("double well_segment_get_depth(well_segment)")
WellSegment.cNamespace().length = cwrapper.prototype("double well_segment_get_length(well_segment)")
WellSegment.cNamespace().total_length = cwrapper.prototype("double well_segment_get_total_length(well_segment)")
WellSegment.cNamespace().diameter = cwrapper.prototype("double well_segment_get_diameter(well_segment)")
