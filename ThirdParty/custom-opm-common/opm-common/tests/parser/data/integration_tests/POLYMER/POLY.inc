-- This reservoir simulation deck is made available under the Open Database
-- License: http://opendatacommons.org/licenses/odbl/1.0/. Any rights in
-- individual contents of the database are licensed under the Database Contents
-- License: http://opendatacommons.org/licenses/dbcl/1.0/

-- Copyright (C) 2015 Statoil
-- Copyright (C) 2015 SINTEF ICT, Applied Mathematics
RUNSPEC

TABDIMS
 1  1 /

EQLDIMS
 /
 

REGDIMS
 9* 1 /


PROPS
NOECHO
PLYVISC
0      1.0
0.5    3.0
1.0    6.0
1.5   12.0
2.0   24.0
3.0   48.0 /


PLYROCK
--IPV  RRF  dens  AI  max ads
 0.05  1.2  2500  2  0.000025 /


PLYADS
-- conc  adsorb-conc
 0.000   0.000000
 0.250   0.000012
 0.500   0.000016
 0.750   0.000019
 1.000   0.000020
 1.250   0.000021
 1.500   0.000023
 1.750   0.000025
 3.000   0.000025 /

PLMIXPAR
 1.0  /

PLYMAX
 3.00  0  /

ECHO
