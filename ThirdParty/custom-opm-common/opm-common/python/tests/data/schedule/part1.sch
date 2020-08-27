WELSPECS
     'C-4H'    'MANI-C'   11   35  1*       'OIL'  7* /
     'B-2H'  'B1-DUMMY'   15   31  1*       'OIL'  2*      'STOP'  4* /
     'D-1H'   'MANI-D1'   22   22  1*       'OIL'  2*      'STOP'  4* /
/

COMPDAT 
-- WELL        I    J    K1  K2            Sat.        CF       DIAM        KH SKIN ND        DIR   Ro 
     'C-4H'   11   35    1    1      'OPEN'  1*     45.314      0.216   4253.571  2*         'Z'     16.503 /
     'C-4H'   11   35    2    2      'OPEN'  1*     43.674      0.216   4103.809  2*         'Z'     16.588 /
     'B-2H'   17   31    9    9      'OPEN'  1*     17.246      0.216   1285.863  2*         'X'      5.865 /
     'B-2H'   19   31    9    9      'OPEN'  1*     13.200      0.216    991.575  2*         'X'      6.044 /
     'B-2H'   20   31   10   10      'OPEN'  1*     36.540      0.216   2804.161  2*         'X'      6.593 /
     'B-2H'   21   31   10   10      'OPEN'  1*     12.052      0.216    921.178  2*         'X'      6.486 /
     'B-2H'   22   32   10   10      'OPEN'  1*     67.732      0.216   5174.542  2*         'X'      6.472 /
     'B-2H'   24   32   10   10      'OPEN'  1*     42.421      0.216   3232.419  2*         'X'      6.404 /
     'B-2H'   25   32   10   10      'OPEN'  1*     29.697      0.216   2261.930  2*         'X'      6.393 /
     'B-2H'   29   33   10   10      'OPEN'  1*     10.490      0.216    807.533  2*         'X'      6.677 /
     'D-1H'   22   22    5    5      'OPEN'  1*      5.505      0.216    510.312  2*         'Z'     15.511 /
     'D-1H'   23   22    6    6      'OPEN'  1*      0.101      0.216      9.456  2*         'Z'     16.532 /
     'D-1H'   23   22    7    7      'OPEN'  1*      4.938      0.216    452.905  2*         'Z'     14.704 /
     'D-1H'   23   22    9    9      'OPEN'  1*     19.086      0.216   1745.284  2*         'Z'     14.493 /
     'D-1H'   23   22   10   10      'OPEN'  1*     50.101      0.216   4655.453  2*         'Z'     15.689 /
     'D-1H'   23   22   11   11      'OPEN'  1*      8.974      0.216    823.585  2*         'Z'     14.751 /
     'D-1H'   23   22   12   12      'OPEN'  1*      0.479      0.216     43.304  2*         'Z'     13.707 /
     'D-1H'   23   22   13   13      'OPEN'  1*     12.603      0.216   1152.420  2*         'Z'     14.489 /
/

GRUPTREE 
   'INJE'     'FIELD'  /
   'PROD'     'FIELD'  /
   'MANI-B2'     'PROD'  /
   'MANI-B1'     'PROD'  /
   'MANI-D1'     'PROD'  /
   'MANI-D2'     'PROD'  /
   'MANI-E1'     'PROD'  /
   'MANI-E2'     'PROD'  /
   'MANI-K1'     'MANI-B1'  /
   'MANI-K2'     'MANI-D2'  /
   'MANI-C'     'INJE'  /
   'MANI-F'     'INJE'  /
   'WI-GSEG'     'INJE'  /
   'B1-DUMMY'     'MANI-B1'  /
   'D2-DUMMY'     'MANI-D2'  /
/

WCONHIST 
     'C-4H'      'OPEN'      'ORAT'      0.000      0.000      0.000  5* /
     'B-2H'      'OPEN'      'ORAT'      0.000      0.000      0.000  5* /
     'D-1H'      'OPEN'      'RESV'   4347.700      0.000 482594.703  5* /
/

WPAVE 
  1*      0.000      'WELL'       'ALL' /


GRUPNET 
    'FIELD'     20.000  5* /
     'PROD'     20.000  5* /
  'MANI-B2'  1*    8  1*        'NO'  2* /
  'MANI-B1'  1*    8  1*        'NO'  2* /
  'MANI-K1'  1* 9999  4* /
 'B1-DUMMY'  1* 9999  4* /
  'MANI-D1'  1*    8  1*        'NO'  2* /
  'MANI-D2'  1*    8  1*        'NO'  2* /
  'MANI-K2'  1* 9999  4* /
 'D2-DUMMY'  1* 9999  4* /
  'MANI-E1'  1*    9  1*        'NO'  2* /
  'MANI-E2'  1*    9  4* /
/

VAPPARS 
      0.500      0.000 /


