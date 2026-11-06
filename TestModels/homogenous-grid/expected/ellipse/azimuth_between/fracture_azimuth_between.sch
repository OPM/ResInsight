-- Exported from ResInsight 2026-11-06 12:35:06
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
--                                                                                                       Mean       Mean    Mean    Mean   Mean     Mean
--                                                                        Tr   #con     Fcd     Area     KfWf         Kf      wf      Xf      H       Km
-- Well       Fracture      Template                       [cP.rm3/day/bars]             []     [m2]    [mDm]       [mD]     [m]     [m]    [m]     [mD]
-- --------------------------------------------------------------------------------------------------------------------------------------------------------
-- FracWell   Fracture_01   Ellipse Fracture Template                 24.119     17   0.230   9775.5   1000.0   100000.0   0.010   108.6   45.0   40.000
-- 
--
--
-- Maximum number of connections per well
--
-- Well          ConnCount
-- --------------------------
-- FracWell             17
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
-- Fracture_01 : 0.00474034
   FracWell   10   10   7    7    OPEN   1*    4.740342E-03   0.21600   1.965385E-01   0.00000   -6.984347E-01   'Z' /
-- Fracture_01 : 0.0445007
   FracWell   10   10   8    8    OPEN   1*    4.450072E-02   0.21600   1.845037E+00   0.00000   -7.439924E-02   'Z' /
-- Fracture_01 : 0.0715809
   FracWell   10   10   9    9    OPEN   1*    7.158091E-02   0.21600   2.967804E+00   0.00000   -4.625283E-02   'Z' /
-- Fracture_01 : 0.0910342
   FracWell   10   10   10   10   OPEN   1*    9.103425E-02   0.21600   3.774355E+00   0.00000   -3.636894E-02   'Z' /
-- Fracture_01 : 0.0415874
   FracWell   10   10   12   12   OPEN   1*    4.158744E-02   0.21600   1.724250E+00   0.00000   -7.961104E-02   'Z' /
-- Fracture_01 : 0.391027
   FracWell   11   11   6    6    OPEN   1*    3.910273E-01   0.21600   1.621231E+01   0.00000   -8.466978E-03   'Z' /
-- Fracture_01 : 1.3713
   FracWell   11   11   7    7    OPEN   1*    1.371301E+00   0.21600   5.685528E+01   0.00000   -2.414364E-03   'Z' /
-- Fracture_01 : 2.01632
   FracWell   11   11   8    8    OPEN   1*    2.016317E+00   0.21600   8.359815E+01   0.00000   -1.642014E-03   'Z' /
-- Fracture_01 : 3.27902
   FracWell   11   11   9    9    OPEN   1*    3.279019E+00   0.21600   1.359508E+02   0.00000   -1.009698E-03   'Z' /
-- Fracture_01 : 5.6264
   FracWell   11   11   10   10   OPEN   1*    5.626404E+00   0.21600   2.332754E+02   0.00000   -5.884433E-04   'Z' /
-- Fracture_01 : 6.59148
   FracWell   11   11   12   12   OPEN   1*    6.591475E+00   0.21600   2.732880E+02   0.00000   -5.022881E-04   'Z' /
-- Fracture_01 : 4.33672
   FracWell   11   11   13   13   OPEN   1*    4.336719E+00   0.21600   1.798039E+02   0.00000   -7.634388E-04   'Z' /
-- Fracture_01 : 0.00474038
   FracWell   12   12   7    7    OPEN   1*    4.740384E-03   0.21600   1.965403E-01   0.00000   -6.984285E-01   'Z' /
-- Fracture_01 : 0.0445013
   FracWell   12   12   8    8    OPEN   1*    4.450134E-02   0.21600   1.845063E+00   0.00000   -7.439819E-02   'Z' /
-- Fracture_01 : 0.0715819
   FracWell   12   12   9    9    OPEN   1*    7.158190E-02   0.21600   2.967845E+00   0.00000   -4.625219E-02   'Z' /
-- Fracture_01 : 0.0910356
   FracWell   12   12   10   10   OPEN   1*    9.103558E-02   0.21600   3.774411E+00   0.00000   -3.636841E-02   'Z' /
-- Fracture_01 : 0.0415877
   FracWell   12   12   12   12   OPEN   1*    4.158774E-02   0.21600   1.724262E+00   0.00000   -7.961047E-02   'Z' /
/
