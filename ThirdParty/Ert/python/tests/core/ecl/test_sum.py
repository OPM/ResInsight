#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
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

import os
import datetime
import csv
from unittest import skipIf, skipUnless, skipIf

from ert.ecl import EclSum, EclSumVarType
from ert.test import ExtendedTestCase, TestAreaContext
from ert.test.ecl_mock import createEclSum

def fopr(days):
    return days

def fopt(days):
    return days

def fgpt(days):
    if days < 50:
        return days
    else:
        return 100 - days

class SumTest(ExtendedTestCase):

    
    def test_mock(self):
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0)])
        self.assertTrue("FOPT" in case)
        self.assertFalse("WWCT:OPX" in case)

    def test_TIME_special_case(self):
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0)])
        keys = case.keys()
        self.assertEqual( len(keys) , 2 )
        self.assertIn( "FOPT" , keys )
        self.assertIn( "FOPR" , keys )


        keys = case.keys(pattern = "*")
        self.assertEqual( len(keys) , 2 )
        self.assertIn( "FOPT" , keys )
        self.assertIn( "FOPR" , keys )


    def test_identify_var_type(self):
        self.assertEnumIsFullyDefined( EclSumVarType , "ecl_smspec_var_type" , "libecl/include/ert/ecl/smspec_node.h")
        self.assertEqual( EclSum.varType( "WWCT:OP_X") , EclSumVarType.ECL_SMSPEC_WELL_VAR )
        self.assertEqual( EclSum.varType( "RPR") , EclSumVarType.ECL_SMSPEC_REGION_VAR )
        self.assertEqual( EclSum.varType( "WNEWTON") , EclSumVarType.ECL_SMSPEC_MISC_VAR )
        self.assertEqual( EclSum.varType( "AARQ:4") , EclSumVarType.ECL_SMSPEC_AQUIFER_VAR )

        case = createEclSum("CSV" , [("FOPT", None , 0) ,
                                     ("FOPR" , None , 0),
                                     ("AARQ" , None , 10),
                                     ("RGPT" , None  ,1)])

        node = case.smspec_node( "FOPT" )
        self.assertEqual( node.varType( ) , EclSumVarType.ECL_SMSPEC_FIELD_VAR )

        node = case.smspec_node( "AARQ:10" )
        self.assertEqual( node.varType( ) , EclSumVarType.ECL_SMSPEC_AQUIFER_VAR )
        self.assertEqual( node.getNum( ) , 10 )

        node = case.smspec_node("RGPT:1")
        self.assertEqual( node.varType( ) , EclSumVarType.ECL_SMSPEC_REGION_VAR )
        self.assertEqual( node.getNum( ) , 1 )
        self.assertTrue( node.isTotal( ))

        
        
    def test_csv_export(self):
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0)])
        sep = ";"
        with TestAreaContext("ecl/csv"):
            case.exportCSV( "file.csv" , sep = sep)
            self.assertTrue( os.path.isfile( "file.csv" ) )
            input_file = csv.DictReader( open("file.csv") , delimiter = sep )
            for row in input_file:
                self.assertIn("DAYS", row)
                self.assertIn("DATE", row)
                self.assertIn("FOPT", row)
                self.assertIn("FOPR", row)
                self.assertEqual( len(row) , 4 )
                break
                
            

        with TestAreaContext("ecl/csv"):
            case.exportCSV( "file.csv" , keys = ["FOPT"] , sep = sep)
            self.assertTrue( os.path.isfile( "file.csv" ) )
            input_file = csv.DictReader( open("file.csv") , delimiter=sep)
            for row in input_file:
                self.assertIn("DAYS", row)
                self.assertIn("DATE", row)
                self.assertIn("FOPT", row)
                self.assertEqual( len(row) , 3 )
                break
            
            

        with TestAreaContext("ecl/csv"):
            date_format = "%y-%m-%d"
            sep = ","
            case.exportCSV( "file.csv" , keys = ["F*"] , sep=sep , date_format = date_format)
            self.assertTrue( os.path.isfile( "file.csv" ) )
            with open("file.csv") as f:
                time_index = -1
                for line in f.readlines():
                    tmp = line.split( sep )
                    self.assertEqual( len(tmp) , 4)

                    if time_index >= 0:
                        d = datetime.datetime.strptime( tmp[1] , date_format )
                        self.assertEqual( case.iget_date( time_index ) , d )

                    time_index += 1
                
                
    def test_solve(self):
        length = 100
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0), ("FGPT" , None , 0)],
                            sim_length_days = length,
                            num_report_step = 10,
                            num_mini_step = 10,
                            func_table = {"FOPT" : fopt,
                                          "FOPR" : fopr ,
                                          "FGPT" : fgpt })
        
        with self.assertRaises( KeyError ):
            case.solveDays( "MISSING:KEY" , 0.56)
            
        sol = case.solveDays( "FOPT" , 150 )
        self.assertEqual( len(sol) , 0 )

        sol = case.solveDays( "FOPT" , -10 )
        self.assertEqual( len(sol) , 0 )

        sol = case.solveDays( "FOPT" , 50 )
        self.assertEqual( len(sol) , 1 )
        self.assertFloatEqual( sol[0] , 50 )

        sol = case.solveDays( "FOPT" , 50.50 )
        self.assertEqual( len(sol) , 1 )
        self.assertFloatEqual( sol[0] , 50.50 )

        sol = case.solveDays( "FOPR" , 50.90 )
        self.assertEqual( len(sol) , 1 )
        self.assertFloatEqual( sol[0] , 50.00 + 1.0/86400 )

        sol = case.solveDates("FOPR" , 50.90)
        t = case.getDataStartTime( ) + datetime.timedelta( days = 50 ) + datetime.timedelta( seconds = 1 )
        self.assertEqual( sol[0] , t )
        
        sol = case.solveDays( "FOPR" , 50.90 , rates_clamp_lower = False)
        self.assertEqual( len(sol) , 1 )
        self.assertFloatEqual( sol[0] , 51.00 )

        sol = case.solveDays( "FGPT" ,25.0)
        self.assertEqual( len(sol) , 2 )
        self.assertFloatEqual( sol[0] , 25.00 )
        self.assertFloatEqual( sol[1] , 75.00 )

        sol = case.solveDates( "FGPT" , 25 )
        self.assertEqual( len(sol) , 2 )
        t0 = case.getDataStartTime( )
        t1 = t0 + datetime.timedelta( days = 25 )
        t2 = t0 + datetime.timedelta( days = 75 )
        self.assertEqual( sol[0] , t1 )
        self.assertEqual( sol[1] , t2 )

    def test_ecl_sum_vector_algebra(self):
        scalar = 0.78
        addend = 2.718281828459045

        # setup
        length = 100
        case = createEclSum("CSV" , [("FOPT", None , 0) , ("FOPR" , None , 0), ("FGPT" , None , 0)],
                            sim_length_days = length,
                            num_report_step = 10,
                            num_mini_step = 10,
                            func_table = {"FOPT" : fopt,
                                          "FOPR" : fopr ,
                                          "FGPT" : fgpt })
        with self.assertRaises( KeyError ):
            case.scaleVector( "MISSING:KEY" , scalar)
            case.shiftVector( "MISSING:KEY" , addend)

        # scale all vectors with scalar
        for key in case.keys():
            x = case.get_values(key) # get vector key
            case.scaleVector(key , scalar)
            y = case.get_values(key)
            x = x * scalar # numpy vector scaling
            for i in range(len(x)):
                self.assertFloatEqual(x[i], y[i])

        # shift all vectors with addend
        for key in case.keys():
            x = case.get_values(key) # get vector key
            case.shiftVector(key , addend)
            y = case.get_values(key)
            x = x + addend # numpy vector shifting
            for i in range(len(x)):
                self.assertFloatEqual(x[i], y[i])
