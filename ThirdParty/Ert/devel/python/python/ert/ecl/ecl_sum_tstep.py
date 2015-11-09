from ert.cwrap import BaseCClass, CWrapper
from ert.ecl import ECL_LIB


class EclSumTStep(BaseCClass):
    def __init__(self, report_step, mini_step, sim_days, smspec):
        sim_seconds = sim_days * 24 * 60 * 60

        c_pointer = EclSumTStep.cNamespace().alloc(report_step, mini_step, sim_seconds, smspec)
        super(EclSumTStep, self).__init__(c_pointer)

    def getSimDays(self):
        """ @rtype: double """
        return EclSumTStep.cNamespace().get_sim_days(self)

    def getReport(self):
        """ @rtype: int """
        return EclSumTStep.cNamespace().get_report(self)

    def getMiniStep(self):
        """ @rtype: int """
        return EclSumTStep.cNamespace().get_ministep(self)

    def __getitem__(self, key):
        """ @rtype: double """
        if not key in self:
            raise KeyError("Key '%s' is not available." % key)

        return EclSumTStep.cNamespace().get_from_key(self, key)

    def __setitem__(self, key, value):
        if not key in self:
            raise KeyError("Key '%s' is not available." % key)

        EclSumTStep.cNamespace().set_from_key(self, key, value)

    def __contains__(self, key):
        return EclSumTStep.cNamespace().has_key(self, key)

    def free(self):
        EclSumTStep.cNamespace().free(self)


cwrapper = CWrapper(ECL_LIB)
cwrapper.registerObjectType("ecl_sum_tstep", EclSumTStep)

EclSumTStep.cNamespace().alloc = cwrapper.prototype("void* ecl_sum_tstep_alloc_new(int, int, float, void*)")
EclSumTStep.cNamespace().free = cwrapper.prototype("void ecl_sum_tstep_free(ecl_sum_tstep)")

EclSumTStep.cNamespace().get_sim_days = cwrapper.prototype("double ecl_sum_tstep_get_sim_days(ecl_sum_tstep)")
EclSumTStep.cNamespace().get_report = cwrapper.prototype("int ecl_sum_tstep_get_report(ecl_sum_tstep)")
EclSumTStep.cNamespace().get_ministep = cwrapper.prototype("int ecl_sum_tstep_get_ministep(ecl_sum_tstep)")

EclSumTStep.cNamespace().set_from_key = cwrapper.prototype("void ecl_sum_tstep_set_from_key(ecl_sum_tstep, char*, float)")
EclSumTStep.cNamespace().get_from_key = cwrapper.prototype("double ecl_sum_tstep_get_from_key(ecl_sum_tstep, char*)")
EclSumTStep.cNamespace().has_key = cwrapper.prototype("bool ecl_sum_tstep_has_key(ecl_sum_tstep)")