NETBALAN 
      0.000      0.200  6* /


-- 8.000000 days from start of simulation ( 6 'NOV' 1997 )
DATES
 14 'NOV' 1997 00:00:00.001 /
/


WTEST
'C-4H'  100  'C' 10 /
/

WCONHIST 
     'D-1H'      'OPEN'      'RESV'   5601.953      0.000 634722.739  5* /
/

WCONINJE 
     'C-4H'       'GAS'  1*      'RATE' 392180.973  1* 600 /
/

GECON 
    'FIELD'   1000.000  5*        'NO'  1* /
/

-- 25.000000 days from start of simulation ( 6 'NOV' 1997 )
DATES
 1 'DEC' 1997 /
/

WCONHIST 
     'B-2H'      'OPEN'      'RESV'   2079.506      0.000 230825.037  5* /
     'D-1H'      'OPEN'      'RESV'   5433.419      0.000 651415.006  5* /
/

WCONINJE 
     'C-4H'       'GAS'  1*      'RATE' 480352.924  1*  600 /
/

-- 41.000000 days from start of simulation ( 6 'NOV' 1997 )
DATES
 17 'DEC' 1997 /
/

--      : D-2H Perforation Ile 1.3 2002 Top:  4116.90 Bot:  4136.40 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  4036.00 Bot:  4105.30 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  3978.00 Bot:  4001.00 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002 (DIVIDED)
--      : D-2H Perforation Ile 1.3 2002 Top:  4001.00 Bot:  4008.80 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  3902.00 Bot:  3963.60 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  3438.00 Bot:  3484.20 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  3368.00 Bot:  3421.90 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  3284.00 Bot:  3330.20 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  3078.00 Bot:  3093.40 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Perforation Ile 1.3 2002 Top:  3016.00 Bot:  3023.70 Diam: 0.22 Skin: 0.00
--      : >> --Ile 1.3 2002
--      : D-2H Connection 14 26  9 Perf. Len   7.70 (  8.9%)
--  WARN: D-2H Cell 14 25 9 is intersected 2 times
--      : D-2H Connection 14 25  9 Perf. Len  15.40 ( 17.4%)
--      : D-2H Connection 14 23  9 Perf. Len   4.43 (  4.9%)
--      : D-2H Connection 14 22  9 Perf. Len  56.62 ( 59.0%)
--      : D-2H Connection 14 21  9 Perf. Len  71.53 ( 84.7%)
--      : D-2H Connection 14 20  9 Perf. Len  13.72 ( 15.8%)
--      : D-2H Connection 14 15  9 Perf. Len  24.00 ( 26.4%)
--      : D-2H Connection 14 14  9 Perf. Len  69.30 ( 77.1%)
--      : D-2H Connection 14 13  9 Perf. Len  19.50 ( 20.6%)
WELSPECS 
     'D-2H'  'D2-DUMMY'   14   28  1*       'OIL'  2*      'STOP'  4* /
/

COMPDAT 
-- WELL        I    J    K1  K2            Sat.        CF       DIAM        KH SKIN ND        DIR   Ro 
     'D-2H'   14   26    9    9      'OPEN'  1*     21.450      0.216   1590.754  2*         'Y'      5.741 /
     'D-2H'   14   25    9    9      'OPEN'  1*     39.557      0.216   2921.561  2*         'Y'      5.648 /
     'D-2H'   14   23    9    9      'OPEN'  1*     10.183      0.216    748.871  2*         'Y'      5.554 /
     'D-2H'   14   22    9    9      'OPEN'  1*    121.842      0.216   8821.805  2*         'Y'      5.225 /
     'D-2H'   14   21    9    9      'OPEN'  1*    140.551      0.216  10196.747  2*         'Y'      5.266 /
     'D-2H'   14   20    9    9      'OPEN'  1*     24.486      0.216   1793.318  2*         'Y'      5.465 /
     'D-2H'   14   15    9    9      'OPEN'  1*     29.883      0.216   2344.667  2*         'Y'      7.229 /
     'D-2H'   14   14    9    9      'OPEN'  1*     82.852      0.216   6372.295  2*         'Y'      6.653 /
     'D-2H'   14   13    9    9      'OPEN'  1*     24.664      0.216   1809.697  2*         'Y'      5.504 /
/

WCONHIST 
     'B-2H'      'OPEN'      'RESV'   5359.973      0.000 594956.842  5* /
     'D-1H'      'OPEN'      'RESV'   5481.020      0.000 693727.779  5* /
     'D-2H'      'OPEN'      'RESV'    161.927      0.000  17973.921  5* /
/

WCONINJE 
     'C-4H'       'GAS'  1*      'RATE' 1129535.350  1* 600  /
/

-- 56.000000 days from start of simulation ( 6 'NOV' 1997 )

