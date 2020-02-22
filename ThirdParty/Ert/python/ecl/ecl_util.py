#  Copyright (C) 2011  Equinor ASA, Norway.
#
#  The file 'ecl_util.py' is part of ERT - Ensemble based Reservoir Tool.
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
"""
Constants from the header ecl_util.h - some stateless functions.

This module does not contain any class definitions; it mostly consists
of enum definitions/values from ecl_util.h; the enum values are
extracted from the shared library in a semi-automagic manner using the
BaseCEnum class from cwrap.

In addition to the enum definitions there are a few stateless
functions from ecl_util.c which are not bound to any class type.
"""
from __future__ import absolute_import
import ctypes

from cwrap import BaseCEnum
from ecl.util.util import monkey_the_camel
from ecl import EclPrototype

class EclFileEnum(BaseCEnum):
    TYPE_NAME = "ecl_file_enum"
    ECL_OTHER_FILE = None
    ECL_RESTART_FILE = None
    ECL_UNIFIED_RESTART_FILE = None
    ECL_SUMMARY_FILE = None
    ECL_UNIFIED_SUMMARY_FILE = None
    ECL_GRID_FILE = None
    ECL_EGRID_FILE = None
    ECL_INIT_FILE = None
    ECL_RFT_FILE = None
    ECL_DATA_FILE = None


EclFileEnum.addEnum("ECL_OTHER_FILE",           0)
EclFileEnum.addEnum("ECL_RESTART_FILE",         1)
EclFileEnum.addEnum("ECL_UNIFIED_RESTART_FILE", 2)
EclFileEnum.addEnum("ECL_SUMMARY_FILE",         4)
EclFileEnum.addEnum("ECL_UNIFIED_SUMMARY_FILE", 8)
EclFileEnum.addEnum("ECL_SUMMARY_HEADER_FILE",  16)
EclFileEnum.addEnum("ECL_GRID_FILE",            32)
EclFileEnum.addEnum("ECL_EGRID_FILE",           64)
EclFileEnum.addEnum("ECL_INIT_FILE",            128)
EclFileEnum.addEnum("ECL_RFT_FILE",             256)
EclFileEnum.addEnum("ECL_DATA_FILE",            512)


#-----------------------------------------------------------------

class EclPhaseEnum(BaseCEnum):
    TYPE_NAME="ecl_phase_enum"
    ECL_OIL_PHASE = None
    ECL_GAS_PHASE = None
    ECL_WATER_PHASE = None

EclPhaseEnum.addEnum("ECL_OIL_PHASE", 1)
EclPhaseEnum.addEnum("ECL_GAS_PHASE", 2)
EclPhaseEnum.addEnum("ECL_WATER_PHASE", 4)


#-----------------------------------------------------------------

class EclUnitTypeEnum(BaseCEnum):
    TYPE_NAME = "ecl_unit_enum"

    ECL_METRIC_UNITS = None
    ECL_FIELD_UNITS  = None
    ECL_LAB_UNITS    = None
    ECL_PVT_M_UNITS  = None

EclUnitTypeEnum.addEnum("ECL_METRIC_UNITS", 1)
EclUnitTypeEnum.addEnum("ECL_FIELD_UNITS", 2)
EclUnitTypeEnum.addEnum("ECL_LAB_UNITS", 3)
EclUnitTypeEnum.addEnum("ECL_PVT_M_UNITS", 4)



#-----------------------------------------------------------------

class EclFileFlagEnum(BaseCEnum):
    TYPE_NAME="ecl_file_flag_enum"
    ECL_FILE_CLOSE_STREAM = None
    ECL_FILE_WRITABLE = None

EclFileFlagEnum.addEnum("ECL_FILE_CLOSE_STREAM", 1)
EclFileFlagEnum.addEnum("ECL_FILE_WRITABLE", 2)


#-----------------------------------------------------------------

class EclUtil(object):
    _get_num_cpu    = EclPrototype("int ecl_util_get_num_cpu(char*)", bind = False)
    _get_file_type  = EclPrototype("ecl_file_enum ecl_util_get_file_type(char*, bool*, int*)", bind = False)
    _get_start_date = EclPrototype("time_t ecl_util_get_start_date(char*)", bind = False)
    _get_report_step = EclPrototype("int ecl_util_filename_report_nr(char*)", bind = False)


    @staticmethod
    def get_num_cpu(datafile):
        """
        Parse ECLIPSE datafile and determine how many CPUs are needed.

        Will look for the "PARALLELL" keyword, and then read off the
        number of CPUs required. Will return one if no PARALLELL keyword
        is found.
        """
        return EclUtil._get_num_cpu(datafile)

    @staticmethod
    def get_file_type(filename):
        """
        Will inspect an ECLIPSE filename and return an integer type flag.
        """
        file_type, fmt, step = EclUtil.inspectExtension(filename)
        return file_type

    @staticmethod
    def get_start_date(datafile):
        return EclUtil._get_start_date(datafile).datetime()

    @staticmethod
    def inspect_extension(filename):
        """Will inspect an ECLIPSE filename and return a tuple consisting of
        file type (EclFileEnum), a bool for formatted or not, and an
        integer for the step number.

        """
        fmt_file = ctypes.c_bool()
        report_step = ctypes.c_int(-1)
        file_type = EclUtil._get_file_type(filename, ctypes.byref(fmt_file), ctypes.byref(report_step))
        if report_step.value == -1:
            step = None
        else:
            step = report_step.value

        return (file_type, fmt_file.value, step)

    @staticmethod
    def report_step(filename):
        report_step = EclUtil._get_report_step(filename)
        if report_step < 0:
            raise ValueError("Could not infer report step from: %s" % filename)

        return report_step



get_num_cpu = EclUtil.get_num_cpu
get_file_type = EclUtil.get_file_type
get_start_date = EclUtil.get_start_date

monkey_the_camel(EclUtil, 'inspectExtension', EclUtil.inspect_extension, staticmethod)
monkey_the_camel(EclUtil, 'reportStep', EclUtil.report_step, staticmethod)
