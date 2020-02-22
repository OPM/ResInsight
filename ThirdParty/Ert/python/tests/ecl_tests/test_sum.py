#  Copyright (C) 2011  Equinor ASA, Norway.
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

import os.path
import os
import inspect
import datetime
import csv
import shutil
import cwrap
import stat
import pandas

def assert_frame_equal(a,b):
    if not a.equals(b):
        raise AssertionError("Expected dataframes to be equal")

try:
    from pandas.testing import assert_frame_equal
except ImportError:
    pass

from contextlib import contextmanager
from unittest import skipIf, skipUnless

from ecl import EclUnitTypeEnum
from ecl import EclDataType
from ecl.eclfile import FortIO, openFortIO, EclKW, EclFile
from ecl.summary import EclSum, EclSumVarType, EclSumKeyWordVector
from ecl.util.test import TestAreaContext
from tests import EclTest
from ecl.util.test.ecl_mock import createEclSum


@contextmanager
def pushd(path):
    if not os.path.isdir(path):
        os.makedirs(path)
    cwd = os.getcwd()
    os.chdir(path)

    yield

    os.chdir(cwd)

def create_prediction(history, pred_path):
    restart_case = os.path.join( os.getcwd(), history.base)
    restart_step = history.last_report
    with pushd(pred_path):
        prediction = create_case( case = "PREDICTION", restart_case = restart_case, restart_step = restart_step, data_start = history.end_date)
        prediction.fwrite()


def fopr(days):
    return days

def fopt(days):
    return days

def fgpt(days):
    if days < 50:
        return days
    else:
        return 100 - days

def create_case(case = "CSV", restart_case = None, restart_step = -1, data_start = None):
    length = 100
    return createEclSum(case , [("FOPT", None , 0, "SM3") , ("FOPR" , None , 0, "SM3/DAY"), ("FGPT" , None , 0, "SM3")],
                        sim_length_days = length,
                        num_report_step = 10,
                        num_mini_step = 10,
                        data_start = data_start,
                        func_table = {"FOPT" : fopt,
                                      "FOPR" : fopr ,
                                      "FGPT" : fgpt },
                        restart_case = restart_case,
                        restart_step = restart_step)

def create_case2(case = "CSV", restart_case = None, restart_step = -1, data_start = None):
    length = 100
    return createEclSum(case , [("WOPT", "OPX" , 0, "SM3") , ("FOPR" , None , 0, "SM3/DAY"), ("BPR" , None , 10, "SM3"), ("RPR", None, 3, "BARS"), ("COPR", "OPX", 421, "BARS")],
                        sim_length_days = length,
                        num_report_step = 10,
                        num_mini_step = 10,
                        data_start = data_start,
                        func_table = {"FOPT" : fopt,
                                      "FOPR" : fopr ,
                                      "FGPT" : fgpt },
                        restart_case = restart_case,
                        restart_step = restart_step)



