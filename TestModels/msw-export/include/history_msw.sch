-- This reservoir simulation deck is made available under the Open Database
-- License: http://opendatacommons.org/licenses/odbl/1.0/. Any rights in
-- individual contents of the database are licensed under the Database Contents
-- License: http://opendatacommons.org/licenses/dbcl/1.0/

-- Copyright (C) 2018 Equinor

-- This file is one of the include files for MSW_MODEL_1.DATA.

-- INJ = injection well
-- PROD = production well 
-- -----------------------------------------------------------------------------
-- -----------------------------------------------------------------------------

-- Define data to be written to the RESTART file
RPTRST
 'BASIC=2' /


-- Define well specifications 
WELSPECS 
     'INJ-1'     'INJ_N'     1    7  1*       'GAS'  7* /
     'PROD-2'     'PROD_N'    3    2  1*       'OIL'  7* /
     'INJ-6'     'INJ_N'     6    7  1*       'WATER'  7* /
/


-- Define well connections to the grid
COMPDAT 
-- WELL        I    J    K1  K2            Sat.        CF       DIAM        KH SKIN ND        DIR   Ro 
     'INJ-1'    1    7    1    1      'OPEN'  1*     88.845      0.216  11305.989  2*         'Z'     98.722 /
     'INJ-1'    1    7    2    2      'OPEN'  1*     88.867      0.216  11308.735  2*         'Z'     98.722 /
     'PROD-2'    3    2    1    1      'OPEN'  1*   1033.807      0.216 110773.953  2*         'Y'     33.623 /
     'PROD-2'    3    3    1    1      'OPEN'  1*   1013.742      0.216 108619.070  2*         'Y'     33.614 /

     'INJ-6'    5    7    7    7      'OPEN'  1*     76.231      0.211   9700.861  2*         'Z'     98.722 /
     'INJ-6'    6    7    6    6      'OPEN'  1*     88.872      0.212  11309.282  2*         'Z'     98.722 /
     'INJ-6'    6    7    5    5      'OPEN'  1*     88.873      0.213  11309.283  2*         'Z'     98.722 /
     'INJ-6'    6    7    4    4      'OPEN'  1*     88.874      0.214  11309.284  2*         'Z'     98.722 /
     'INJ-6'    6    7    3    3      'OPEN'  1*     88.865      0.215  11308.365  2*         'Z'     98.722 /
/

-- Define multi-segment wells and their segment structure.
WELSEGS 
    'PROD-2'   2642.535   3687.165     91.753       'INC'  2* /
    2    2    1    1    235.914      1.859      0.178      0.010      0.025      5.871 /
    3    3    1    2    464.903      2.017      0.178      0.010      0.025     11.569 /
/


-- Define well connections for multi-segment wells.
COMPSEGS
    'PROD-2' /
    3    2    1    1   3687.165   4158.992  3* /
    3    3    1    1   4158.992   4616.970  3* /
/


-- Define well connection ordering 
COMPORD
 'INJ-6'  'INPUT' /
/
 

-- Define group tree hierarcy 
GRUPTREE 
---------------------------------------
   'FIELD_N'     'FIELD'  /
   'FIELD_S'     'FIELD'  /
---------------------------------------
   'PROD_N'     'FIELD_N'  /
   'INJ_N'      'FIELD_N'  /
---------------------------------------
  'PROD_S'     'FIELD_S'  /
   'INJ_S'      'FIELD_S'  /
/

-- Define well historical production rates and pressures.
WCONHIST 
     'PROD-2'      'OPEN'      'RESV'   1161.290      1.839 181847.097  5* /
/

-- Well historical observed injection rates and pressures. 
WCONINJH 
     'INJ-1'       'GAS'  'OPEN'       145477.742  5* /
     'INJ-6'     'WATER'  'OPEN'         3870.968  5* /
/

