-- This reservoir simulation deck is made available under the Open Database
-- License: http://opendatacommons.org/licenses/odbl/1.0/. Any rights in
-- individual contents of the database are licensed under the Database Contents
-- License: http://opendatacommons.org/licenses/dbcl/1.0/

-- Copyright (C) 2020 Equinor

-- This file is one of the include files for model2

-- Summary file for the base case, specific version

-- Field Data
-----------------
-- Production rates
FPR
FOPR
FWPR
FGPR
FLPR
FOPRH
FWPRH
FGPRH
FWIR
FVPR
FVIR
-- Production cumulatives
FOPT
FWPT
FGPT
FLPT
FVPT
FVIT
-- Production ratios
FGOR
FGORH
FWCT
FWCTH

FMCTP
FMCTW
FMCTG
-- Injection rates
-- Injection cumulatives
-- Pressure and fluid
-- In-place volumes
-- Number of wells

FGLIR


-----------------
--  Region data
-----------------

RPR
 1 2 / 

ROIP
 1 2 / 

RRPV
 1 2 / 

RHPV
 1 2 / 

RWIP
 1 2 / 

ROE
 1 2 / 

ROEW
 1 2 / 

ROPT
 1 2 / 

RWPT
 1 2 / 

RWIT
 1 2 / 

-- custom region ABC 

RPR__ABC
 1 2 3 / 

ROIP_ABC
 1 2 3 / 

RRPV_ABC
 1 2 3 / 

RHPV_ABC
 1 2 3 / 

RWIP_ABC
 1 2 3 / 

ROE__ABC
 1 2 3 / 

ROEW_ABC
 1 2 3 / 

ROPT_ABC
 1 2 3 / 

RWPT_ABC
 1 2 3 / 

RWIT_ABC
 1 2 3 / 

-----------------
-- Well Data
-----------------
-- Production rates
WOPR
 'B-*' 'C-*' /

WGPR
 'B-*' 'C-*' /

WWPR
 'B-*' 'C-*' /

WLPR
 'B-*' 'C-*' /

WOPRH
 'B-*' 'C-*' /

WGPRH
 'B-*' 'C-*' /

WWPRH
 'B-*' 'C-*' /

WGLIR
 'B-*' 'C-*' /

WALQ
 'B-*' 'C-*' /

-- Production cumulatives
WOPT
 'B-*' 'C-*' /

WGPT
 'B-*' 'C-*' /

WWPT
 'B-*' 'C-*' /

WLPT
 'B-*' 'C-*' /


WOPTH
 'B-*' 'C-*' /

WGPTH
 'B-*' 'C-*' /

WWPTH
 'B-*' 'C-*' /

-- Production ratios

WGOR
 'B-*' 'C-*' /

WWCT
 'B-*' 'C-*' /

WGORH
 'B-*' 'C-*' /

WWCTH
 'B-*' 'C-*' /

-- Injection Rates

WGIR
 'F-*' 'G-*' /
 
WWIR
 'F-*' 'G-*' /

WGIRH
 'F-*' 'G-*' /
 
WWIRH
 'F-*' 'G-*' /

-- Pressure

WBHP
  /

WTHP
 'B-*' 'C-*' /

-- potentials 

WGIP
 'F-*' 'G-*' /

WWIP
 'F-*' 'G-*' /


WOPP
 'B-*' 'C-*' /

WGPP
 'B-*' 'C-*' /

WWPP
 'B-*' 'C-*' /



-- Group data


GOPR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GWPR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GGPR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GLPR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GGOR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GMCTP
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GMCTW
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GMCTG
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GOPGR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GGPGR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GWPGR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GVPGR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GOPT
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GWPT
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GGPT
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GLPT
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /


GWIR
 'F1' 'G1' 'M5N'  'M5S'  'PLAT-A' /

GGIR
 'F1' 'G1' 'M5N'  'M5S'  'PLAT-A' /

GWIT
 'F1' 'G1' 'M5N'  'M5S'  'PLAT-A' /

GGIT
 'F1' 'G1' 'M5N'  'M5S'  'PLAT-A' /

GOPP
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GGPP
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GWPP
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GWPI
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GGPI
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GGLIR
 'B1' 'C1' 'M5N'  'M5S'  'PLAT-A' /

GVIR
 'M5N'  'M5S'  'PLAT-A' /

GVPR
 'M5N'  'M5S'  'PLAT-A' /
 
