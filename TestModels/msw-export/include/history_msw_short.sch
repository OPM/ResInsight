-- This reservoir simulation deck is made available under the Open Database
-- License: http://opendatacommons.org/licenses/odbl/1.0/. Any rights in
-- individual contents of the database are licensed under the Database Contents
-- License: http://opendatacommons.org/licenses/dbcl/1.0/

-- Copyright (C) 2025 Equinor

-- This file is one of the include files for MSW_MODEL_1_SHORT.DATA.

-- PROD = production well 
-- -----------------------------------------------------------------------------
-- -----------------------------------------------------------------------------

-- Define data to be written to the RESTART file
RPTRST
 'BASIC=2' /



-- Define group tree hierarcy 
GRUPTREE 
---------------------------------------
   'FIELD_N'     'FIELD'  /
   'FIELD_S'     'FIELD'  /
---------------------------------------
   'PROD_N'     'FIELD_N'  /
---------------------------------------
   'PROD_S'     'FIELD_S'  /
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
     'PROD-3'    'OPEN'      'RESV'   1158.621     11.062 340625.655  5* /
/


-- 60.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'MAR' 2000 /
/



WCONHIST 
     'PROD-3'    'OPEN'      'RESV'   1200.000     13.755 531919.806  5* /
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
     'PROD-3'    'OPEN'      'RESV'    620.000      8.970 378130.400  5* /
     'PROD-4'    'OPEN'      'RESV'   1160.000     15.793 165309.833  5* /
/

-- 121.000000 days from start of simulation ( 1 'JAN' 2000 )
DATES
 1 'MAY' 2000 /
/
END
