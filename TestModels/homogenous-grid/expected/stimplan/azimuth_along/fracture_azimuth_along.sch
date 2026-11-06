-- Exported from ResInsight 2026-11-06 12:35:07
--========================================================================================================
-- RESINSIGHT DATA
--
-- Grid Model:
-- C:\gitroot\ResInsight\TestModels\homogenous-grid\HOMOGENEOUS_20X20X20.GRDECL
--
-- Well   Location
-- ------------------
-- 
--
-- StimPlan Name               Location                                                                      
-- -------------------------------------------------------------------------------------------------------------
-- UNIFORM_STIMPLAN_FRACTURE   C:\gitroot\ResInsight\TestModels\homogenous-grid\UNIFORM_STIMPLAN_FRACTURE.xml
-- 
--
--
-- StimPlan                                     WDiam    Skin
-- Template                    Orientation        [m]     [] 
-- -------------------------------------------------------------
-- UNIFORM_STIMPLAN_FRACTURE   Azimuth          0.216   0.000
-- 
--
--                                 Top    Bot        Fault   Height   Half Length    DFac   Conductivity
-- Template                       Cont   Cont   Truncation    Scale         Scale   Scale          Scale
-- --------------------------------------------------------------------------------------------------------
-- UNIFORM_STIMPLAN_FRACTURE       N/A    N/A          N/A    1.000         1.000   1.000          1.000
-- 
--
--                                                               MD     Dip    Tilt    LPerf   PerfEff    Wdia           Dfac
-- Well       Fracture      Template                                                     [m]        []     [m]          [...]
-- -----------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   UNIFORM_STIMPLAN_FRACTURE      2050.000   0.000   0.000   15.000     1.000   0.216   1.372693E-04
-- 
--
--                                                                                                        Mean       Mean    Mean   Mean   Mean     Mean
--                                                                        Tr   #con     Fcd      Area     KfWf         Kf      wf     Xf      H       Km
-- Well       Fracture      Template                       [cP.rm3/day/bars]             []      [m2]    [mDm]       [mD]     [m]    [m]    [m]     [mD]
-- --------------------------------------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   UNIFORM_STIMPLAN_FRACTURE                 30.127     21   0.263   12825.0   1000.0   100000.0   0.010   95.0   67.5   40.000
-- 
--
--
-- Maximum number of connections per well
--
-- Well          ConnCount
-- --------------------------
-- FracWell             21
-- 
--
-- WELL      GROUP         BHP    PHASE  DRAIN  INFLOW  OPEN  CROSS  PVT    HYDS  FIP 
-- NAME      NAME   I  J   DEPTH  FLUID  AREA   EQUANS  SHUT  FLOW   TABLE  DENS  REGN
WELSPECS
   FracWell  1*     1  11  1*     OIL    1*     STD     STOP  YES    0      SEG   0    /
/
-- WELL                           OPEN   SAT   CONN           WELL      KH             SKIN      D               DIR
-- NAME       I    J    K1   K2   SHUT   TAB   FACT           DIA       FACT           FACT      FACT            PEN
COMPDAT
-- ---- Completions for completion type Fracture ----
-- Fracture_01 : 0.122311
   FracWell   10   11   8    8    OPEN   1*    1.223114E-01   0.21600   4.059881E+00   0.00000   -3.381117E-02   'Z' /
-- Fracture_01 : 0.145552
   FracWell   10   11   9    9    OPEN   1*    1.455522E-01   0.21600   4.831314E+00   0.00000   -2.841242E-02   'Z' /
-- Fracture_01 : 0.153254
   FracWell   10   11   10   10   OPEN   1*    1.532543E-01   0.21600   5.086971E+00   0.00000   -2.698449E-02   'Z' /
-- Fracture_01 : 0.157386
   FracWell   10   11   11   11   OPEN   1*    1.573858E-01   0.21600   5.224106E+00   0.00000   -2.627614E-02   'Z' /
-- Fracture_01 : 0.153254
   FracWell   10   11   12   12   OPEN   1*    1.532543E-01   0.21600   5.086971E+00   0.00000   -2.698449E-02   'Z' /
-- Fracture_01 : 0.145552
   FracWell   10   11   13   13   OPEN   1*    1.455522E-01   0.21600   4.831314E+00   0.00000   -2.841242E-02   'Z' /
-- Fracture_01 : 0.122311
   FracWell   10   11   14   14   OPEN   1*    1.223114E-01   0.21600   4.059881E+00   0.00000   -3.381117E-02   'Z' /
-- Fracture_01 : 2.10735
   FracWell   11   11   8    8    OPEN   1*    2.107347E+00   0.21600   6.994917E+01   0.00000   -1.962415E-03   'Z' /
-- Fracture_01 : 3.1868
   FracWell   11   11   9    9    OPEN   1*    3.186797E+00   0.21600   1.057794E+02   0.00000   -1.297695E-03   'Z' /
-- Fracture_01 : 4.92035
   FracWell   11   11   10   10   OPEN   1*    4.920354E+00   0.21600   1.633213E+02   0.00000   -8.404863E-04   'Z' /
-- Fracture_01 : 7.6986
   FracWell   11   11   11   11   OPEN   1*    7.698601E+00   0.21600   2.555396E+02   0.00000   -5.371743E-04   'Z' /
-- Fracture_01 : 4.92035
   FracWell   11   11   12   12   OPEN   1*    4.920354E+00   0.21600   1.633213E+02   0.00000   -8.404863E-04   'Z' /
-- Fracture_01 : 3.1868
   FracWell   11   11   13   13   OPEN   1*    3.186797E+00   0.21600   1.057794E+02   0.00000   -1.297695E-03   'Z' /
-- Fracture_01 : 2.10735
   FracWell   11   11   14   14   OPEN   1*    2.107347E+00   0.21600   6.994917E+01   0.00000   -1.962415E-03   'Z' /
-- Fracture_01 : 0.122311
   FracWell   12   11   8    8    OPEN   1*    1.223107E-01   0.21600   4.059860E+00   0.00000   -3.381135E-02   'Z' /
-- Fracture_01 : 0.145551
   FracWell   12   11   9    9    OPEN   1*    1.455514E-01   0.21600   4.831288E+00   0.00000   -2.841257E-02   'Z' /
-- Fracture_01 : 0.153253
   FracWell   12   11   10   10   OPEN   1*    1.532535E-01   0.21600   5.086942E+00   0.00000   -2.698464E-02   'Z' /
-- Fracture_01 : 0.157385
   FracWell   12   11   11   11   OPEN   1*    1.573849E-01   0.21600   5.224076E+00   0.00000   -2.627629E-02   'Z' /
-- Fracture_01 : 0.153253
   FracWell   12   11   12   12   OPEN   1*    1.532535E-01   0.21600   5.086942E+00   0.00000   -2.698464E-02   'Z' /
-- Fracture_01 : 0.145551
   FracWell   12   11   13   13   OPEN   1*    1.455514E-01   0.21600   4.831288E+00   0.00000   -2.841257E-02   'Z' /
-- Fracture_01 : 0.122311
   FracWell   12   11   14   14   OPEN   1*    1.223107E-01   0.21600   4.059860E+00   0.00000   -3.381135E-02   'Z' /
/