-- Activate well RFT reporting to the RFT file. 
-- RFT=repeat formation tester. 
WRFTPLT
 'INJ-6' YES NO /
/  

-- 31.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'FEB' 2000 /
/


WELSPECS 
     'PROD-3'   'PROD_S'    2    2  1*       'OIL'  7* /
/

COMPDAT 
-- WELL        I    J    K1  K2            Sat.        CF       DIAM        KH SKIN ND        DIR   Ro 
     'PROD-3'  2    2    5    5      'OPEN'  1*     88.5      0.215  11305.5  2*         'X'     98.722 /
     'PROD-3'  2    2    3    3      'OPEN'  1*     88.3      0.213  11309.3  2*         'Z'     98.722 /
     'PROD-3'  2    2    4    4      'OPEN'  1*     88.4      0.214  11309.4  2*         'Z'     98.722 /
     'PROD-3'  2    2    2    2      'OPEN'  1*     88.2      0.212  11309.2  2*         'Z'     98.722 /
     'PROD-3'  2    2    1    1      'OPEN'  1*     88.1      0.211  11306.1  2*         'Z'     98.722 /
/



WELSEGS 
    'PROD-3' 2626.327   3111.327     77.424       'INC'  2* /
    2    2    1    1      7.545      7.545      0.178      0.010      0.025      0.188 /
    3    3    1    2     15.086     15.086      0.178      0.010      0.025      0.375 /
    4    4    1    3     15.087     15.088      0.178      0.010      0.025      0.375 /
    5    5    1    4     15.088     15.087      0.178      0.010      0.025      0.375 /
    6    6    1    5     15.085     15.085      0.178      0.010      0.025      0.375 /
/

COMPSEGS 
    'PROD-3'/
    2    2    1    1   3111.330   3126.415  3* /
    2    2    2    1   3126.415   3141.502  3* /
    2    2    3    1   3141.502   3156.589  3* /
    2    2    4    1   3156.589   3171.677  3* /
    2    2    5    1   3171.677   3186.760  3* /
/


WCONHIST 
     'PROD-2'      'OPEN'      'RESV'   1200.000      1.600 188068.586  5* /
     'PROD-3'    'OPEN'      'RESV'   1158.621     11.062 340625.655  5* /
/

WCONINJH 
     'INJ-1'      'GAS'  'OPEN'       422955.414  5* /
     'INJ-6'     'WATER'  'OPEN'      	4000.000  5* /
/

-- 60.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'MAR' 2000 /
/



WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.600 187878.548  5* /
     'PROD-3'    'OPEN'      'RESV'   1200.000     13.755 531919.806  5* /
/

WCONINJH 
     'INJ-1'      'GAS' 'OPEN'     575838.871  5* /
/

-- 91.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'APR' 2000 /
/

WELSPECS 
     'PROD-4'   'PROD_S'    3    6  1*       'OIL'  7* /
/

COMPDAT 
-- WELL        I    J    K1  K2            Sat.        CF       DIAM        KH SKIN ND        DIR   Ro 
     'PROD-4'  3    6    1    1      'OPEN'  1*     88.876      0.216  11309.722  2*         'Z'     98.707 /
     'PROD-4'  3    6    2    2      'OPEN'  1*     88.970      0.216  11318.249  2*         'Z'     98.506 /
     'PROD-4'  3    6    3    3      'OPEN'  1*     88.878      0.216  11309.912  2*         'Z'     98.707 /
/

WELSEGS 
    'PROD-4' 2643.581   2643.581     65.784       'INC'  2* /
    2    2    1    1      7.573      7.522      0.178      0.010      0.025      0.188 /
    3    3    1    2     15.176     15.022      0.178      0.010      0.025      0.378 /
    4    4    1    3     15.176     15.022      0.178      0.010      0.025      0.378 /
    5    5    1    4      7.573      7.522      0.178      0.010      0.025      0.188 /
/

