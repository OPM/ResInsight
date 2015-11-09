#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'sum_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ert.ecl import EclUtil, EclTypeEnum , EclFileEnum
from ert.test import ExtendedTestCase 


class EclUtilTest(ExtendedTestCase):

    def test_enums(self):
        source_file_path = "libecl/include/ert/ecl/ecl_util.h"
        self.assertEnumIsFullyDefined(EclFileEnum, "ecl_file_enum", source_file_path)
    
    
    def test_file_type(self):
        file_type , fmt , report = EclUtil.inspectExtension("CASE.X0078")
        self.assertEqual( file_type , EclFileEnum.ECL_RESTART_FILE )
        
