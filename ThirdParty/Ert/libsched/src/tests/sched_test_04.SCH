SKIPREST

RPTSCHED
	RESTART=2 /

-- SET 'NO RESOLUTION' OPTION
DRSDT
 0  /

-- WELL SPECIFICATION DATA
--
--     WELL   GROUP LOCATION  BHP   PI
--     NAME   NAME    I  J   DEPTH DEFN
--			      m 	
WELSPECS
  'WWI1'  'G'  1  1  -1  'WATER'  /
  'WP1'  'G'  30 30 -1 'OIL' /
/


-- COMPLETION SPECIFICATION DATA
--
--     WELL     -LOCATION-    OPEN/ SAT CONN  WELL
--     NAME     I  J K1 K2    SHUT  TAB FACT  DIAM
--					       m2	
COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/

-- PRODUCTION WELL CONTROLS
--
--      WELL     OPEN/  CNTL   OIL   WATER    GAS    LIQU    RES    BHP.TARGET
--      NAME     SHUT   MODE  RATE    RATE   RATE    RATE    RATE   PRESS
--                          sm3/day sm3/day sm3/day sm3/day rm3/day bar

-- USED IF THE WELL IS CONTROLED BY THE OIL RATE
WCONPROD
     'WP1'  'OPEN'   'LRAT' 3* 2000   1*    30  0  0  0 /
--     'WP2'  'OPEN'   'ORAT' 2000     4*    30  0  0  0 /
--     'WP3'  'OPEN'   'ORAT' 2000     4*    30  0  0  0 /
--     'WP4'  'OPEN'   'ORAT' 2000     4*    30  0  0  0 /
/

-- INJECTION WELL CONTROLS
--
--      WELL     INJ   OPEN/  CNTL    FLOW      BHP.TARGET
--      NAME    TYPE   SHUT   MODE    RATE      PRESS
--				      sm3/day   bar
WCONINJE
       'WWI1'  'WAT'  'SHUT'  'RATE'  6000  1*    400 /
/


RPTRST
BASIC=2 /

DATES
 1 'FEB' 2004 /
/
--END0001


DATES
 1 'APR' 2005 /
/
--END0002


COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/


WCONPROD
     'WP1'  'OPEN'   'LRAT' 3* 5500   1*    30  0  0  0 /
     /
WCONINJE
       'WWI1'  'WAT'  'OPEN'  'RATE'  6000  1*    400 /
/


--TSTEP
--5*91.3125 /
--456.5625 /
DATES
 1 'JUL' 2006 /
/

--END0003

COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/


--TSTEP
--5*91.3125 /
--456.5625 /
DATES
 1 'OCT' 2007 /
/
--END0004

COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/


--TSTEP
--5*91.3125 /
--456.5625 /
DATES
 1 'JAN' 2009 /
/
--END0005

COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/


--TSTEP
--5*91.3125 /
--456.5625 /
DATES
 1 'APR' 2010 /
/
--END0006

COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/


--TSTEP
--91.3125 /
--456.5625 /
DATES
 1 'JUL' 2011 /
/
--END0007

COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/

--TSTEP
--91.3125 /
--456.5625 /

DATES
 1 'OCT' 2012 /
/
--END0008

COMPDAT
      'WWI1'     1  1  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
      'WP1'    30 30  1  5   'OPEN'  0   -1   0.5  -1  0  0 /
/

--TSTEP
--91.3125 /
DATES
 1 'JAN' 2013 /
/
--END0009

END