COMPSEGS 
    'PROD-4'/
    3    6    1    1   2643.581   2658.727  3* /
    3    6    2    1   2658.727   2673.933  3* /
    3    6    3    1   2673.933   2689.079  3* /
/


WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.600 187708.900  5* /
     'PROD-3'    'OPEN'      'RESV'    620.000      8.970 378130.400  5* /
     'PROD-4'    'OPEN'      'RESV'   1160.000     15.793 165309.833  5* /
/

WCONINJH 
     'INJ-1'      'GAS'  'OPEN'     584919.300  5* /
/

-- 121.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'MAY' 2000 /
/

WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.687 187535.774  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     10.655 433595.774  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     19.177 171032.065  5* /
/

WCONINJH 
     'INJ-1'      'GAS' 'OPEN'      633731.097  5* /
/

-- 152.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'JUN' 2000 /
/


WELSPECS 
     'INJ-5'     'INJ_S'    6    3  1*       'WATER'  7* /
/

COMPDAT 
-- WELL        I    J    K1  K2            Sat.        CF       DIAM        KH SKIN ND        DIR   Ro 
     'INJ-5'    6    3    4    4      'OPEN'  1*     88.848      0.216  11306.355  2*         'Z'     98.722 /
     'INJ-5'    6    3    5    5      'OPEN'  1*     88.871      0.216  11309.283  2*         'Z'     98.722 /
     'INJ-5'    6    3    6    6      'OPEN'  1*     88.871      0.216  11309.283  2*         'Z'     98.722 /
     'INJ-5'    6    3    7    7      'OPEN'  1*     76.232      0.216   9700.861  2*         'Z'     98.722 /
/


WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.700 187361.200  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     12.647 460440.600  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     22.153 171050.433  5* /
/


WCONINJH 
  'INJ-1'  'GAS'    'OPEN'      655081.700  265.0  1* /
  'INJ-5'   'WATER'  'OPEN'      3866.667  5* /
/

-- 182.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'JUL' 2000 /
/

WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.700 187176.226  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     15.232 467041.387  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     25.932 171066.000  5* /
/


WCONINJH 
   'INJ-5'       'WATER' 'OPEN'      4000.000  5* /
   'INJ-1'      'GAS'   'OPEN'      660226.839  265.0  1* /
/

-- 213.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'AUG' 2000 /
/

WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.713 186990.613  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     18.171 473414.742  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     30.081 171080.355  5* /
/



WCONINJH 
     'INJ-1'      'GAS'  'OPEN'      665188.548  265.0  1* /
/

-- 244.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'SEP' 2000 /
/

WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.800 186813.200  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     21.217 479838.100  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     34.480 171094.267  5* /
/


WCONINJH 
     'INJ-1'      'GAS'  'OPEN'     670196.467  265.0  1* /
/

-- 274.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'OCT' 2000 /
/

WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.813 186640.613  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     24.703 485836.806  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     39.590 171106.903  5* /
/


WCONINJH 
     'INJ-1'      'GAS'  'OPEN'     679375.033  265.0  1* /
/


-- 305.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'NOV' 2000 /
/

WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      1.903 186472.467  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     28.780 491625.633  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     45.060 171120.600  5* /
/


WCONINJH 
     'INJ-1'      'GAS'  'OPEN'    679375.033  265.0  1* /
/


-- 335.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'DEC' 2000 /
/

WCONHIST 
     'PROD-2'    'OPEN'      'RESV'   1200.000      2.014 186313.793  5* /
     'PROD-3'    'OPEN'      'RESV'    600.000     32.962 497580.966  5* /
     'PROD-4'    'OPEN'      'RESV'   1200.000     51.148 171132.586  5* /
/

WCONINJH 
     'INJ-1'      'GAS'  'OPEN'    684021.862  265.0  1* /
/



RPTRST
  'BASIC=2'  /
  
  
DATES
 30 'DEC' 2000 /
/

-- END OF SIMULATION
