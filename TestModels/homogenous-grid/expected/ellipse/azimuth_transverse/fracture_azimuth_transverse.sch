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
-- FracWell   Fracture_01   Ellipse Fracture Template                 26.616     22   0.323   11620.4   1000.0   100000.0   0.010   77.5   75.0   40.000
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
-- Fracture_01 : 0.0507343
   FracWell   11   10   7    7    OPEN   1*    5.073425E-02   0.21600   1.906157E+00   0.00000   -7.201366E-02   'Z' /
-- Fracture_01 : 0.0938843
   FracWell   11   10   8    8    OPEN   1*    9.388428E-02   0.21600   3.527364E+00   0.00000   -3.891556E-02   'Z' /
-- Fracture_01 : 0.114145
   FracWell   11   10   9    9    OPEN   1*    1.141449E-01   0.21600   4.288585E+00   0.00000   -3.200807E-02   'Z' /
-- Fracture_01 : 0.131159
   FracWell   11   10   10   10   OPEN   1*    1.311595E-01   0.21600   4.927845E+00   0.00000   -2.785585E-02   'Z' /
-- Fracture_01 : 0.139401
   FracWell   11   10   11   11   OPEN   1*    1.394014E-01   0.21600   5.237505E+00   0.00000   -2.620892E-02   'Z' /
-- Fracture_01 : 0.129997
   FracWell   11   10   12   12   OPEN   1*    1.299973E-01   0.21600   4.884181E+00   0.00000   -2.810488E-02   'Z' /
-- Fracture_01 : 0.0301183
   FracWell   11   10   13   13   OPEN   1*    3.011833E-02   0.21600   1.131588E+00   0.00000   -1.213068E-01   'Z' /
-- Fracture_01 : 0.350974
   FracWell   11   11   6    6    OPEN   1*    3.509737E-01   0.21600   1.318657E+01   0.00000   -1.040978E-02   'Z' /
-- Fracture_01 : 1.08598
   FracWell   11   11   7    7    OPEN   1*    1.085984E+00   0.21600   4.080193E+01   0.00000   -3.364285E-03   'Z' /
-- Fracture_01 : 1.59685
   FracWell   11   11   8    8    OPEN   1*    1.596852E+00   0.21600   5.999595E+01   0.00000   -2.287976E-03   'Z' /
-- Fracture_01 : 2.66875
   FracWell   11   11   9    9    OPEN   1*    2.668750E+00   0.21600   1.002687E+02   0.00000   -1.369015E-03   'Z' /
-- Fracture_01 : 4.64094
   FracWell   11   11   10   10   OPEN   1*    4.640940E+00   0.21600   1.743666E+02   0.00000   -7.872454E-04   'Z' /
-- Fracture_01 : 6.1508
   FracWell   11   11   11   11   OPEN   1*    6.150800E+00   0.21600   2.310942E+02   0.00000   -5.939973E-04   'Z' /
-- Fracture_01 : 5.15571
   FracWell   11   11   12   12   OPEN   1*    5.155713E+00   0.21600   1.937073E+02   0.00000   -7.086428E-04   'Z' /
-- Fracture_01 : 3.5871
   FracWell   11   11   13   13   OPEN   1*    3.587098E+00   0.21600   1.347723E+02   0.00000   -1.018528E-03   'Z' /
-- Fracture_01 : 0.0507343
   FracWell   11   12   7    7    OPEN   1*    5.073425E-02   0.21600   1.906157E+00   0.00000   -7.201366E-02   'Z' /
-- Fracture_01 : 0.0938843
   FracWell   11   12   8    8    OPEN   1*    9.388428E-02   0.21600   3.527364E+00   0.00000   -3.891556E-02   'Z' /
-- Fracture_01 : 0.114145
   FracWell   11   12   9    9    OPEN   1*    1.141449E-01   0.21600   4.288585E+00   0.00000   -3.200807E-02   'Z' /
-- Fracture_01 : 0.131159
   FracWell   11   12   10   10   OPEN   1*    1.311595E-01   0.21600   4.927845E+00   0.00000   -2.785585E-02   'Z' /
-- Fracture_01 : 0.139401
   FracWell   11   12   11   11   OPEN   1*    1.394014E-01   0.21600   5.237505E+00   0.00000   -2.620892E-02   'Z' /
-- Fracture_01 : 0.129997
   FracWell   11   12   12   12   OPEN   1*    1.299973E-01   0.21600   4.884181E+00   0.00000   -2.810488E-02   'Z' /
-- Fracture_01 : 0.0301183
   FracWell   11   12   13   13   OPEN   1*    3.011833E-02   0.21600   1.131588E+00   0.00000   -1.213068E-01   'Z' /
/
