-- Exported from ResInsight 2026-11-06 12:35:05
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
-- Ellipse                                           Xf   Height           Kf      Wf   WDiam    Skin
-- Template                    Orientation          [m]      [m]         [mD]     [m]     [m]     [] 
-- -----------------------------------------------------------------------------------------------------
-- Ellipse Fracture Template   Azimuth          100.000   75.000   100000.000   0.010   0.216   0.000
-- 
--
--
--                                 Top    Bot        Fault   Height   Half Length    DFac   Conductivity
-- Template                       Cont   Cont   Truncation    Scale         Scale   Scale          Scale
-- --------------------------------------------------------------------------------------------------------
-- Ellipse Fracture Template       N/A    N/A          N/A    1.000         1.000   1.000          1.000
-- 
--
--                                                               MD     Dip    Tilt    LPerf   PerfEff    Wdia           Dfac
-- Well       Fracture      Template                                                     [m]        []     [m]          [...]
-- -----------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   Ellipse Fracture Template      2050.000   0.000   0.000   15.000     1.000   0.216   1.372693E-04
-- 
--
--                                                                                                        Mean       Mean    Mean   Mean   Mean     Mean
--                                                                        Tr   #con     Fcd      Area     KfWf         Kf      wf     Xf      H       Km
-- Well       Fracture      Template                       [cP.rm3/day/bars]             []      [m2]    [mDm]       [mD]     [m]    [m]    [m]     [mD]
-- --------------------------------------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   Ellipse Fracture Template                 28.167     22   0.323   11620.4   1000.0   100000.0   0.010   77.5   75.0   40.000
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
-- Fracture_01 : 0.0538246
   FracWell   10   11   7    7    OPEN   1*    5.382464E-02   0.21600   1.910889E+00   0.00000   -7.183533E-02   'Z' /
-- Fracture_01 : 0.101012
   FracWell   10   11   8    8    OPEN   1*    1.010120E-01   0.21600   3.586138E+00   0.00000   -3.827776E-02   'Z' /
-- Fracture_01 : 0.124586
   FracWell   10   11   9    9    OPEN   1*    1.245865E-01   0.21600   4.423084E+00   0.00000   -3.103475E-02   'Z' /
-- Fracture_01 : 0.144963
   FracWell   10   11   10   10   OPEN   1*    1.449629E-01   0.21600   5.146488E+00   0.00000   -2.667242E-02   'Z' /
-- Fracture_01 : 0.155401
   FracWell   10   11   11   11   OPEN   1*    1.554005E-01   0.21600   5.517047E+00   0.00000   -2.488094E-02   'Z' /
-- Fracture_01 : 0.14546
   FracWell   10   11   12   12   OPEN   1*    1.454602E-01   0.21600   5.164145E+00   0.00000   -2.658123E-02   'Z' /
-- Fracture_01 : 0.0337456
   FracWell   10   11   13   13   OPEN   1*    3.374563E-02   0.21600   1.198041E+00   0.00000   -1.145781E-01   'Z' /
-- Fracture_01 : 0.342754
   FracWell   11   11   6    6    OPEN   1*    3.427538E-01   0.21600   1.216849E+01   0.00000   -1.128072E-02   'Z' /
-- Fracture_01 : 1.07135
   FracWell   11   11   7    7    OPEN   1*    1.071350E+00   0.21600   3.803518E+01   0.00000   -3.609010E-03   'Z' /
-- Fracture_01 : 1.57511
   FracWell   11   11   8    8    OPEN   1*    1.575106E+00   0.21600   5.591960E+01   0.00000   -2.454762E-03   'Z' /
-- Fracture_01 : 2.63475
   FracWell   11   11   9    9    OPEN   1*    2.634754E+00   0.21600   9.353933E+01   0.00000   -1.467504E-03   'Z' /
-- Fracture_01 : 4.61123
   FracWell   11   11   10   10   OPEN   1*    4.611233E+00   0.21600   1.637085E+02   0.00000   -8.384982E-04   'Z' /
-- Fracture_01 : 7.28072
   FracWell   11   11   11   11   OPEN   1*    7.280723E+00   0.21600   2.584811E+02   0.00000   -5.310614E-04   'Z' /
-- Fracture_01 : 5.40733
   FracWell   11   11   12   12   OPEN   1*    5.407332E+00   0.21600   1.919717E+02   0.00000   -7.150497E-04   'Z' /
-- Fracture_01 : 3.7261
   FracWell   11   11   13   13   OPEN   1*    3.726103E+00   0.21600   1.322846E+02   0.00000   -1.037682E-03   'Z' /
-- Fracture_01 : 0.0538243
   FracWell   12   11   7    7    OPEN   1*    5.382426E-02   0.21600   1.910875E+00   0.00000   -7.183584E-02   'Z' /
-- Fracture_01 : 0.101011
   FracWell   12   11   8    8    OPEN   1*    1.010115E-01   0.21600   3.586121E+00   0.00000   -3.827794E-02   'Z' /
-- Fracture_01 : 0.124586
   FracWell   12   11   9    9    OPEN   1*    1.245859E-01   0.21600   4.423063E+00   0.00000   -3.103490E-02   'Z' /
-- Fracture_01 : 0.144962
   FracWell   12   11   10   10   OPEN   1*    1.449621E-01   0.21600   5.146463E+00   0.00000   -2.667256E-02   'Z' /
-- Fracture_01 : 0.1554
   FracWell   12   11   11   11   OPEN   1*    1.553997E-01   0.21600   5.517018E+00   0.00000   -2.488107E-02   'Z' /
-- Fracture_01 : 0.145459
   FracWell   12   11   12   12   OPEN   1*    1.454593E-01   0.21600   5.164112E+00   0.00000   -2.658140E-02   'Z' /
-- Fracture_01 : 0.0337453
   FracWell   12   11   13   13   OPEN   1*    3.374529E-02   0.21600   1.198029E+00   0.00000   -1.145793E-01   'Z' /
/
