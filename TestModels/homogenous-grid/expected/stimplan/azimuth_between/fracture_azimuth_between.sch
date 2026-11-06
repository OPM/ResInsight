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
--                                                               MD     Dip    Tilt    LPerf   PerfEff    Wdia           Dfac
-- Well       Fracture      Template                                                     [m]        []     [m]          [...]
-- -----------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   UNIFORM_STIMPLAN_FRACTURE      2050.000   0.000   0.000   15.000     1.000   0.216   1.372693E-04
-- 
--
--                                                                                                        Mean       Mean    Mean    Mean   Mean     Mean
--                                                                        Tr   #con     Fcd      Area     KfWf         Kf      wf      Xf      H       Km
-- Well       Fracture      Template                       [cP.rm3/day/bars]             []      [m2]    [mDm]       [mD]     [m]     [m]    [m]     [mD]
-- ---------------------------------------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   UNIFORM_STIMPLAN_FRACTURE                 25.781     18   0.137   10925.0   1000.0   100000.0   0.010   182.1   30.0   40.000
-- 
--
--
-- Maximum number of connections per well
--
-- Well          ConnCount
-- --------------------------
-- FracWell             18
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
-- Fracture_01 : 0.0558978
   FracWell   10   10   8    8    OPEN   1*    5.589779E-02   0.21600   2.168157E+00   0.00000   -6.331152E-02   'Z' /
-- Fracture_01 : 0.0678607
   FracWell   10   10   9    9    OPEN   1*    6.786073E-02   0.21600   2.632174E+00   0.00000   -5.215055E-02   'Z' /
-- Fracture_01 : 0.0752776
   FracWell   10   10   10   10   OPEN   1*    7.527756E-02   0.21600   2.919857E+00   0.00000   -4.701234E-02   'Z' /
-- Fracture_01 : 0.0752776
   FracWell   10   10   12   12   OPEN   1*    7.527756E-02   0.21600   2.919857E+00   0.00000   -4.701234E-02   'Z' /
-- Fracture_01 : 0.0678607
   FracWell   10   10   13   13   OPEN   1*    6.786073E-02   0.21600   2.632174E+00   0.00000   -5.215055E-02   'Z' /
-- Fracture_01 : 0.0558978
   FracWell   10   10   14   14   OPEN   1*    5.589779E-02   0.21600   2.168157E+00   0.00000   -6.331152E-02   'Z' /
-- Fracture_01 : 2.63029
   FracWell   11   11   8    8    OPEN   1*    2.630287E+00   0.21600   1.020233E+02   0.00000   -1.345471E-03   'Z' /
-- Fracture_01 : 3.91903
   FracWell   11   11   9    9    OPEN   1*    3.919028E+00   0.21600   1.520108E+02   0.00000   -9.030235E-04   'Z' /
-- Fracture_01 : 5.94324
   FracWell   11   11   10   10   OPEN   1*    5.943236E+00   0.21600   2.305255E+02   0.00000   -5.954626E-04   'Z' /
-- Fracture_01 : 5.94324
   FracWell   11   11   12   12   OPEN   1*    5.943236E+00   0.21600   2.305255E+02   0.00000   -5.954626E-04   'Z' /
-- Fracture_01 : 3.91903
   FracWell   11   11   13   13   OPEN   1*    3.919028E+00   0.21600   1.520108E+02   0.00000   -9.030235E-04   'Z' /
-- Fracture_01 : 2.63029
   FracWell   11   11   14   14   OPEN   1*    2.630287E+00   0.21600   1.020233E+02   0.00000   -1.345471E-03   'Z' /
-- Fracture_01 : 0.0558978
   FracWell   12   12   8    8    OPEN   1*    5.589779E-02   0.21600   2.168157E+00   0.00000   -6.331153E-02   'Z' /
-- Fracture_01 : 0.0678613
   FracWell   12   12   9    9    OPEN   1*    6.786134E-02   0.21600   2.632198E+00   0.00000   -5.215008E-02   'Z' /
-- Fracture_01 : 0.0752782
   FracWell   12   12   10   10   OPEN   1*    7.527823E-02   0.21600   2.919883E+00   0.00000   -4.701192E-02   'Z' /
-- Fracture_01 : 0.0752782
   FracWell   12   12   12   12   OPEN   1*    7.527823E-02   0.21600   2.919883E+00   0.00000   -4.701192E-02   'Z' /
-- Fracture_01 : 0.0678613
   FracWell   12   12   13   13   OPEN   1*    6.786134E-02   0.21600   2.632198E+00   0.00000   -5.215008E-02   'Z' /
-- Fracture_01 : 0.0558978
   FracWell   12   12   14   14   OPEN   1*    5.589779E-02   0.21600   2.168157E+00   0.00000   -6.331153E-02   'Z' /
/
