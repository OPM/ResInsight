-- This reservoir simulation deck is made available under the Open Database
-- License: http://opendatacommons.org/licenses/odbl/1.0/. Any rights in
-- individual contents of the database are licensed under the Database Contents
-- License: http://opendatacommons.org/licenses/dbcl/1.0/

-- Copyright (C) 2015 Statoil

-- This simulation is based on the data given in 
-- 'Comparison of Solutions to a Three-Dimensional
-- Black-Oil Reservoir Simulation Problem' by Aziz S. Odeh,
-- Journal of Petroleum Technology, January 1981



PROPS
-- -------------------------------------------------------------------------

PVTW
-- Item 1: pressure reference (psia)
-- Item 2: water FVF (rb per bbl or rb per stb)
-- Item 3: water compressibility (psi^{-1})
-- Item 4: water viscosity (cp)
-- Item 5: water 'viscosibility' (psi^{-1})

-- Using values from Norne:
-- In METRIC units:
-- 	277.0 1.038 4.67E-5 0.318 0.0 /
-- In FIELD units:
    	4017.55 1.038 3.22E-6 0.318 0.0 /

ROCK
-- Item 1: reference pressure (psia)
-- Item 2: rock compressibility (psi^{-1})

-- Using values from table 1 in Odeh:
	14.7 3E-6 /

SWOF
-- Column 1: water saturation
--   	     - this has been set to (almost) equally spaced values from 0.12 to 1
-- Column 2: water relative permeability
--   	     - generated from the Corey-type approx. formula
--	       the coeffisient is set to 10e-5, S_{orw}=0 and S_{wi}=0.12
-- Column 3: oil relative permeability when only oil and water are present
--	     - we will use the same values as in column 3 in SGOF.
-- 	       This is not really correct, but since only the first 
--	       two values are of importance, this does not really matter
-- Column 4: water-oil capillary pressure (psi) 

0.12	0    		 	1	0
0.18	4.64876033057851E-008	1	0
0.24	0.000000186		0.997	0
0.3	4.18388429752066E-007	0.98	0
0.36	7.43801652892562E-007	0.7	0
0.42	1.16219008264463E-006	0.35	0
0.48	1.67355371900826E-006	0.2	0
0.54	2.27789256198347E-006	0.09	0
0.6	2.97520661157025E-006	0.021	0
0.66	3.7654958677686E-006	0.01	0
0.72	4.64876033057851E-006	0.001	0
0.78	0.000005625		0.0001	0
0.84	6.69421487603306E-006	0	0
0.91	8.05914256198347E-006	0	0
1	0.00001			0	0 /


SGOF
-- Column 1: gas saturation
-- Column 2: gas relative permeability
-- Column 3: oil relative permeability when oil, gas and connate water are present
-- Column 4: oil-gas capillary pressure (psi)
-- 	     - stated to be zero in Odeh's paper

-- Values in column 1-3 are taken from table 3 in Odeh's paper:
0	0	1	0
0.001	0	1	0
0.02	0	0.997	0
0.05	0.005	0.980	0
0.12	0.025	0.700	0
0.2	0.075	0.350	0
0.25	0.125	0.200	0
0.3	0.190	0.090	0
0.4	0.410	0.021	0
0.45	0.60	0.010	0
0.5	0.72	0.001	0
0.6	0.87	0.0001	0
0.7	0.94	0.000	0
0.85	0.98	0.000	0 
0.88	0.984	0.000	0 /
--1.00	1.0	0.000	0 /
-- Warning from Eclipse: first sat. value in SWOF + last sat. value in SGOF
-- 	   		 must not be greater than 1, but Eclipse still runs
-- Flow needs the sum to be excactly 1 so I added a row with gas sat. =  0.88
-- The corresponding krg value was estimated by assuming linear rel. between
-- gas sat. and krw. between gas sat. 0.85 and 1.00 (the last two values given)

DENSITY
-- Density (lb per ft³) at surface cond. of 
-- oil, water and gas, respectively (in that order)

-- Using values from Norne:
-- In METRIC units:
--      859.5 1033.0 0.854 /
-- In FIELD units:
      	53.66 64.49 0.0533 /

PVDG
-- Column 1: gas phase pressure (psia)
-- Column 2: gas formation volume factor (rb per Mscf)
-- 	     - in Odeh's paper the units are said to be given in rb per bbl, 
-- 	       but this is assumed to be a mistake: FVF-values in Odeh's paper 
--	       are given in rb per scf, not rb per bbl. This will be in 
--	       agreement with conventions
-- Column 3: gas viscosity (cP)

-- Using values from lower right table in Odeh's table 2:
14.700	166.666	0.008000
264.70	12.0930	0.009600
514.70	6.27400	0.011200
1014.7	3.19700	0.014000
2014.7	1.61400	0.018900
2514.7	1.29400	0.020800
3014.7	1.08000	0.022800
4014.7	0.81100	0.026800
5014.7	0.64900	0.030900
9014.7	0.38600	0.047000 /

PVTO
-- Column 1: dissolved gas-oil ratio (Mscf per stb)
-- Column 2: bubble point pressure (psia)
-- Column 3: oil FVF for saturated oil (rb per stb)
-- Column 4: oil viscosity for saturated oil (cP)

-- Use values from top left table in Odeh's table 2:
0.0010	14.7	1.0620	1.0400 /
0.0905	264.7	1.1500	0.9750 /
0.1800	514.7	1.2070	0.9100 /
0.3710	1014.7	1.2950	0.8300 /
0.6360	2014.7	1.4350	0.6950 /
0.7750	2514.7	1.5000	0.6410 /
0.9300	3014.7	1.5650	0.5940 /
1.2700	4014.7	1.6950	0.5100 
	9014.7	1.5790	0.7400 /
1.6180	5014.7	1.8270	0.4490 
	9014.7	1.7370	0.6310 /	
-- It is required to enter data for undersaturated oil for the highest GOR
-- (i.e. the last row) in the PVTO table.
-- In order to fulfill this requirement, values for oil FVF and viscosity
-- at 9014.7psia and GOR=1.618 for undersaturated oil have been approximated:
-- It has been assumed that there is a linear relation between the GOR
-- and the FVF when keeping the pressure constant at 9014.7psia.
-- From Odeh we know that (at 9014.7psia) the FVF is 2.357 at GOR=2.984
-- for saturated oil and that the FVF is 1.579 at GOR=1.27 for undersaturated oil,
-- so it is possible to use the assumption described above. 
-- An equivalent approximation for the viscosity has been used.
/
