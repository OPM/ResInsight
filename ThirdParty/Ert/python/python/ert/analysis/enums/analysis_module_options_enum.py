#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'analysis_module_options_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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


class AnalysisModuleOptionsEnum(BaseCEnum):
    ANALYSIS_NEED_ED = None
    ANALYSIS_USE_A = None
    ANALYSIS_UPDATE_A = None
    ANALYSIS_SCALE_DATA = None
    ANALYSIS_ITERABLE = None
 
AnalysisModuleOptionsEnum.populateEnum(ANALYSIS_LIB , "analysis_module_flag_enum_iget")
AnalysisModuleOptionsEnum.registerEnum(ANALYSIS_LIB , "analysis_module_options_enum")


    
