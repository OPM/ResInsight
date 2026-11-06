-- Exported from ResInsight 2026-11-06 12:35:08
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
--                                                               MD     Dip    Tilt   LPerf   PerfEff    Wdia           Dfac
-- Well       Fracture      Template                                                    [m]        []     [m]          [...]
-- ----------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   UNIFORM_STIMPLAN_FRACTURE      2050.000   0.000   0.000     N/A     1.000   0.216   5.057197E-03
-- 
--
--                                                                                                        Mean       Mean    Mean   Mean   Mean     Mean
--                                                                        Tr   #con     Fcd      Area     KfWf         Kf      wf     Xf      H       Km
-- Well       Fracture      Template                       [cP.rm3/day/bars]             []      [m2]    [mDm]       [mD]     [m]    [m]    [m]     [mD]
-- --------------------------------------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   UNIFORM_STIMPLAN_FRACTURE                 10.337     21   0.263   12825.0   1000.0   100000.0   0.010   95.0   67.5   40.000
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
-- Fracture_01 : 0.0389056
   FracWell   11   10   8    8    OPEN   1*    3.890558E-02   0.21600   3.763700E+00   0.00000   -1.343677E+00   'Z' /
-- Fracture_01 : 0.0459824
   FracWell   11   10   9    9    OPEN   1*    4.598237E-02   0.21600   4.448304E+00   0.00000   -1.136882E+00   'Z' /
-- Fracture_01 : 0.047975
   FracWell   11   10   10   10   OPEN   1*    4.797505E-02   0.21600   4.641074E+00   0.00000   -1.089661E+00   'Z' /
-- Fracture_01 : 0.0490206
   FracWell   11   10   11   11   OPEN   1*    4.902057E-02   0.21600   4.742217E+00   0.00000   -1.066420E+00   'Z' /
-- Fracture_01 : 0.047975
   FracWell   11   10   12   12   OPEN   1*    4.797505E-02   0.21600   4.641074E+00   0.00000   -1.089661E+00   'Z' /
-- Fracture_01 : 0.0459824
   FracWell   11   10   13   13   OPEN   1*    4.598237E-02   0.21600   4.448304E+00   0.00000   -1.136882E+00   'Z' /
-- Fracture_01 : 0.0389056
   FracWell   11   10   14   14   OPEN   1*    3.890558E-02   0.21600   3.763700E+00   0.00000   -1.343677E+00   'Z' /
-- Fracture_01 : 0.729192
   FracWell   11   11   8    8    OPEN   1*    7.291917E-01   0.21600   7.054151E+01   0.00000   -7.169107E-02   'Z' /
-- Fracture_01 : 1.10137
   FracWell   11   11   9    9    OPEN   1*    1.101365E+00   0.21600   1.065454E+02   0.00000   -4.746520E-02   'Z' /
-- Fracture_01 : 1.69748
   FracWell   11   11   10   10   OPEN   1*    1.697480E+00   0.21600   1.642131E+02   0.00000   -3.079655E-02   'Z' /
-- Fracture_01 : 2.65149
   FracWell   11   11   11   11   OPEN   1*    2.651490E+00   0.21600   2.565034E+02   0.00000   -1.971591E-02   'Z' /
-- Fracture_01 : 1.69748
   FracWell   11   11   12   12   OPEN   1*    1.697480E+00   0.21600   1.642131E+02   0.00000   -3.079655E-02   'Z' /
-- Fracture_01 : 1.10137
   FracWell   11   11   13   13   OPEN   1*    1.101365E+00   0.21600   1.065454E+02   0.00000   -4.746520E-02   'Z' /
-- Fracture_01 : 0.729192
   FracWell   11   11   14   14   OPEN   1*    7.291917E-01   0.21600   7.054151E+01   0.00000   -7.169107E-02   'Z' /
-- Fracture_01 : 0.0389056
   FracWell   11   12   8    8    OPEN   1*    3.890558E-02   0.21600   3.763700E+00   0.00000   -1.343677E+00   'Z' /
-- Fracture_01 : 0.0459824
   FracWell   11   12   9    9    OPEN   1*    4.598237E-02   0.21600   4.448304E+00   0.00000   -1.136882E+00   'Z' /
-- Fracture_01 : 0.047975
   FracWell   11   12   10   10   OPEN   1*    4.797505E-02   0.21600   4.641074E+00   0.00000   -1.089661E+00   'Z' /
-- Fracture_01 : 0.0490206
   FracWell   11   12   11   11   OPEN   1*    4.902057E-02   0.21600   4.742217E+00   0.00000   -1.066420E+00   'Z' /
-- Fracture_01 : 0.047975
   FracWell   11   12   12   12   OPEN   1*    4.797505E-02   0.21600   4.641074E+00   0.00000   -1.089661E+00   'Z' /
-- Fracture_01 : 0.0459824
   FracWell   11   12   13   13   OPEN   1*    4.598237E-02   0.21600   4.448304E+00   0.00000   -1.136882E+00   'Z' /
-- Fracture_01 : 0.0389056
   FracWell   11   12   14   14   OPEN   1*    3.890558E-02   0.21600   3.763700E+00   0.00000   -1.343677E+00   'Z' /
/