class SumTest(EclTest):


    def test_mock(self):
        case = createEclSum("CSV" , [("FOPT", None , 0, "SM3") , ("FOPR" , None , 0, "SM3/DAY")])
        self.assertTrue("FOPT" in case)
        self.assertFalse("WWCT:OPX" in case)

    def test_TIME_special_case(self):
        case = createEclSum("CSV" , [("FOPT", None , 0, "SM3") , ("FOPR" , None , 0, "SM3/DAY")])
        keys = case.keys()
        self.assertEqual( len(keys) , 2 )
        self.assertIn( "FOPT" , keys )
        self.assertIn( "FOPR" , keys )


        keys = case.keys(pattern = "*")
        self.assertEqual( len(keys) , 2 )
        self.assertIn( "FOPT" , keys )
        self.assertIn( "FOPR" , keys )


    def test_identify_var_type(self):
        self.assertEnumIsFullyDefined( EclSumVarType , "ecl_smspec_var_type" , "lib/include/ert/ecl/smspec_node.h")
        self.assertEqual( EclSum.varType( "WWCT:OP_X") , EclSumVarType.ECL_SMSPEC_WELL_VAR )
        self.assertEqual( EclSum.varType( "RPR") , EclSumVarType.ECL_SMSPEC_REGION_VAR )
        self.assertEqual( EclSum.varType( "WNEWTON") , EclSumVarType.ECL_SMSPEC_MISC_VAR )
        self.assertEqual( EclSum.varType( "AARQ:4") , EclSumVarType.ECL_SMSPEC_AQUIFER_VAR )

        self.assertEqual( EclSum.varType("RXFT"),  EclSumVarType.ECL_SMSPEC_REGION_2_REGION_VAR)
        self.assertEqual( EclSum.varType("RxxFT"), EclSumVarType.ECL_SMSPEC_REGION_2_REGION_VAR)
        self.assertEqual( EclSum.varType("RXFR"),  EclSumVarType.ECL_SMSPEC_REGION_2_REGION_VAR)
        self.assertEqual( EclSum.varType("RxxFR"), EclSumVarType.ECL_SMSPEC_REGION_2_REGION_VAR)
        self.assertEqual( EclSum.varType("RORFR"), EclSumVarType.ECL_SMSPEC_REGION_VAR)

        case = createEclSum("CSV" , [("FOPT", None , 0, "SM3") ,
                                     ("FOPR" , None , 0, "SM3/DAY"),
                                     ("AARQ" , None , 10, "???"),
                                     ("RGPT" , None  ,1, "SM3")])

        node1 = case.smspec_node( "FOPT" )
        self.assertEqual( node1.varType( ) , EclSumVarType.ECL_SMSPEC_FIELD_VAR )
        self.assertIsNone(node1.wgname)

        node2 = case.smspec_node( "AARQ:10" )
        self.assertEqual( node2.varType( ) , EclSumVarType.ECL_SMSPEC_AQUIFER_VAR )
        self.assertEqual( node2.getNum( ) , 10 )

        node3 = case.smspec_node("RGPT:1")
        self.assertEqual( node3.varType( ) , EclSumVarType.ECL_SMSPEC_REGION_VAR )
        self.assertEqual( node3.getNum( ) , 1 )
        self.assertTrue( node3.isTotal( ))

        self.assertLess( node1, node3 )
        self.assertGreater( node2, node3 )
        self.assertEqual( node1, node1 )
        self.assertNotEqual( node1, node2 )

        with self.assertRaises(TypeError):
            a = node1 < 1

    def test_csv_export(self):
        case = createEclSum("CSV" , [("FOPT", None , 0, "SM3") , ("FOPR" , None , 0, "SM3/DAY")])
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

            self.assertEqual(case.unit("FOPT"), "SM3")

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
        case = create_case()
        self.assert_solve( case )

    def assert_solve(self, case):
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



    def test_different_names(self):
        case = create_case()
        with TestAreaContext("sum_different"):
            case.fwrite( )
            shutil.move("CSV.SMSPEC" , "CSVX.SMSPEC")
            with self.assertRaises(IOError):
                case2 = EclSum.load( "Does/not/exist" , "CSV.UNSMRY")

            with self.assertRaises(IOError):
                case2 = EclSum.load( "CSVX.SMSPEC" , "CSVX.UNSMRY")

            case2 = EclSum.load( "CSVX.SMSPEC" , "CSV.UNSMRY" )
            self.assert_solve( case2 )
            self.assertEqual(case.unit("FOPR"), "SM3/DAY")

    def test_invalid(self):
        case = create_case()
        with TestAreaContext("sum_invalid"):
            case.fwrite( )
            with open("CASE.txt", "w") as f:
                f.write("No - this is not EclKW file ....")

            with self.assertRaises( IOError ):
                case2 = EclSum.load( "CSV.SMSPEC" , "CASE.txt" )

            with self.assertRaises( IOError ):
                case2 = EclSum.load( "CASE.txt" , "CSV.UNSMRY" )

            kw1 = EclKW("TEST1", 30, EclDataType.ECL_INT)
            kw2 = EclKW("TEST2", 30, EclDataType.ECL_INT)

            with openFortIO( "CASE.KW" , FortIO.WRITE_MODE) as f:
                kw1.fwrite( f )
                kw2.fwrite( f )

            with self.assertRaises( IOError ):
                case2 = EclSum.load( "CSV.SMSPEC" , "CASE.KW")

            with self.assertRaises( IOError ):
                case2 = EclSum.load( "CASE.KW" , "CSV.UNSMRY" )


    def test_kw_vector(self):
        case1 = create_case()
        case2 = createEclSum("CSV" , [("FOPR", None , 0, "SM3/DAY") , ("FOPT" , None , 0, "SM3"), ("FWPT" , None , 0, "SM3")],
                             sim_length_days = 100,
                             num_report_step = 10,
                             num_mini_step = 10,
                             func_table = {"FOPT" : fopt,
                                           "FOPR" : fopr ,
                                           "FWPT" : fgpt })

        kw_list = EclSumKeyWordVector( case1 )
        kw_list.add_keyword("FOPT")
        kw_list.add_keyword("FGPT")
        kw_list.add_keyword("FOPR")

        t = case1.getDataStartTime( ) + datetime.timedelta( days = 43 );
        data = case1.get_interp_row( kw_list , t )
        for d1,d2 in zip(data, [ case1.get_interp("FOPT", date = t),
                                 case1.get_interp("FOPT", date = t),
                                 case1.get_interp("FOPT", date = t) ]):

            self.assertFloatEqual(d1,d2)

        tmp = []
        for key in kw_list:
            tmp.append(key)

        for (k1,k2) in zip(kw_list,tmp):
            self.assertEqual(k1,k2)

        kw_list2 = kw_list.copy(case2)
        self.assertIn("FOPT", kw_list2)
        self.assertIn("FOPR", kw_list2)
        self.assertIn("FGPT", kw_list2)
        data2 = case2.get_interp_row( kw_list2 , t )

        self.assertEqual(len(data2), 3)
        self.assertEqual(data[0], data2[0])
        self.assertEqual(data[2], data2[2])

        with TestAreaContext("sum_vector"):
            with cwrap.open("f1.txt","w") as f:
                case1.dumpCSVLine(t, kw_list, f)

            with cwrap.open("f2.txt", "w") as f:
                case2.dumpCSVLine(t,kw_list2,f)

            with open("f1.txt") as f:
                d1 = f.readline().split(",")

            with open("f2.txt") as f:
                d2 = f.readline().split(",")

            self.assertEqual(d1[0],d2[0])
            self.assertEqual(d1[2],d2[2])
            self.assertEqual(d2[1],"")



    def test_vector_select_all(self):
        case = create_case()
        ecl_sum_vector = EclSumKeyWordVector(case, True)
        keys = case.keys()
        self.assertEqual( len(keys), len(ecl_sum_vector))
        for key in keys:
            self.assertIn(key, ecl_sum_vector)


    def test_first_last(self):
        case = create_case()
        with self.assertRaises(KeyError):
            case.last_value("NO_SUCH_KEY")
        last_fopt = case.last_value("FOPT")
        values = case.get_values("FOPT")
        self.assertEqual( last_fopt, values[-1])

        with self.assertRaises(KeyError):
            case.first_value("NO_SUCH_KEY")

        first_fopt = case.first_value("FOPT")
        self.assertEqual(first_fopt, values[0])


    def test_time_range(self):
        case = create_case()
        with self.assertRaises(ValueError):
            case.time_range(num_timestep = 1)

        time_range = case.time_range( num_timestep = 10)
        self.assertEqual( len(time_range), 10)
        self.assertEqual( time_range[0], case.get_data_start_time())
        self.assertEqual( time_range[-1], case.get_end_time())


    def test_resample(self):
        case = create_case()
        time_vector = case.alloc_time_vector( False )
        case2 = case.resample( "RS", time_vector)
        time_vector_resample = case2.alloc_time_vector(False)
        first_diff = time_vector_resample.first_neq( time_vector)
        self.assertEqual( time_vector_resample, time_vector)



    # The purpose of this test is to reproduce a slightly contrived error situation.
    #
    # 1. A history simulation is created and stored somewhere in the
    #    filesystem.
    #
    # 2. We create a prediction, which has 'RESTART' reference to
    #    the history case.
    #
    # 3. The prediction case is loaded from disk, with a cwd different from the
    #    location of the predition case.
    #
    # This configuration would previously lead to a bug in the path used to
    # resolve the history case, and the history would silently be ignored.

    def test_restart_abs_path(self):
        with TestAreaContext("restart_test"):
           history = create_case(case = "HISTORY")
           history.fwrite()

           pred_path = "prediction"
           create_prediction(history, pred_path)

           pred = EclSum(os.path.join(pred_path, "PREDICTION"))
           # The restart case has a maximum length of 72 characters, depending
           # on the path used for $TMP and so on we do not really know here if
           # the restart_case has been set or not.
           if pred.restart_case:
               self.assertTrue(isinstance(pred.restart_case, EclSum))
               self.assertEqual(pred.restart_case.case, os.path.join(os.getcwd(), history.case))
               self.assertEqual(pred.restart_step, history.last_report)

               length = pred.sim_length
               pred_times = pred.alloc_time_vector(False)
               hist_times = history.alloc_time_vector(False)

               for index in range(len(hist_times)):
                   self.assertEqual(hist_times[index], pred_times[index])




    def test_restart_too_long_history_path(self):
        with TestAreaContext("restart_test_too_fucking_long_path_for_the_eclipse_restart_keyword_1234567890123456789012345678901234567890"):
            history =  create_case(case = "HISTORY")
            history.fwrite()

            pred_path = "prediction"
            create_prediction(history, pred_path)

            pred = EclSum(os.path.join(pred_path, "PREDICTION"))
            self.assertIsNone(pred.restart_case)


    def test_restart_perm_denied(self):
        with TestAreaContext("restart_test"):
            with pushd("history/case1"):
                history =  create_case(case = "HISTORY")
                history.fwrite()

            prediction = create_case( case = "PREDICTION", restart_case = "history/case1/HISTORY", data_start = history.end_date)
            prediction.fwrite()

            os.chmod("history", 0)

            # This just tests that we can create a summary instance even if we do not
            # have access to load the history case.
            pred = EclSum("PREDICTION")

            os.chmod("history", stat.S_IRWXU)


    def test_units(self):
        case = create_case()
        self.assertEqual(case.unit_system, EclUnitTypeEnum.ECL_METRIC_UNITS)


        # We do not really have support for writing anything else than the
        # default MERIC unit system. To be able to test the read functionality
        # we therefor monkey-patch the summary files in place.
        with TestAreaContext("unit_test"):
            case = create_case("UNITS")
            case.fwrite()
            case2 = EclSum("UNITS")

            kw_list = []
            f = EclFile("UNITS.SMSPEC")
            for kw in f:
                if kw.name == "INTEHEAD":
                    kw[0] = 3
                kw_list.append(kw.copy())

            f.close()
            with openFortIO("UNITS.SMSPEC", mode = FortIO.WRITE_MODE) as f:
                for kw in kw_list:
                    kw.fwrite(f)


            case = EclSum("UNITS")
            self.assertEqual(case.unit_system, EclUnitTypeEnum.ECL_LAB_UNITS)


    def test_numpy_vector(self):
        case = create_case()

        with self.assertRaises(KeyError):
            case.numpy_vector("NO_SUCH_KEY")

        numpy_vector = case.numpy_vector("FOPT")
        self.assertEqual(len(numpy_vector), len(case))
        numpy_dates = case.numpy_dates
        self.assertEqual( numpy_dates[0].tolist(), case.getDataStartTime())
        self.assertEqual( numpy_dates[-1].tolist(), case.getEndTime())

        dates = case.dates
        self.assertEqual( dates[0], case.getDataStartTime())
        self.assertEqual( dates[-1], case.getEndTime())

        dates = [datetime.datetime(2000,1,1)] + case.dates + [datetime.datetime(2020,1,1)]
        fopr = case.numpy_vector("FOPR", time_index = dates)
        fopt = case.numpy_vector("FOPT", time_index = dates)
        self.assertEqual(len(fopt), len(dates))

        self.assertEqual(fopr[0], 0)
        self.assertEqual(fopr[-1], 0)

        self.assertEqual(fopt[0], 0)
        self.assertEqual(fopt[0], case.first_value("FOPT"))
        self.assertEqual(fopt[-1], case.last_value("FOPT"))

        with self.assertRaises(ValueError):
            v = case.numpy_vector("FOPR", time_index=dates, report_only=True)

        v = case.numpy_vector("FOPR", report_only=True)
        self.assertEqual(len(v), len(case.report_dates))


    def test_vector(self):
        case = create_case()

        # The get_vector method is extremely deprecated.
        v1 = case.get_vector("FOPT")
        v2 = case.get_vector("FOPT", report_only = True)
        s1 = sum( [x.value for x in v1 ])
        s2 = sum( [x.value for x in v2 ])

    def test_pandas(self):
        case = create_case()
        dates = [datetime.datetime(2000,1,1)] + case.dates + [datetime.datetime(2020,1,1)]
        frame = case.pandas_frame(column_keys=["FOPT", "FOPR"], time_index = dates)

        fopr = frame["FOPR"]
        fopt = frame["FOPT"]

        self.assertEqual(fopr[0], 0)
        self.assertEqual(fopr[-1], 0)

        self.assertEqual(fopt[0], 0)
        self.assertEqual(fopt[0], case.first_value("FOPT"))
        self.assertEqual(fopt[-1], case.last_value("FOPT"))


        with self.assertRaises(ValueError):
            frame = case.pandas_frame(column_keys=[])

        with self.assertRaises(ValueError):
            frame = case.pandas_frame(column_keys=["NO_KEY"])

        frame = case.pandas_frame( )
        rows, columns = frame.shape
        self.assertEqual(len(case.keys()), columns)
        self.assertEqual(len(case), rows)


    def test_csv_load(self):
        case = create_case2()
        frame = case.pandas_frame()
        ecl_sum = EclSum.from_pandas("PANDAS", frame, dims=[20,10,5])

        for key in frame.columns:
            self.assertTrue(key in ecl_sum)

        df = ecl_sum.pandas_frame()
        assert_frame_equal(frame, df)

        ecl_sum_less = EclSum.from_pandas("PANDAS", frame, dims=[20,10,5], headers=['BPR:10', 'RPR:3,1,1', 'COPR:OPX:1,2,3'])
        del frame['WOPT:OPX']
        del frame['FOPR']
        df_less = ecl_sum_less.pandas_frame()
        assert_frame_equal(frame, df_less)


    def test_total_and_rate(self):
        self.assertTrue( EclSum.is_total("FOPT"))
        self.assertTrue( EclSum.is_total("WWPT:OP_3"))
        self.assertFalse( EclSum.is_total("RPR:2"))

        self.assertTrue( EclSum.is_rate("WOPR:OP_4"))
        self.assertFalse( EclSum.is_rate("BPR:123"))
        self.assertTrue(EclSum.is_rate("FWIR"))


    def test_load_case(self):
        path = os.path.join(self.TESTDATA_ROOT, "local/ECLIPSE/cp_simple3/SIMPLE_SUMMARY3")
        case = EclSum( path )
        self.assertFloatEqual(case.sim_length, 545.0)

        fopr = case.numpy_vector("FOPR")
        for time_index,value in enumerate(fopr):
            self.assertEqual(fopr[time_index], value)

    def test_write_not_implemented(self):
        path = os.path.join(self.TESTDATA_ROOT, "local/ECLIPSE/cp_simple3/SIMPLE_SUMMARY3")
        case = EclSum( path, lazy_load=True )
        self.assertFalse(case.can_write())
        with self.assertRaises(NotImplementedError):
            case.fwrite( )


    def test_directory_conflict(self):
        with TestAreaContext("dir_conflict"):
            case = create_case("UNITS")
            case.fwrite()
            os.mkdir("UNITS")
            case2 = EclSum("./UNITS")


    def test_resample_extrapolate(self):
        """
        Test resampling of summary with extrapolate option of lower and upper boundaries enabled
        """
        from ecl.util.util import TimeVector, CTime

        time_points = TimeVector()

        path = os.path.join(self.TESTDATA_ROOT, "local/ECLIPSE/cp_simple3/SIMPLE_SUMMARY3")
        ecl_sum = EclSum( path, lazy_load=True )

        start_time = ecl_sum.get_data_start_time() - datetime.timedelta(seconds=86400)
        end_time = ecl_sum.get_end_time() + datetime.timedelta(seconds=86400)
        delta = end_time - start_time

        N = 25
        time_points.initRange( CTime(start_time),
                               CTime(end_time),
                               CTime(int(delta.total_seconds()/(N - 1))))
        time_points.append(CTime(end_time))
        resampled = ecl_sum.resample( "OUTPUT_CASE", time_points, lower_extrapolation=True, upper_extrapolation=True )

        for key in ecl_sum.keys():
            self.assertIn( key, resampled )

        self.assertEqual(ecl_sum.get_data_start_time() -  datetime.timedelta(seconds=86400), resampled.get_data_start_time())

        key_not_rate = "FOPT"
        for time_index,t in enumerate(time_points):
            if t < ecl_sum.get_data_start_time():
                self.assertFloatEqual(resampled.iget( key_not_rate, time_index), ecl_sum._get_first_value(key_not_rate))
            elif t >  ecl_sum.get_end_time():
                self.assertFloatEqual(resampled.iget( key_not_rate, time_index), ecl_sum.get_last_value( key_not_rate))
            else:
                self.assertFloatEqual(resampled.iget( key_not_rate, time_index), ecl_sum.get_interp_direct( key_not_rate, t))

        key_rate = "FOPR"
        for time_index,t in enumerate(time_points):
            if t < ecl_sum.get_data_start_time():
                self.assertFloatEqual(resampled.iget( key_rate, time_index), 0)
            elif t >  ecl_sum.get_end_time():
                self.assertFloatEqual(resampled.iget( key_rate, time_index), 0)
            else:
                self.assertFloatEqual(resampled.iget( key_rate, time_index), ecl_sum.get_interp_direct( key_rate, t))
