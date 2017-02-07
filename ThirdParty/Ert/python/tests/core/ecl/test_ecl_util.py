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

from ert.ecl import EclGrid , EclUtil, EclTypeEnum , EclFileEnum, EclPhaseEnum, EclUnitTypeEnum
from ert.test import ExtendedTestCase 


class EclUtilTest(ExtendedTestCase):

    def test_enums(self):
        source_file_path = "libecl/include/ert/ecl/ecl_util.h"
        self.assertEnumIsFullyDefined(EclFileEnum, "ecl_file_enum", source_file_path)
        self.assertEnumIsFullyDefined(EclPhaseEnum, "ecl_phase_enum", source_file_path)
        self.assertEnumIsFullyDefined(EclUnitTypeEnum, "ert_ecl_unit_enum", source_file_path)

        # The ecl_type_enum has an extra type ECL_C010_TYPE defined in
        # C, the implementation/behavior of that type is based on
        # guessing and might very well be wrong, it is therefor not
        # exposed in Python. For this reason we have commented out the
        # default enum test, and instead test the element manually.
        #
        # self.assertEnumIsFullyDefined(EclTypeEnum, "ecl_type_enum", source_file_path)

        enum_elements = [ EclTypeEnum.ECL_CHAR_TYPE,
                          EclTypeEnum.ECL_FLOAT_TYPE,
                          EclTypeEnum.ECL_DOUBLE_TYPE,
                          EclTypeEnum.ECL_INT_TYPE,
                          EclTypeEnum.ECL_BOOL_TYPE,
                          EclTypeEnum.ECL_MESS_TYPE ]
        
        for (value , enum_elm) in zip( [0,1,2,3,4,5] , enum_elements):
            self.assertEqual( value , enum_elm.value )
            
        
        
    def test_file_type(self):
        file_type , fmt , report = EclUtil.inspectExtension("CASE.X0078")
        self.assertEqual( file_type , EclFileEnum.ECL_RESTART_FILE )
        
    def test_file_report_nr(self):
        report_nr = EclUtil.reportStep("CASE.X0080")
        self.assertEqual( report_nr , 80 )

        with self.assertRaises(ValueError):
            EclUtil.reportStep("CASE.EGRID")
            
