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
-- FracWell   Fracture_01   UNIFORM_STIMPLAN_FRACTURE                 27.045     21   0.263   12825.0   1000.0   100000.0   0.010   95.0   67.5   40.000
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
-- Fracture_01 : 0.102493
   FracWell   11   10   8    8    OPEN   1*    1.024926E-01   0.21600   3.789659E+00   0.00000   -3.622208E-02   'Z' /
-- Fracture_01 : 0.120578
   FracWell   11   10   9    9    OPEN   1*    1.205785E-01   0.21600   4.458385E+00   0.00000   -3.078903E-02   'Z' /
-- Fracture_01 : 0.125021
   FracWell   11   10   10   10   OPEN   1*    1.250212E-01   0.21600   4.622656E+00   0.00000   -2.969490E-02   'Z' /
-- Fracture_01 : 0.127302
   FracWell   11   10   11   11   OPEN   1*    1.273016E-01   0.21600   4.706973E+00   0.00000   -2.916297E-02   'Z' /
-- Fracture_01 : 0.125021
   FracWell   11   10   12   12   OPEN   1*    1.250212E-01   0.21600   4.622656E+00   0.00000   -2.969490E-02   'Z' /
-- Fracture_01 : 0.120578
   FracWell   11   10   13   13   OPEN   1*    1.205785E-01   0.21600   4.458385E+00   0.00000   -3.078903E-02   'Z' /
-- Fracture_01 : 0.102493
   FracWell   11   10   14   14   OPEN   1*    1.024926E-01   0.21600   3.789659E+00   0.00000   -3.622208E-02   'Z' /
-- Fracture_01 : 2.02484
   FracWell   11   11   8    8    OPEN   1*    2.024838E+00   0.21600   7.486830E+01   0.00000   -1.833477E-03   'Z' /
-- Fracture_01 : 3.05612
   FracWell   11   11   9    9    OPEN   1*    3.056124E+00   0.21600   1.130001E+02   0.00000   -1.214772E-03   'Z' /
-- Fracture_01 : 4.70537
   FracWell   11   11   10   10   OPEN   1*    4.705369E+00   0.21600   1.739808E+02   0.00000   -7.889910E-04   'Z' /
-- Fracture_01 : 5.8257
   FracWell   11   11   11   11   OPEN   1*    5.825695E+00   0.21600   2.154049E+02   0.00000   -6.372619E-04   'Z' /
-- Fracture_01 : 4.70537
   FracWell   11   11   12   12   OPEN   1*    4.705369E+00   0.21600   1.739808E+02   0.00000   -7.889910E-04   'Z' /
-- Fracture_01 : 3.05612
   FracWell   11   11   13   13   OPEN   1*    3.056124E+00   0.21600   1.130001E+02   0.00000   -1.214772E-03   'Z' /
-- Fracture_01 : 2.02484
   FracWell   11   11   14   14   OPEN   1*    2.024838E+00   0.21600   7.486830E+01   0.00000   -1.833477E-03   'Z' /
-- Fracture_01 : 0.102493
   FracWell   11   12   8    8    OPEN   1*    1.024926E-01   0.21600   3.789659E+00   0.00000   -3.622208E-02   'Z' /
-- Fracture_01 : 0.120578
   FracWell   11   12   9    9    OPEN   1*    1.205785E-01   0.21600   4.458385E+00   0.00000   -3.078903E-02   'Z' /
-- Fracture_01 : 0.125021
   FracWell   11   12   10   10   OPEN   1*    1.250212E-01   0.21600   4.622656E+00   0.00000   -2.969490E-02   'Z' /
-- Fracture_01 : 0.127302
   FracWell   11   12   11   11   OPEN   1*    1.273016E-01   0.21600   4.706973E+00   0.00000   -2.916297E-02   'Z' /
-- Fracture_01 : 0.125021
   FracWell   11   12   12   12   OPEN   1*    1.250212E-01   0.21600   4.622656E+00   0.00000   -2.969490E-02   'Z' /
-- Fracture_01 : 0.120578
   FracWell   11   12   13   13   OPEN   1*    1.205785E-01   0.21600   4.458385E+00   0.00000   -3.078903E-02   'Z' /
-- Fracture_01 : 0.102493
   FracWell   11   12   14   14   OPEN   1*    1.024926E-01   0.21600   3.789659E+00   0.00000   -3.622208E-02   'Z' /
/
