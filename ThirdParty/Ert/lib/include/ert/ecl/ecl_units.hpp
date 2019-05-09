/*
   Copyright (C) 2017  Equinor ASA, Norway.

   The file 'ecl_units.h' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#ifndef ECL_UNITS_H
#define ECL_UNITS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ECL_UNITS_CUBIC(x) ((x)*(x)*(x))
#define ECL_UNITS_MILLI(x) ((x)*0.001)
#define ECL_UNITS_MEGA(x)  ((x)*1000000)

#define ECL_UNITS_LENGTH_INCH   0.0254
#define ECL_UNITS_LENGTH_FEET   12 * ECL_UNITS_LENGTH_INCH

#define ECL_UNITS_VOLUME_GALLON       231 * ECL_UNITS_CUBIC( ECL_UNITS_LENGTH_INCH )
#define ECL_UNITS_VOLUME_BARREL       ECL_UNITS_VOLUME_GALLON * 42
#define ECL_UNITS_VOLUME_LITER        0.001
#define ECL_UNITS_VOLUME_MILLI_LITER  ECL_UNITS_MILLI( ECL_UNITS_VOLUME_LITER )
#define ECL_UNITS_VOLUME_GAS_FIELD    ECL_UNITS_MEGA( ECL_UNITS_CUBIC( ECL_UNITS_LENGTH_FEET ) )

#define ECL_UNITS_TIME_HOUR 3600
#define ECL_UNITS_TIME_DAY  24 * ECL_UNITS_TIME_HOUR


#ifdef __cplusplus
}
#endif
#endif
