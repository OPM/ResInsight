
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'analysis_module_load_status_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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


from cwrap import BaseCEnum
from ert.analysis import ANALYSIS_LIB


class AnalysisModuleLoadStatusEnum(BaseCEnum):
    LOAD_OK        = None
    DLOPEN_FAILURE = None
    LOAD_SYMBOL_TABLE_NOT_FOUND = None

AnalysisModuleLoadStatusEnum.addEnum("LOAD_OK", 0)
AnalysisModuleLoadStatusEnum.addEnum("DLOPEN_FAILURE", 1)
AnalysisModuleLoadStatusEnum.addEnum("LOAD_SYMBOL_TABLE_NOT_FOUND", 2)
AnalysisModuleLoadStatusEnum.registerEnum(ANALYSIS_LIB, "analysis_module_load_status_enum")




