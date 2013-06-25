/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
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


#ifndef __WELL_CONST_H__
#define __WELL_CONST_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
  Observe that the values given as _ITEM are not indices which can
  be directly used in the IWEL or ICON keywords; an offset must be
  added. 
*/

#define IWEL_HEADI_ITEM               0
#define IWEL_HEADJ_ITEM               1
#define IWEL_HEADK_ITEM               2
#define IWEL_CONNECTIONS_ITEM         4
#define IWEL_TYPE_ITEM                6 
#define IWEL_STATUS_ITEM             10
#define IWEL_LGR_ITEM                42     
#define IWEL_SEGMENTED_WELL_NR_ITEM  70

#define ISEG_OUTLET_ITEM        1
#define ISEG_BRANCH_ITEM        3

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
    UNDOCUMENTED_ZERO = 0,
    PRODUCER          = 10,
    WATER_INJECTOR    = 22,
    GAS_INJECTOR      = 21,
    OIL_INJECTOR      = 78
  } well_type_enum;
  
  

#ifdef __cplusplus
}
#endif

#endif
