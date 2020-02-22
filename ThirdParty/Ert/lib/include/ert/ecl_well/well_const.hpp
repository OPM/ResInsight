/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'well_const.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_WELL_CONST_H
#define ERT_WELL_CONST_H

#ifdef __cplusplus
extern "C" {
#endif

#define WELL_SEGMENT_OFFSET 1
#define WELL_BRANCH_OFFSET  1

#define ECLIPSE_WELL_SEGMENT_OFFSET                 1
#define ECLIPSE_WELL_BRANCH_OFFSET                  1

/* These values are taken from the ISEG description in table 6.1 in ECLIPSE file formats reference. */
#define ECLIPSE_WELL_SEGMENT_OUTLET_END_VALUE       0
#define ECLIPSE_WELL_SEGMENT_BRANCH_MAIN_STEM_VALUE 1
#define ECLIPSE_WELL_SEGMENT_INACTIVE_VALUE         0
#define ECLIPSE_CONN_NORMAL_WELL_SEGMENT_VALUE      0

#define WELL_SEGMENT_OUTLET_END_VALUE        (WELL_SEGMENT_OFFSET + ECLIPSE_WELL_SEGMENT_OUTLET_END_VALUE       - ECLIPSE_WELL_SEGMENT_OFFSET) // -1
#define WELL_SEGMENT_BRANCH_MAIN_STEM_VALUE  (WELL_BRANCH_OFFSET  + ECLIPSE_WELL_SEGMENT_BRANCH_MAIN_STEM_VALUE - ECLIPSE_WELL_BRANCH_OFFSET)  //  0
#define WELL_SEGMENT_BRANCH_INACTIVE_VALUE   (WELL_BRANCH_OFFSET  + ECLIPSE_WELL_SEGMENT_INACTIVE_VALUE         - ECLIPSE_WELL_BRANCH_OFFSET)  // -1
#define CONN_NORMAL_WELL_SEGMENT_VALUE       (WELL_SEGMENT_OFFSET + ECLIPSE_CONN_NORMAL_WELL_SEGMENT_VALUE      - ECLIPSE_WELL_SEGMENT_OFFSET)



/*
  Observe that the values given as _ITEM are not indices which can
  be directly used in the IWEL or ICON keywords; an offset must be
  added.
*/

#define IWEL_HEADI_INDEX               0
#define IWEL_HEADJ_INDEX               1
#define IWEL_HEADK_INDEX               2
#define IWEL_CONNECTIONS_INDEX         4
#define IWEL_GROUP_INDEX               5
#define IWEL_TYPE_INDEX                6
#define IWEL_STATUS_INDEX             10
#define IWEL_LGR_INDEX                42
#define IWEL_SEGMENTED_WELL_NR_INDEX  70

#define IWEL_HEADI_ITEM               0
#define IWEL_HEADJ_ITEM               1
#define IWEL_HEADK_ITEM               2
#define IWEL_CONNECTIONS_ITEM         4
#define IWEL_GROUP_ITEM               5
#define IWEL_TYPE_ITEM                6
#define IWEL_STATUS_ITEM             10
#define IWEL_LGR_ITEM                42
#define IWEL_SEGMENTED_WELL_NR_ITEM  70

#define IWEL_SEGMENTED_WELL_NR_NORMAL_VALUE -1
#define ISEG_OUTLET_INDEX        1
#define ISEG_BRANCH_INDEX        3

#define XWEL_RES_WRAT_ITEM        1
#define XWEL_RES_GRAT_ITEM        2
#define XWEL_RES_ORAT_ITEM        3
#define XWEL_RESV_ITEM        4


#define ICON_IC_INDEX         0
#define ICON_I_INDEX          1
#define ICON_J_INDEX          2
#define ICON_K_INDEX          3
#define ICON_STATUS_INDEX     5
#define ICON_DIRECTION_INDEX 13
#define ICON_SEGMENT_INDEX   14

#define ICON_IC_ITEM         0
#define ICON_I_ITEM          1
#define ICON_J_ITEM          2
#define ICON_K_ITEM          3
#define ICON_STATUS_ITEM     5
#define ICON_DIRECTION_ITEM 13
#define ICON_SEGMENT_ITEM   14

#define ICON_DIRX                 1
#define ICON_DIRY                 2
#define ICON_DIRZ                 3
#define ICON_FRACX                4
#define ICON_FRACY                5
#define ICON_DEFAULT_DIR_VALUE    0
#define ICON_DEFAULT_DIR_TARGET   ICON_DIRZ

#define SCON_CF_INDEX              0

#define XCON_ORAT_INDEX            0
#define XCON_WRAT_INDEX            1
#define XCON_GRAT_INDEX            2
#define XCON_QR_INDEX             49

#define RSEG_LENGTH_INDEX       0
#define RSEG_DIAMETER_INDEX     2
#define RSEG_TOTAL_LENGTH_INDEX 6
#define RSEG_DEPTH_INDEX        7

/*
  The ECLIPSE documentation says that a certain item in the IWEL array
  should indicate the type of the well, the available types are the
  ones given in the enum below. Unfortunately it turns out that when
  the well is closed the integer value in the IWEL array can be 0, if
  the well is indeed closed we accept this zero - otherwise we fail
  hard. Theese hoops are in the well_state_alloc() routine.
*/

#define IWEL_UNDOCUMENTED_ZERO 0
#define IWEL_PRODUCER          1
#define IWEL_OIL_INJECTOR      2
#define IWEL_WATER_INJECTOR    3
#define IWEL_GAS_INJECTOR      4

  typedef enum {
    ERT_UNDOCUMENTED_ZERO   = 0,   // Deprecated - retained for Resinsight compatibility
    ECL_WELL_ZERO           = 0,

    ERT_PRODUCER            = 1,   // Deprecated
    ECL_WELL_PRODUCER       = 1,

    ERT_OIL_INJECTOR        = 2,   // Deprecated
    ECL_WELL_OIL_INJECTOR   = 2,

    ERT_WATER_INJECTOR      = 3,   // Deprecated
    ECL_WELL_WATER_INJECTOR = 3,

    ERT_GAS_INJECTOR        = 4,   // Deprecated
    ECL_WELL_GAS_INJECTOR   = 4,
  } well_type_enum;

#ifdef __cplusplus
}
#endif

#endif
