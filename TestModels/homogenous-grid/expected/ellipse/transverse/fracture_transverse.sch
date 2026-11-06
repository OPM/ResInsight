-- Exported from ResInsight 2026-11-06 12:35:04
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
--
-- Ellipse                                                                Xf   Height           Kf      Wf   WDiam    Skin
-- Template                    Orientation                               [m]      [m]         [mD]     [m]     [m]     [] 
-- --------------------------------------------------------------------------------------------------------------------------
-- Ellipse Fracture Template   Transverse (normal) to Well Path      100.000   75.000   100000.000   0.010   0.216   0.000
-- 
--
--
--                                 Top    Bot        Fault   Height   Half Length    DFac   Conductivity
-- Template                       Cont   Cont   Truncation    Scale         Scale   Scale          Scale
-- --------------------------------------------------------------------------------------------------------
-- Ellipse Fracture Template       N/A    N/A          N/A    1.000         1.000   1.000          1.000
-- 
--
--                                                               MD     Dip    Tilt   LPerf   PerfEff    Wdia           Dfac
-- Well       Fracture      Template                                                    [m]        []     [m]          [...]
-- ----------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   Ellipse Fracture Template      2050.000   0.000   0.000     N/A     1.000   0.216   5.057197E-03
-- 
--
--                                                                                                        Mean       Mean    Mean   Mean   Mean     Mean
--                                                                        Tr   #con     Fcd      Area     KfWf         Kf      wf     Xf      H       Km
-- Well       Fracture      Template                       [cP.rm3/day/bars]             []      [m2]    [mDm]       [mD]     [m]    [m]    [m]     [mD]
-- --------------------------------------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   Ellipse Fracture Template                 10.213     22   0.323   11620.4   1000.0   100000.0   0.010   77.5   75.0   40.000
-- 
--
--
-- Maximum number of connections per well
--
-- Well          ConnCount
-- --------------------------
-- FracWell             22
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
-- Fracture_01 : 0.0190035
   FracWell   11   10   7    7    OPEN   1*    1.900353E-02   0.21600   1.860784E+00   0.00000   -2.717778E+00   'Z' /
-- Fracture_01 : 0.0354808
   FracWell   11   10   8    8    OPEN   1*    3.548078E-02   0.21600   3.474200E+00   0.00000   -1.455643E+00   'Z' /
-- Fracture_01 : 0.0435398
   FracWell   11   10   9    9    OPEN   1*    4.353984E-02   0.21600   4.263326E+00   0.00000   -1.186209E+00   'Z' /
-- Fracture_01 : 0.0504569
   FracWell   11   10   10   10   OPEN   1*    5.045687E-02   0.21600   4.940626E+00   0.00000   -1.023594E+00   'Z' /
-- Fracture_01 : 0.0539786
   FracWell   11   10   11   11   OPEN   1*    5.397865E-02   0.21600   5.285471E+00   0.00000   -9.568111E-01   'Z' /
-- Fracture_01 : 0.0505248
   FracWell   11   10   12   12   OPEN   1*    5.052480E-02   0.21600   4.947278E+00   0.00000   -1.022218E+00   'Z' /
-- Fracture_01 : 0.0117278
   FracWell   11   10   13   13   OPEN   1*    1.172779E-02   0.21600   1.148360E+00   0.00000   -4.403843E+00   'Z' /
-- Fracture_01 : 0.125158
   FracWell   11   11   6    6    OPEN   1*    1.251582E-01   0.21600   1.225522E+01   0.00000   -4.126567E-01   'Z' /
-- Fracture_01 : 0.389535
   FracWell   11   11   7    7    OPEN   1*    3.895349E-01   0.21600   3.814240E+01   0.00000   -1.325873E-01   'Z' /
-- Fracture_01 : 0.572725
   FracWell   11   11   8    8    OPEN   1*    5.727248E-01   0.21600   5.607996E+01   0.00000   -9.017833E-02   'Z' /
-- Fracture_01 : 0.957649
   FracWell   11   11   9    9    OPEN   1*    9.576486E-01   0.21600   9.377085E+01   0.00000   -5.393144E-02   'Z' /
-- Fracture_01 : 1.67492
   FracWell   11   11   10   10   OPEN   1*    1.674924E+00   0.21600   1.640048E+02   0.00000   -3.083566E-02   'Z' /
-- Fracture_01 : 2.64301
   FracWell   11   11   11   11   OPEN   1*    2.643013E+00   0.21600   2.587980E+02   0.00000   -1.954110E-02   'Z' /
-- Fracture_01 : 1.96324
   FracWell   11   11   12   12   OPEN   1*    1.963243E+00   0.21600   1.922364E+02   0.00000   -2.630717E-02   'Z' /
-- Fracture_01 : 1.35698
   FracWell   11   11   13   13   OPEN   1*    1.356977E+00   0.21600   1.328722E+02   0.00000   -3.806061E-02   'Z' /
-- Fracture_01 : 0.0190035
   FracWell   11   12   7    7    OPEN   1*    1.900353E-02   0.21600   1.860784E+00   0.00000   -2.717778E+00   'Z' /
-- Fracture_01 : 0.0354808
   FracWell   11   12   8    8    OPEN   1*    3.548078E-02   0.21600   3.474200E+00   0.00000   -1.455643E+00   'Z' /
-- Fracture_01 : 0.0435398
   FracWell   11   12   9    9    OPEN   1*    4.353984E-02   0.21600   4.263326E+00   0.00000   -1.186209E+00   'Z' /
-- Fracture_01 : 0.0504569
   FracWell   11   12   10   10   OPEN   1*    5.045687E-02   0.21600   4.940626E+00   0.00000   -1.023594E+00   'Z' /
-- Fracture_01 : 0.0539786
   FracWell   11   12   11   11   OPEN   1*    5.397865E-02   0.21600   5.285471E+00   0.00000   -9.568111E-01   'Z' /
-- Fracture_01 : 0.0505248
   FracWell   11   12   12   12   OPEN   1*    5.052480E-02   0.21600   4.947278E+00   0.00000   -1.022218E+00   'Z' /
-- Fracture_01 : 0.0117278
   FracWell   11   12   13   13   OPEN   1*    1.172779E-02   0.21600   1.148360E+00   0.00000   -4.403843E+00   'Z' /
/
