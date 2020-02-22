#  Copyright (C) 2017  Equinor ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

from ecl.util.util import monkey_the_camel
from ecl.util.util import CTime
from ecl import EclPrototype



class EclSumTStep(BaseCClass):
    TYPE_NAME = "ecl_sum_tstep"
    _alloc        = EclPrototype("void* ecl_sum_tstep_alloc_new(int, int, float, void*)", bind=False)
    _free         = EclPrototype("void ecl_sum_tstep_free(ecl_sum_tstep)")
    _get_sim_days = EclPrototype("double ecl_sum_tstep_get_sim_days(ecl_sum_tstep)")
    _get_sim_time = EclPrototype("time_t ecl_sum_tstep_get_sim_time(ecl_sum_tstep)")
    _get_report   = EclPrototype("int ecl_sum_tstep_get_report(ecl_sum_tstep)")
    _get_ministep = EclPrototype("int ecl_sum_tstep_get_ministep(ecl_sum_tstep)")
    _set_from_key = EclPrototype("void ecl_sum_tstep_set_from_key(ecl_sum_tstep, char*, float)")
    _get_from_key = EclPrototype("double ecl_sum_tstep_get_from_key(ecl_sum_tstep, char*)")
    _has_key = EclPrototype("bool ecl_sum_tstep_has_key(ecl_sum_tstep, char*)")



    def __init__(self, report_step, mini_step, sim_days, smspec):
        sim_seconds = sim_days * 24 * 60 * 60
        c_pointer = self._alloc(report_step, mini_step, sim_seconds, smspec)
        super(EclSumTStep, self).__init__(c_pointer)


    def get_sim_days(self):
        """ @rtype: double """
        return self._get_sim_days()

    def get_report(self):
        """ @rtype: int """
        return self._get_report()

    def get_mini_step(self):
        """ @rtype: int """
        return self._get_ministep()

    def get_sim_time(self):
        """ @rtype: CTime """
        return self._get_sim_time()

    def __getitem__(self, key):
        """ @rtype: double """
        if not key in self:
            raise KeyError("Key '%s' is not available." % key)

        return self._get_from_key(key)

    def __setitem__(self, key, value):
        if not key in self:
            raise KeyError("Key '%s' is not available." % key)

        self._set_from_key(key, value)

    def __contains__(self, key):
        return self._has_key(key)

    def free(self):
        self._free(self)

    def __repr__(self):
        d = self._get_sim_days()
        t = self._get_sim_time()
        r = self._get_report()
        m = self._get_ministep()
        cnt = 'sim_days={}, sim_time={}, report={}, ministep={}'
        return self._create_repr(cnt.format(d, t, r, m))


monkey_the_camel(EclSumTStep, 'getSimDays', EclSumTStep.get_sim_days)
monkey_the_camel(EclSumTStep, 'getReport', EclSumTStep.get_report)
monkey_the_camel(EclSumTStep, 'getMiniStep', EclSumTStep.get_mini_step)
monkey_the_camel(EclSumTStep, 'getSimTime', EclSumTStep.get_sim_time)
