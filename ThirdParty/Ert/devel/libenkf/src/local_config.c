/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'local_config.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/int_vector.h>

#include <ert/geometry/geo_polygon.h>
#include <ert/geometry/geo_region.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_region.h>


#include <ert/enkf/local_ministep.h>
#include <ert/enkf/local_updatestep.h>
#include <ert/enkf/local_config.h>
#include <ert/enkf/local_dataset.h>
#include <ert/enkf/local_obsset.h>
#include <ert/enkf/local_context.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/enkf_defaults.h>
/******************************************************************/
/*

To create a configuration for localization you must "program" your own
configuration file, this file is then loaded from the ert/enkf proper
application. The 'commands' available in the local_config programming
language are listed below. [In the listing below the arguments to the
functions are shown in square braces, these braces should NOT be
included in the program file:


CREATE_UPDATESTEP [NAME_OF_UPDATESTEP]
---------------------------------------
This function will create a new updatestep with the name
'NAME_OF_UPDATESTEP'. Observe that you must add (at least) one
ministep to the updatestep, otherwise it will not be able to do
anything.


CREATE_MINISTEP [NAME_OF_MINISTEP  OBSSET_NAME]
-----------------------------------------------
This function will create a new ministep with the name
'NAME_OF_MINISTEP'. The ministep will be based on the observation
set given by OBSSET_NAME (which must be created first).The ministep 
is then ready for adding data. Before the ministep can be used you 
must attach it to an updatestep with the ATTACH_MINISTEP command

CREATE_DATASET [NAME_OF_DATASET]
--------------------------------
This function will create a new dataset, i.e. a collection of
enkf_nodes which should be updated together. Before you can actually
use a dataset you must attach it to a ministep with the ATTACH_DATASET
command.

COPY_DATASET [SRC_DATASET TARGET_DATASET]
-----------------------------------------
Will create a new local_dataset instance which is a copy of the
'SRC_DATASET'; this is a deep copy where also the lowest level
active_list instances are copied, and can then subsequently be updated
independently of eachother.


COPY_OBSSET [SRC_OBSSET TARGET_OBSSET]
-----------------------------------------
Will create a new local_obsset instance which is a copy of the
'SRC_OBSSET'; this is a deep copy where also the lowest level
active_list instances are copied, and can then subsequently be updated
independently of eachother.


CREATE_OBSSET [NAME_OF_OBSSET]
------------------------------
This function will create an observation set, i.e. a collection of
observation keys which will be used as the observations in one
ministep. Before the obsset can be used it must be attached to a
ministep with the ATTACH_OBSSET command.


ATTACH_MINISTEP [NAME_OF_UPDATESTEP  NAME_OF_MINISTEP]
------------------------------------------------------
This function will attach the ministep 'NAME_OF_MINISTEP' to the
updatestep 'NAME_OF_UPDATESTEP'; one ministep can be attached to many
updatesteps.


ATTACH_DATASET [NAME_OF_MINISTEP NAME_OF_DATASET]
-------------------------------------------------
Will attach the dataset 'NAME_OF_DATASET' to the ministep given by
'NAME_OF_MINISTEP'.


ATTACH_OBSSET [NAME_OF_MINISTEP NAME_OF_OBSSET]
-------------------------------------------------
Will attach the obsset 'NAME_OF_OBSSET' to the ministep given by
'NAME_OF_MINISTEP'.


ADD_DATA [NAME_OF_DATASET   KEY]
---------------------------------
This function will install 'KEY' as one enkf node which should be
updated in this dataset. If you do not manipulate the KEY further
with the ACTIVE_LIST_ADD_DATA_INDEX function the KEY will be added as
'ALL_ACTIVE', i.e. all elements will be updated.


ADD_OBS [NAME_OF_OBSSET  OBS_KEY]
-----------------------------------
This function will install the observation 'OBS_KEY' as an observation
for this obsset - similarly to the ADD_DATA function.


DEL_DATA [NAME_OF_DATASET  KEY]
--------------------------------
This function will delete the data 'KEY' from the dataset
'NAME_OF_DATASET'.


DEL_OBS [NAME_OF_OBSSET  OBS_KEY]
-----------------------------------
This function will delete the obs 'OBS_KEY' from the obsset
'NAME_OF_OBSSET'.


DATASET_DEL_ALL_DATA [NAME_OF_DATASET]
--------------------------------------
This function will delete all the data keys from the dataset
'NAME_OF_MINISTEP'.


OBSSET_DEL_ALL_OBS [NAME_OF_OBSSET]
-----------------------------------
This function will delete all the obs keys from the obsset
'NAME_OF_OBSSET'.


ACTIVE_LIST_ADD_OBS_INDEX[OBSSET_NAME  OBS_KEY  INDEX]
--------------------------------------------------------
This function will say that the observation with name 'OBS_KEY' in
obsset with name 'OBSSET_NAME' should have the index 'INDEX'
active.


ACTIVE_LIST_ADD_DATA_INDEX[DATASET_NAME  DATA_KEY  INDEX]
--------------------------------------------------------
This function will say that the data with name 'DATA_KEY' in dataset
with name 'DATASTEP_NAME' should have the index 'INDEX' active.


ACTIVE_LIST_ADD_MANY_OBS_INDEX[OBSSET_NAME  OBS_KEY  N INDEX1 INDEX2 INDEX3 .. INDEXN]
----------------------------------------------------------------------------------------
This function is simular to ACTIVE_LIST_ADD_OBS_INDEX, but it will add many indices.



ACTIVE_LIST_ADD_MANY_DATA_INDEX[DATA_NAME  DATA_KEY  N INDEX1 INDEX2 INDEX3 .. INDEXN]
------------------------------------------------------------------------------------------
This function is simular to ACTIVE_LIST_ADD_DATA_INDEX, but it will add many indices.


INSTALL_UPDATESTEP [NAME_OF_UPDATESTEP  STEP1   STEP2]
----------------------------------------------------
This function will install the updatestep 'NAME_OF_UPDATESTEP' for the
report steps [STEP1,..,STEP2].


INSTALL_DEFAULT_UPDATESTEP [NAME_OF_UPDATESTEP]
-----------------------------------------------
This function will install 'NAME_OF_UPDATESTEP' as the default
updatestep which applies to all report streps where you have not
explicitly set another updatestep with the INSTALL_UPDATESTEP function.



ADD_FIELD   [DATASET_NAME    FIELD_NAME    ECLREGION_NAME]
--------------------------------------------------------

This function will install the node with name 'FIELD_NAME' in the
dataset 'DATASET_NAME'. It will in addition select all the
(currently) active cells in the region 'ECLREGION_NAME' as active for
this field/ministep combination. The ADD_FIELD command is actually a
shortcut for the following:

   ADD_DATA   DATASET  FIELD_NAME
   ACTIVE_LIST_ADD_MANY_DATA_INDEX  <All the indices from the region>



LOAD_FILE       [KEY    FILENAME]
---------------------------------
This function will load an ECLIPSE file in restart format
(i.e. restart file or INIT file), the keywords in this file can then
subsequently be used in ECLREGION_SELECT_VALUE_XXX commands below. The
'KEY' argument is a string which will be used later when we refer to
the content of this file


CREATE_ECLREGION   [ECLREGION_NAME    SELECT_ALL]
-------------------------------------------
This function will create a new region 'ECLREGION_NAME', which can
subsequently be used when defining active regions for fields. The
second argument, SELECT_ALL, is a boolean value. If this value is set
to true the region will start with all cells selected, if set to false
the region will start with no cells selected.


ECLREGION_SELECT_ALL     [ECLREGION_NAME   SELECT]
--------------------------------------------
Will select all the cells in the region (or deselect if SELECT == FALSE).


ECLREGION_SELECT_VALUE_EQUAL   [ECLREGION_NAME   FILE_KEY:KEYWORD<:NR>    VALUE   SELECT]
-----------------------------------------------------------------------------------
This function will compare an ecl_kw instance loaded from file with a
user supplied value, and select (or deselect) all cells which match
this value. It is assumed that the ECLIPSE keyword is an INTEGER
keyword, for float comparisons use the ECLREGION_SELECT_VALUE_LESS and
ECLREGION_SELECT_VALUE_MORE functions.


ECLREGION_SELECT_VALUE_LESS
ECLREGION_SELECT_VALUE_MORE    [ECLREGION_NAME   FILE_KEY:KEYWORD<:NR>  VALUE   SELECT]
---------------------------------------------------------------------------------
This function will compare an ecl_kw instance loaded from disc with a
numerical value, and select all cells which have numerical below or
above the limiting value. The ecl_kw value should be a floating point
value like e.g. PRESSURE or PORO. The arguments are just as for ECLREGION_SELECT_VALUE_EQUAL.


ECLREGION_SELECT_BOX            [ ECLREGION_NAME i1 i2 j1 j2 k1 k2 SELECT]
--------------------------------------------------------------------
This function will select (or deselect) all the cells in the box
defined by the six coordinates i1 i2 j1 j2 k1 k2. The coordinates are
inclusive, and the counting starts at 1.


ECLREGION_SELECT_SLICE         [ ECLREGION_NAME dir n1 n2 SELECT]
-----------------------------------------------------------
This function will select a slice in the direction given by 'dir',
which can 'x', 'y' or 'z'. Depending on the value of 'dir' the numbers
n1 and n2 are interpreted as (i1 i2), (j1 j2) or (k1 k2)
respectively. The numbers n1 and n2 are inclusice and the counting
starts at 1. It is OK to use very high/low values to imply "the rest
of the cells" in one direction.


ECLREGION_SELECT_PLANE  [ECLREGION_NAME nx ny nz px py pz sign SELECT]
---------------------------------------------------------
Will select all points which have positive (sign > 0) distance to 
the plane defined by normal vector n = (nx,ny,nz) and point 
p = (px,py,pz). If sign < 0 all cells with negative distance to 
plane will be selected.


ECLREGION_SELECT_IN_POLYGON [ECLREGION_NAME POLYGON_NAME SELECT]
---------------------------------------------------
Well select all the points which are inside the polygon with name
'POLYGON_NAME'. The polygon should have been created with command 
CREATE_POLYGON or loaded with command 'LOAD_POLYGON' first.


CREATE_POLYGON  [POLYGON_NAME  num_points x1 y1 x2 y2 x3 y3 ....]
---------------------------------------------------------------
Will create a geo_polygon instance based on the coordinate list: 
(x1,y1), (x2,y2), (x3,y3), ... The polygon should not be explicitly 
closed - i.e. you should in general have (x1,y1) != (xn,yn). The 
polygon will be stored under the name 'POLYGON_NAME' - which should
later be used when referring to the polygon in region select operations.


LOAD_POLYGON  [POLYGON_NAME  FILENAME]
--------------------------------------------------------------
Will load a polygon instance from the file 'FILENAME' - the file should
be in irap RMS format. The polygon will be stored under the name 'POLYGON_NAME'
which can then later be used to refer to the polygon for e.g. select operations.


LOAD_SURFACE  [SURFACE_NAME  SURFACE_FILE]
---------------------------------------------------------------
Will load an irap surface from file 'SURFACE_FILE'. The surface will be
stored internally as 'SURFACE_NAME' - this function is mainly needed to
have a base surface available for the CREATE_SURFACE_REGION command.


CREATE_SURFACE_REGION  [REGION_NAME BASE_SURFACE  PRESELECT]
----------------------------------------------------------------
Will create a new surface region object which can be used to select
and deselect parts of a surface. The region will be called 'REGION_NAME' 
and it will be based on the surface given by 'BASE_SURFACE'. 'PRESELECT' 
is a boolean 'TRUE' or 'FALSE' which determines whether the region is
created with all points selected, or no points selected.


SURFACE_REGION_SELECT_IN_POLYGON  [REGION_NAME  POLYGON_NAME SELECT]
--------------------------------------------------------------------
Well select|deselect all the points in the surface which are inside the 
polygon.


SURFACE_REGION_SELECT_LINE [ REGION_NAME  X1 Y1 X2 Y2 SIGN SELECT]
------------------------------------------------------------------
Well select|deselect all the points which are above|below the line: (x1,y1) -> (x2,y2)

If SIGN is positive the select will apply to all points with a 
positive (right hand system) distance to the line; if SIGN is negative 
the selector will apply to all points with a negative distance to the line.


ADD_DATA_SURFACE  [ DATASET_NAME  SURFACE_NAME   REGION NAME]
-------------------------------------------------------------------
Will add the node 'SURFACE_NAME' (not one of the loaded surfaces, but
an enkf_node object) to the dataset 'DATASET_NAME'. Only the elements 
in the region 'REGION_NAME' will be added. Typically SURFACE_REGION_SELECT_xxxx
has been used first to build a suitable region selection.


I have added comments in the example - that is not actually supported (yet at least)

-------------------------------------------------------------------------------------
CREATE_DATASET     MSTEP
CREATE_ECLREGION   FIPNUM3       FALSE              --- We create a region called FIPNUM3 with no elements
                                                    --- selected from the start.
CREATE_ECLREGION   WATER_FLOODED TRUE               --- We create a region called WATER_FLOEDED,
                                                    --- which starts with all elements selected.
CREATE_ECLREGION   MIDLLE        FALSE              --- Create a region called MIDDLE with
                                                 --- no elements initially.
LOAD_FILE       INIT          /path/to/ECL.INIT  --- We load the INIT file and label
                                                 --- it as INIT for further use.
LOAD_FILE       RESTART       /path/to/ECL.UNRST --- We load a unified restart fila
                                                 --- and label it RESTART

-- We select all the cells corresponding to a FIPNUM value of 3. Since there is
-- only one FIPNUM keyword in the INIT file we do not need the :NR suffix on the key.
ECLREGION_SELECT_VALUE_EQUAL     FIPNUM3     INIT:FIPNUM    3    TRUE


-- In the region WATER_FLOODED all cells are selected from the start, now
-- we deselect all the cells which have SWAT value below 0.90, at report step 100:
ECLREGION_SELECT_VALUE_LESS    WATER_FLOODED RESTART:SWAT:100   0.90    FALSE

-- We select the the layers k=4,5,6 in the region MIDDLE. The indices 4,5
-- and 6 are "normal" k values, where the counting starts at 1.
ECLREGION_SELECT_SLICE  MIDDLE   Z   4  6   TRUE


-- We add field data in the current dataset, corresponding to the two
-- selection regions (poro is only updated in FIPNUM3, PERMX is only updated in
-- the water flooded region and NTG is only updates in the MIDDLE region).
ADD_FIELD    MSTEP    PORO    FIPNUM3
ADD_FIELD    MSTEP    PERMX   WATER_FLOODED
ADD_FIELD    MSTEP    NTG     MIDDLE
-------------------------------------------------------------------------------------
Second example:
CREATE_DATASET SURFACE_DATA

-- We load a surface from file which will be used as a base-surface when
-- selecting active elements in surfaces. We give the surface the name 
-- 'BASE_SURFACE' - the surface should be in irap format.
LOAD_SURFACE                         BASE_SURFACE Surface/base.irap


-- We load two polygons in irap format; the polygons ire called 'North'
-- and 'South'. Alternatively we can create a polygon with CREATE_POLYGON 
-- command:
LOAD_POLYGON                         North       Polygon/north.irap
LOAD_POLYGON                         South       Polygon/south.irap


-- We create a new surface region - a surface region is a set of 
-- points in a surface; the region need not be mathematically connected.
-- The surface region is called myRegion - it is based on 'BASE_SURFACE'
-- surface, and we start out with no elements selected.
CREATE_SURFACE_REGION                myRegion  BASE_SURFACE  False


-- We update the region selection in 'myRegion' be selecting all the 
-- points which are inside the two polygons 'North' and 'South':
SURFACE_REGION_SELECT_IN_POLYGON     myRegion   North   True
SURFACE_REGION_SELECT_IN_POLYGON     myRegion   South   True


-- We add two enkf surface objects, called TOP and BOTTOM ('TOP' and
-- 'BOTTOM' should be added in the enkf config file). For each of these
-- surfaces we say that only the points within the region 'myRegion'
-- should be updated:
ADD_DATA_SURFACE               ALL_DATA    TOP         myRegion
ADD_DATA_SURFACE               ALL_DATA    BOTTOM      myRegion

    _________________________________________________________________________
   /                                                                         \
   | Observe that prior to loading your hand-crafted configuration file      |
   | the program will load an ALL_ACTIVE configuration which will be         |
   | installed as default. If you want you can start with this               |
   | configuration:                                                          |
   |                                                                         |
   | DEL_OBS   ALL_ACTIVE   <Some observation key you do not want to use..>  |
   |                                                                         |
   \_________________________________________________________________________/

*/
/******************************************************************/

/*

  +-------------------------- local_updatestep_type ---------------------------------------+
  |                                                                                        |
  |                                                                                        |
  |    +----------------- local_ministep_type --------------------------------------+      |
  |    |                                                                            |      |
  |    |                                       /    +--- local_dataset_type ---+    |      |
  |    |                                       |    | PRESSURE                 |    |      |
  |    |                                       |    | SWAT                     |    |      |
  |    |                                       |    | SGAS                     |    |      |
  |    |                                       |    +--------------------------+    |      |
  |    |    +-- local_obsset_type ---+         |                                    |      |
  |    |    | WWCT:OP_2              |         |    +--- local_dataset_type ---+    |      |
  |    |    | WGOR:OP_1              |         |    | MULTFLT1                 |    |      |
  |    |    | RFT:WELL1              |  <------|    | MULTFLT2                 |    |      |
  |    |    | RFT:WELL3              |         |    | MULTFLT3                 |    |      |
  |    |    | WWCT:WELLX             |         |    +--------------------------+    |      |
  |    |    +------------------------+         |                                    |      |
  |    |                                       |    +--- local_dataset_type ---+    |      |
  |    |                                       |    | RELPERM1                 |    |      |
  |    |                                       |    | RELPERM2                 |    |      |
  |    |                                       |    | RELPERM3                 |    |      |
  |    |                                       \    +--------------------------+    |      |
  |    |                                                                            |      |
  |    +----------------------------------------------------------------------------+      |
  |                                                                                        |
  |                                                                                        |  
  |    +----------------- local_ministep_type --------------------------------------+      |
  |    |                                                                            |      |
  |    |                                       /    +--- local_dataset_type ---+    |      |
  |    |    +-- local_obsset_type ---+         |    | PERMX PORO               |    |      |
  |    |    | 4D Seismic             |         |    | PRESSURE SWAT            |    |      |
  |    |    | Gravimetri             |         |    | SGAS                     |    |      |
  |    |    |                        |  <------|    +--------------------------+    |      |
  |    |    |                        |         |                                    |      |
  |    |    |                        |         |    +--- local_dataset_type ---+    |      |
  |    |    +------------------------+         |    | MULTFLT1                 |    |      |
  |    |                                       |    | MULTFLT2                 |    |      |
  |    |                                       |    | MULTFLT3                 |    |      |
  |    |                                       \    +--------------------------+    |      |
  |    |                                                                            |      |
  |    +----------------------------------------------------------------------------+      |
  |                                                                                        |
  +----------------------------------------------------------------------------------------+

This figure illustrates the different objects when configuring local
analysis:

local_updatestep_type: This is is the top level configuration of the
   updating at one timestep. In principle you can have different
   updatestep configurations at the different timesteps, but it will
   typically be identical for all the time steps. Observe that the
   update at one time step can typically conist of several enkf
   updates, this is handled by using several local_ministep.

local_ministep_type: The ministep defines a collection of observations
   and state/parameter variables which are mutually dependant on
   eachother and should be updated together. The local_ministep will
   consist of *ONE* local_obsset of observations, and one or more
   local_dataset of data which should be updated.

local_obsset_type: This is a collection of observation data; there is
   exactly one local_obsset for each local_ministep.

local_dataset_type: This is a collection of data/parameters which
   should be updated together in the EnKF updating. 


How the local_dataset_type is configured is quite important for the
core EnKF updating:

 1. All the members in one local_dataset instance are serialized and
    packed in the A-matrix together; i.e. in the example above the
    parameters RELPERM1,RELPERM2 and RELPERM3 are updated in one go.

 2. When using the standard EnKF the X matrix is calculated with using
    the actual data vectors, and the results will be identical if we
    use one large local_dataset instance or several small. However
    when using more advanced techniques where the A matrix is used
    explicitly when calculating the update this will matter.

 3. If you have not entered a local configuration explicitly the
    default ALL_ACTIVE local configuration will be used. The
    ALL_ACTIVE configuration can be modified slightly with the
    UPDATE_RESULTS and SINGLE_NODE_UPDATE settings in the config file:

      UPDATE_RESULTS: Determines whether variables with enkf_type ==
         DYNAMIC_RESULT should be updated. 

      SINGLE_NODE_UPDATE: If SINGLE_NODE_UPDATE is set to true the
         ALL_ACTIVE configuration will consist of maaany
         local_dataset_type instances with one enkf_node in each. In
         the opposite case only one local_dataset_type instance will
         be created with all the enkf_node instances assembled in one
         big package. A middle-ground might be good?
*/


struct local_config_struct {
  vector_type           * updatestep;            /* This is an indexed vector with (pointers to) local_reportsstep instances. */
  local_updatestep_type * default_updatestep;    /* A default report step returned if no particular report step has been installed for this time index. */
  hash_type             * updatestep_storage;    /* These three hash tables are the 'holding area' for the local_updatestep, */
  hash_type             * ministep_storage;      /* local_ministep instances. */
  hash_type             * dataset_storage;
  hash_type             * obsset_storage;
  stringlist_type       * config_files;
};


static void local_config_clear( local_config_type * local_config ) {
  local_config->default_updatestep  = NULL;
  hash_clear( local_config->updatestep_storage );
  hash_clear( local_config->ministep_storage );
  hash_clear( local_config->dataset_storage );
  hash_clear( local_config->obsset_storage );
  vector_clear( local_config->updatestep );
}




local_config_type * local_config_alloc( ) {
  local_config_type * local_config = util_malloc( sizeof * local_config );

  local_config->default_updatestep  = NULL;
  local_config->updatestep_storage  = hash_alloc();
  local_config->ministep_storage    = hash_alloc();
  local_config->dataset_storage     = hash_alloc();
  local_config->obsset_storage      = hash_alloc();
  local_config->updatestep          = vector_alloc_new();
  local_config->config_files = stringlist_alloc_new();

  local_config_clear( local_config );
  return local_config;
}


void local_config_free(local_config_type * local_config) {
  vector_free( local_config->updatestep );
  hash_free( local_config->updatestep_storage );
  hash_free( local_config->ministep_storage);
  hash_free( local_config->dataset_storage);
  stringlist_free( local_config->config_files );
  free( local_config );
}



/**
   Actual report step must have been installed in the
   updatestep_storage with local_config_alloc_updatestep() first.
*/

void local_config_set_default_updatestep( local_config_type * local_config , const char * default_key) {
  local_updatestep_type * default_updatestep = local_config_get_updatestep( local_config , default_key );
  local_config->default_updatestep = default_updatestep;
}


/**
   Instances of local_updatestep and local_ministep are allocated from
   the local_config object, and then subsequently manipulated from the calling scope.
*/

local_updatestep_type * local_config_alloc_updatestep( local_config_type * local_config , const char * key ) {
  local_updatestep_type * updatestep = local_updatestep_alloc( key );
  hash_insert_hash_owned_ref( local_config->updatestep_storage , key , updatestep , local_updatestep_free__);
  return updatestep;
}


local_ministep_type * local_config_alloc_ministep( local_config_type * local_config , const char * key , const char * obsset_name) {
  local_obsset_type * obsset = hash_get( local_config->obsset_storage , obsset_name );
  local_ministep_type * ministep = local_ministep_alloc( key , obsset);
  hash_insert_hash_owned_ref( local_config->ministep_storage , key , ministep , local_ministep_free__);
  return ministep;
}

local_obsset_type * local_config_alloc_obsset( local_config_type * local_config , const char * obsset_name ) {
  local_obsset_type * obsset = local_obsset_alloc( obsset_name );
  hash_insert_hash_owned_ref( local_config->obsset_storage , obsset_name , obsset , local_obsset_free__);
  return obsset;
}



local_dataset_type * local_config_alloc_dataset( local_config_type * local_config , const char * key ) {
  local_dataset_type * dataset = local_dataset_alloc( key );
  hash_insert_hash_owned_ref( local_config->dataset_storage , key , dataset , local_dataset_free__);
  return dataset;
}


local_dataset_type * local_config_alloc_dataset_copy( local_config_type * local_config , const char * src_key , const char * target_key) {
  local_dataset_type * src_dataset = hash_get( local_config->dataset_storage , src_key );
  local_dataset_type * copy_dataset = local_dataset_alloc_copy( src_dataset , target_key );
  
  hash_insert_hash_owned_ref( local_config->dataset_storage , target_key , copy_dataset , local_dataset_free__);
  return copy_dataset;
}


local_obsset_type * local_config_alloc_obsset_copy( local_config_type * local_config , const char * src_key , const char * target_key) {
  local_obsset_type * src_obsset = hash_get( local_config->obsset_storage , src_key );
  local_obsset_type * copy_obsset = local_obsset_alloc_copy( src_obsset , target_key );
  
  hash_insert_hash_owned_ref( local_config->obsset_storage , target_key , copy_obsset , local_obsset_free__);
  return copy_obsset;
}


local_ministep_type * local_config_get_ministep( const local_config_type * local_config , const char * key) {
  local_ministep_type * ministep = hash_get( local_config->ministep_storage , key );
  return ministep;
}


local_obsset_type * local_config_get_obsset( const local_config_type * local_config , const char * key) {
  local_obsset_type * obsset = hash_get( local_config->obsset_storage , key );
  return obsset;
}


local_dataset_type * local_config_get_dataset( const local_config_type * local_config , const char * key) {
  local_dataset_type * dataset = hash_get( local_config->dataset_storage , key );
  return dataset;
}



local_ministep_type * local_config_alloc_ministep_copy( local_config_type * local_config , const char * src_key , const char * new_key) {
  local_ministep_type * src_step = hash_get( local_config->ministep_storage , src_key );
  local_ministep_type * new_step = local_ministep_alloc_copy( src_step , new_key );
  hash_insert_hash_owned_ref( local_config->ministep_storage , new_key , new_step , local_ministep_free__);
  return new_step;
}



const local_updatestep_type * local_config_iget_updatestep( const local_config_type * local_config , int index) {
  const local_updatestep_type * updatestep = vector_safe_iget_const( local_config->updatestep , index );
  if (updatestep == NULL)
    /*
      No particular report step has been installed for this
      time-index, revert to the default.
    */
    updatestep = local_config->default_updatestep;
  
  if (updatestep == NULL)
    util_exit("%s: fatal error. No report step information for step:%d - and no default \n",__func__ , index);

  return updatestep;
}


local_updatestep_type * local_config_get_updatestep( const local_config_type * local_config , const char * key) {
  return hash_get( local_config->updatestep_storage , key );
}


/**
   This will 'install' the updatestep instance identified with 'key'
   for report steps [step1,step2]. Observe that the report step must
   have been allocated with 'local_config_alloc_updatestep()' first.
*/


void local_config_set_updatestep(local_config_type * local_config, int step1 , int step2 , const char * key) {
  local_updatestep_type * updatestep = hash_get( local_config->updatestep_storage , key );
  int step;
  
  for ( step = step1; step < step2 + 1; step++)
    vector_safe_iset_ref(local_config->updatestep , step , updatestep );

}


/*******************************************************************/
/* Functions related to loading a local config instance from disk. */


static local_config_instruction_type local_config_cmd_from_string( hash_type * cmd_table , char * cmd_string ) {

  util_strupr( cmd_string );
  if (hash_has_key( cmd_table, cmd_string))
    return hash_get_int( cmd_table , cmd_string);
  else {
    util_abort("%s: command:%s not recognized \n",__func__ , cmd_string);
    return -1;
  }
}



const char * local_config_get_cmd_string( local_config_instruction_type cmd ) {
  switch (cmd) {
  case(CREATE_UPDATESTEP):
    return CREATE_UPDATESTEP_STRING;
    break;
  case(CREATE_MINISTEP):
    return CREATE_MINISTEP_STRING;
    break;
  case(ATTACH_MINISTEP):
    return ATTACH_MINISTEP_STRING;
    break;
  case(CREATE_DATASET):
    return CREATE_DATASET_STRING;
    break;
  case(ATTACH_DATASET):
    return ATTACH_DATASET_STRING;
    break;
  case(CREATE_OBSSET):
    return CREATE_OBSSET_STRING;
    break;
  case(ADD_DATA):
    return ADD_DATA_STRING;
    break;
  case(ADD_OBS):
    return ADD_OBS_STRING;
    break;
  case(ACTIVE_LIST_ADD_OBS_INDEX):
    return ACTIVE_LIST_ADD_OBS_INDEX_STRING;
    break;
  case(ACTIVE_LIST_ADD_DATA_INDEX):
    return ACTIVE_LIST_ADD_DATA_INDEX_STRING;
    break;
  case(ACTIVE_LIST_ADD_MANY_OBS_INDEX):
    return ACTIVE_LIST_ADD_MANY_OBS_INDEX_STRING;
    break;
  case(ACTIVE_LIST_ADD_MANY_DATA_INDEX):
    return ACTIVE_LIST_ADD_MANY_DATA_INDEX_STRING;
    break;
  case(INSTALL_UPDATESTEP):
    return INSTALL_UPDATESTEP_STRING;
    break;
  case(INSTALL_DEFAULT_UPDATESTEP):
    return INSTALL_DEFAULT_UPDATESTEP_STRING;
    break;
  case(DEL_DATA):
    return DEL_DATA_STRING;
    break;
  case(DEL_OBS):
    return DEL_OBS_STRING;
    break;
  case(DATASET_DEL_ALL_DATA):
    return DATASET_DEL_ALL_DATA_STRING;
    break;
  case(OBSSET_DEL_ALL_OBS):
    return OBSSET_DEL_ALL_OBS_STRING;
    break;
  case(ADD_FIELD):
    return ADD_FIELD_STRING;
    break;
  case(CREATE_ECLREGION):
    return CREATE_ECLREGION_STRING;
    break;
  case(LOAD_FILE):
    return LOAD_FILE_STRING;
    break;
  case(ECLREGION_SELECT_ALL):
    return ECLREGION_SELECT_ALL_STRING;
    break;
  case(ECLREGION_SELECT_VALUE_EQUAL):
    return ECLREGION_SELECT_VALUE_EQUAL_STRING;
    break;
  case(ECLREGION_SELECT_VALUE_LESS):
    return ECLREGION_SELECT_VALUE_LESS_STRING;
    break;
  case(ECLREGION_SELECT_VALUE_MORE):
    return ECLREGION_SELECT_VALUE_MORE_STRING;
    break;
  case(ECLREGION_SELECT_BOX):
    return ECLREGION_SELECT_BOX_STRING;
    break;
  case(ECLREGION_SELECT_SLICE):
    return ECLREGION_SELECT_SLICE_STRING;
    break;
  case(ECLREGION_SELECT_PLANE):
    return ECLREGION_SELECT_PLANE_STRING;
    break;
  case(ECLREGION_SELECT_IN_POLYGON):
    return ECLREGION_SELECT_IN_POLYGON_STRING;
    break;
  case(CREATE_POLYGON):
    return CREATE_POLYGON_STRING;
    break;
  case(LOAD_POLYGON):
    return LOAD_POLYGON_STRING;
    break;
  case(LOAD_SURFACE):
    return LOAD_SURFACE_STRING;
    break;
  case(CREATE_SURFACE_REGION):
    return CREATE_SURFACE_REGION_STRING;
    break;
  case(SURFACE_REGION_SELECT_IN_POLYGON):
    return SURFACE_REGION_SELECT_IN_POLYGON_STRING;
    break;
  case(SURFACE_REGION_SELECT_LINE):
    return SURFACE_REGION_SELECT_LINE_STRING;
    break;
  case(ADD_DATA_SURFACE):
    return ADD_DATA_SURFACE_STRING;
    break;
  default:
    util_abort("%s: command:%d not recognized \n",__func__ , cmd);
    return NULL;
  }
}


static int read_int(FILE * stream , bool binary ) {
  if (binary)
    return util_fread_int( stream );
  else {
    int value;
    fscanf(stream , "%d" , &value);
    return value;
  }
}


static double read_double(FILE * stream , bool binary) {
  if (binary)
    return util_fread_double( stream );
  else {
    double value;
    fscanf(stream , "%lg" , &value);
    return value;
  }
}



static void read_int_vector(FILE * stream , bool binary , int_vector_type * vector) {
  if (binary) {
    int size = util_fread_int( stream );
    int_vector_fread_data( vector , size , stream );
  } else {
    int size,value,i;
    int_vector_reset( vector );
    fscanf(stream , "%d" , &size);
    for (i=0; i < size; i++) {
      if (fscanf(stream , "%d", &value) == 1)
        int_vector_append(vector , value);
      else
        util_abort("%s: premature end of indices when reading local configuraton - malformed file.\n",__func__);
    }
  }
}


static char * read_alloc_string(FILE * stream , bool binary) {
  if (binary)
    return util_fread_alloc_string( stream );
  else {
    char * string = util_calloc(256 , sizeof * string ); /* 256 - outht to be enough for everyone ... */
    fscanf(stream , "%s" , string);
    return string;
  }
}


static bool read_bool(FILE * stream , bool binary) {
  if (binary)
    return util_fread_bool( stream );
  else {
    bool value;
    char * s = read_alloc_string( stream , binary );
    if (!util_sscanf_bool( s , &value))
      util_abort("%s: failed to interpret:\'%s\' as boolean true / false\n",__func__ , s );
    free( s );
    return value;
  }
}





static bool read_cmd( hash_type * cmd_table , FILE * stream , bool binary , local_config_instruction_type * cmd) {
  if (binary) {
    if (fread( cmd , sizeof cmd , 1 , stream) == 1)
      return true;
    else
      return false;
  } else {
    char cmd_string[64];
    if (fscanf(stream , "%s" , cmd_string) == 1) {
      *cmd = local_config_cmd_from_string( cmd_table , cmd_string );
      return true;
    } else
      return false;
  }
}


stringlist_type * local_config_get_config_files( const local_config_type * local_config ) {
  return local_config->config_files;
}


void local_config_clear_config_files( local_config_type * local_config ) {
  stringlist_clear( local_config->config_files );
}


void local_config_add_config_file( local_config_type * local_config , const char * config_file ) {
  stringlist_append_copy( local_config->config_files , config_file );
}



static void local_config_init_cmd_table( hash_type * cmd_table ) {
  hash_insert_int(cmd_table , CREATE_UPDATESTEP_STRING               , CREATE_UPDATESTEP);
  hash_insert_int(cmd_table , CREATE_MINISTEP_STRING                 , CREATE_MINISTEP);
  hash_insert_int(cmd_table , ATTACH_MINISTEP_STRING                 , ATTACH_MINISTEP);
  hash_insert_int(cmd_table , CREATE_DATASET_STRING                  , CREATE_DATASET);
  hash_insert_int(cmd_table , ATTACH_DATASET_STRING                  , ATTACH_DATASET);
  hash_insert_int(cmd_table , CREATE_OBSSET_STRING                   , CREATE_OBSSET);
  hash_insert_int(cmd_table , ADD_DATA_STRING                        , ADD_DATA);
  hash_insert_int(cmd_table , ADD_OBS_STRING                         , ADD_OBS );
  hash_insert_int(cmd_table , ACTIVE_LIST_ADD_OBS_INDEX_STRING       , ACTIVE_LIST_ADD_OBS_INDEX);
  hash_insert_int(cmd_table , ACTIVE_LIST_ADD_DATA_INDEX_STRING      , ACTIVE_LIST_ADD_DATA_INDEX);
  hash_insert_int(cmd_table , ACTIVE_LIST_ADD_MANY_OBS_INDEX_STRING  , ACTIVE_LIST_ADD_MANY_OBS_INDEX);
  hash_insert_int(cmd_table , ACTIVE_LIST_ADD_MANY_DATA_INDEX_STRING , ACTIVE_LIST_ADD_MANY_DATA_INDEX);
  hash_insert_int(cmd_table , INSTALL_UPDATESTEP_STRING              , INSTALL_UPDATESTEP);
  hash_insert_int(cmd_table , INSTALL_DEFAULT_UPDATESTEP_STRING      , INSTALL_DEFAULT_UPDATESTEP);
  hash_insert_int(cmd_table , DEL_DATA_STRING                        , DEL_DATA);
  hash_insert_int(cmd_table , DEL_OBS_STRING                         , DEL_OBS);
  hash_insert_int(cmd_table , COPY_DATASET_STRING                    , COPY_DATASET);
  hash_insert_int(cmd_table , COPY_OBSSET_STRING                     , COPY_OBSSET);
  hash_insert_int(cmd_table , DATASET_DEL_ALL_DATA_STRING            , DATASET_DEL_ALL_DATA);
  hash_insert_int(cmd_table , OBSSET_DEL_ALL_OBS_STRING              , OBSSET_DEL_ALL_OBS);
  hash_insert_int(cmd_table , ADD_FIELD_STRING                       , ADD_FIELD);
  hash_insert_int(cmd_table , CREATE_ECLREGION_STRING                , CREATE_ECLREGION);
  hash_insert_int(cmd_table , LOAD_FILE_STRING                       , LOAD_FILE);
  hash_insert_int(cmd_table , ECLREGION_SELECT_ALL_STRING            , ECLREGION_SELECT_ALL);
  hash_insert_int(cmd_table , ECLREGION_SELECT_VALUE_EQUAL_STRING    , ECLREGION_SELECT_VALUE_EQUAL);
  hash_insert_int(cmd_table , ECLREGION_SELECT_VALUE_LESS_STRING     , ECLREGION_SELECT_VALUE_LESS);
  hash_insert_int(cmd_table , ECLREGION_SELECT_VALUE_MORE_STRING     , ECLREGION_SELECT_VALUE_MORE);
  hash_insert_int(cmd_table , ECLREGION_SELECT_BOX_STRING            , ECLREGION_SELECT_BOX);
  hash_insert_int(cmd_table , ECLREGION_SELECT_SLICE_STRING          , ECLREGION_SELECT_SLICE);
  hash_insert_int(cmd_table , ECLREGION_SELECT_PLANE_STRING          , ECLREGION_SELECT_PLANE);
  hash_insert_int(cmd_table , ECLREGION_SELECT_IN_POLYGON_STRING     , ECLREGION_SELECT_IN_POLYGON);
  hash_insert_int(cmd_table , CREATE_POLYGON_STRING                  , CREATE_POLYGON );
  hash_insert_int(cmd_table , LOAD_POLYGON_STRING                    , LOAD_POLYGON );
  hash_insert_int(cmd_table , LOAD_SURFACE_STRING                    , LOAD_SURFACE );   
  hash_insert_int(cmd_table , CREATE_SURFACE_REGION_STRING           , CREATE_SURFACE_REGION );   
  hash_insert_int(cmd_table , SURFACE_REGION_SELECT_IN_POLYGON_STRING, SURFACE_REGION_SELECT_IN_POLYGON);
  hash_insert_int(cmd_table , SURFACE_REGION_SELECT_LINE_STRING     , SURFACE_REGION_SELECT_LINE);
  hash_insert_int(cmd_table , ADD_DATA_SURFACE_STRING                , ADD_DATA_SURFACE);
}


static void local_config_CREATE_UPDATESTEP( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * update_name = read_alloc_string( stream , binary );
  local_config_alloc_updatestep( config , update_name );
  free( update_name );
}



static void local_config_CREATE_MINISTEP( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * mini_name = read_alloc_string( stream , binary );
  char * obs_name  = read_alloc_string( stream , binary );
  local_config_alloc_ministep( config , mini_name , obs_name );
  free( mini_name );
  free( obs_name );
}


static void local_config_CREATE_OBSSET( local_config_type * config , local_context_type * context , FILE * stream , bool binary)  {
  char * obs_name = read_alloc_string( stream , binary );
  local_config_alloc_obsset( config , obs_name);
  free( obs_name );
}



static void local_config_ATTACH_MINISTEP( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * update_name = read_alloc_string( stream , binary );
  char * mini_name   = read_alloc_string( stream , binary );
  {
    local_updatestep_type * update   = local_config_get_updatestep( config , update_name );
    local_ministep_type   * ministep = local_config_get_ministep( config , mini_name );
    local_updatestep_add_ministep( update , ministep );
  }
  free( update_name );
  free( mini_name );
}



static void local_config_CREATE_DATASET( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * dataset_name = read_alloc_string( stream , binary );
  local_config_alloc_dataset( config , dataset_name );
}


static void local_config_COPY_DATASET( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * src_name    = read_alloc_string( stream , binary );
  char * target_name = read_alloc_string( stream , binary );
  local_config_alloc_dataset_copy( config , src_name , target_name );
  free( target_name );
  free( src_name );
}

static void local_config_COPY_OBSSET( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * src_name     = read_alloc_string( stream , binary );
  char * target_name = read_alloc_string( stream , binary );
  local_config_alloc_obsset_copy( config , src_name , target_name );
  free( target_name );
  free( src_name );
}


static void local_config_ATTACH_DATASET( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * mini_name = read_alloc_string( stream , binary );
  char * dataset_name = read_alloc_string( stream , binary );
  {
    local_ministep_type * ministep = local_config_get_ministep( config , mini_name );
    local_dataset_type * dataset = local_config_get_dataset( config , dataset_name );
    local_ministep_add_dataset( ministep , dataset );
  }
  free( mini_name );
  free( dataset_name );
}

static void local_config_ADD_DATA( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * dataset_name = read_alloc_string( stream , binary );
  char * data_key = read_alloc_string( stream , binary );
  {
    local_dataset_type   * dataset = local_config_get_dataset( config , dataset_name );
    local_dataset_add_node( dataset , data_key );
  }
  free( data_key );
  free( dataset_name );
}

static void local_config_ADD_OBS( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * obs_name = read_alloc_string( stream , binary );
  char * obs_key  = read_alloc_string( stream , binary );
  {
    local_obsset_type * obsset = local_config_get_obsset( config , obs_name );
    local_obsset_add_obs( obsset , obs_key );
  }
  free( obs_name );
  free( obs_key );
}

static void local_config_ACTIVE_LIST_ADD_OBS_INDEX( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * obs_name = read_alloc_string( stream , binary );
  char * obs_key  = read_alloc_string( stream , binary );
  int index = read_int( stream , binary );
  {
    local_obsset_type * obsset  = local_config_get_obsset( config , obs_name );
    active_list_type  * active_list = local_obsset_get_obs_active_list( obsset , obs_key );
    active_list_add_index( active_list , index );
  }
  free( obs_name );
  free( obs_key );
}


static void local_config_ADD_DATA_INDEX( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * dataset_name = read_alloc_string( stream , binary );
  char * data_key     = read_alloc_string( stream , binary );
  int index           = read_int( stream , binary );
  {
    local_dataset_type * dataset     = local_config_get_dataset( config , dataset_name );
    active_list_type   * active_list = local_dataset_get_node_active_list( dataset , data_key );
    active_list_add_index( active_list , index );
  }
  free( data_key );
  free( dataset_name );
}


static void local_config_ACTIVE_LIST_ADD_MANY_OBS_INDEX( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  int_vector_type * int_vector = int_vector_alloc(0,0);
  char * obs_name = read_alloc_string( stream , binary );
  char * obs_key  = read_alloc_string( stream , binary );
    
  read_int_vector( stream , binary , int_vector);
  {
    local_obsset_type * obsset  = local_config_get_obsset( config , obs_name );
    active_list_type  * active_list = local_obsset_get_obs_active_list( obsset , obs_key );
    for (int i = 0; i < int_vector_size( int_vector ); i++)
      active_list_add_index( active_list , int_vector_iget(int_vector , i));
  }
  free( obs_key );
  free( obs_name );
  int_vector_free( int_vector );
}


static void local_config_ACTIVE_LIST_ADD_MANY_DATA_INDEX( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  int_vector_type * int_vector = int_vector_alloc( 0 , 0 );
  char * dataset_name = read_alloc_string( stream , binary );
  char * data_key     = read_alloc_string( stream , binary );

  read_int_vector( stream , binary , int_vector);
  {
    local_dataset_type * dataset     = local_config_get_dataset( config , dataset_name );
    active_list_type   * active_list = local_dataset_get_node_active_list( dataset , data_key );
    for (int i = 0; i < int_vector_size( int_vector ); i++)
      active_list_add_index( active_list , int_vector_iget(int_vector , i));
  }
  free( data_key );
  free( dataset_name );
  int_vector_free( int_vector );
}

static void local_config_INSTALL_UPDATESTEP( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * update_name = read_alloc_string( stream , binary );
  {
    int step1,step2;
    
    step1 = read_int( stream , binary );
    step2 = read_int( stream , binary );
    local_config_set_updatestep( config , step1 , step2 , update_name );
  }
  free( update_name );
}

static void local_config_INSTALL_DEFAULT_UPDATESTEP( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * update_name = read_alloc_string( stream , binary );
  local_config_set_default_updatestep( config , update_name );
  free( update_name );
}

static void local_config_DEL_DATA( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * dataset_name = read_alloc_string( stream , binary );
  char * data_key  = read_alloc_string( stream , binary );
  {
    local_dataset_type * dataset = local_config_get_dataset( config , dataset_name );
    local_dataset_del_node( dataset , data_key );
  }
  free( data_key );
  free( dataset_name );
}


static void local_config_DEL_OBS( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * obs_name = read_alloc_string( stream , binary );
  char * obs_key  = read_alloc_string( stream , binary );
  {
    local_obsset_type * obsset = local_config_get_obsset( config , obs_name );
    local_obsset_del_obs( obsset , obs_key );
  }
  free( obs_name );
  free( obs_key );
}

static void local_config_DATASET_DEL_ALL_DATA( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * dataset_name = read_alloc_string( stream , binary );
  {
    local_dataset_type * dataset = local_config_get_dataset( config , dataset_name );
    local_dataset_clear( dataset );
  }
  free( dataset_name );
}

static void local_config_OBSSET_DEL_ALL_OBS( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * obs_name = read_alloc_string( stream , binary );
  {
    local_obsset_type   * obsset = local_config_get_obsset( config , obs_name );
    local_obsset_clear( obsset );
  }
  free( obs_name );
}


static void local_config_ADD_FIELD( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * dataset_name = read_alloc_string( stream , binary );
  char * field_name   = read_alloc_string( stream , binary );
  char * region_name  = read_alloc_string( stream , binary );
  {
    ecl_region_type     * region  = local_context_get_ecl_region( context , region_name );
    local_dataset_type  * dataset = local_config_get_dataset( config , dataset_name );
    local_dataset_add_node( dataset , field_name );
    {
      active_list_type * active_list        = local_dataset_get_node_active_list( dataset , field_name );
      const int_vector_type * region_active = ecl_region_get_active_list( region );
      
      for (int i=0; i < int_vector_size( region_active ); i++)
        active_list_add_index( active_list , int_vector_iget( region_active , i ) );
    }
  }
  free( field_name );
  free( dataset_name );
  free( region_name );
}

static void local_config_CREATE_ECLREGION( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * region_name  = read_alloc_string( stream , binary );
  bool   preselect    = read_bool( stream , binary );
  local_context_create_ecl_region( context , GLOBAL_GRID , region_name , preselect );
  free( region_name );
}

static void local_config_LOAD_FILE( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * file_key  = read_alloc_string( stream , binary );
  char * file_name = read_alloc_string( stream , binary );

  local_context_load_file( context , file_name , file_key ); /*  */
  
  free( file_key );
  free( file_name );
}


static void local_config_ECLREGION_SELECT_BOX( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * region_name = read_alloc_string( stream , binary );
  int i1          = read_int( stream , binary ) - 1;
  int i2          = read_int( stream , binary ) - 1;
  int j1          = read_int( stream , binary ) - 1;
  int j2          = read_int( stream , binary ) - 1;
  int k1          = read_int( stream , binary ) - 1;
  int k2          = read_int( stream , binary ) - 1;
  bool select     = read_bool( stream , binary );
  
  ecl_region_type * region = local_context_get_ecl_region( context , region_name );
  
  if (select)
    ecl_region_select_from_ijkbox( region , i1 , i2 , j1 , j2 , k1 , k2);
  else
    ecl_region_deselect_from_ijkbox( region , i1 , i2 , j1 , j2 , k1 , k2);
  
  free( region_name );
}


static void local_config_ECLREGION_SELECT_SLICE( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * region_name = read_alloc_string( stream , binary );
  char * dir         = read_alloc_string( stream , binary );
  int n1             = read_int( stream , binary) - 1;
  int n2             = read_int( stream , binary) - 1;
  bool     select    = read_bool( stream , binary );

  ecl_region_type * region = local_context_get_ecl_region( context , region_name );
  
  util_strupr( dir );
  
  if (strcmp( dir , "X") == 0) {
    if (select)
      ecl_region_select_i1i2( region , n1 , n2 );
    else
      ecl_region_deselect_i1i2( region , n1 , n2 );
  } else if (strcmp(dir , "Y") == 0) {
    if (select)
      ecl_region_select_j1j2( region , n1 , n2 );
    else
      ecl_region_deselect_j1j2( region , n1 , n2 );
  } else if (strcmp(dir , "Z") == 0) {
    if (select)
      ecl_region_select_k1k2( region , n1 , n2 );
    else
      ecl_region_deselect_k1k2( region , n1 , n2 );
  } else
    util_abort("%s: slice direction:%s not recognized \n",__func__ , dir );
  
  free(dir );
  free( region_name );
}


static void local_config_ECLREGION_SELECT_ALL( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * region_name = read_alloc_string( stream , binary );
  {
    bool select = read_bool( stream , binary );
    ecl_region_type * region = local_context_get_ecl_region( context , region_name );
    if (select)
      ecl_region_select_all( region );
    else
      ecl_region_deselect_all( region );
  }
  free( region_name );
}


static void local_config_ECLREGION_SELECT_VALUE( local_config_type * config , local_context_type * context , FILE * stream , bool binary , local_config_instruction_type cmd) {
  char * region_name  = read_alloc_string( stream , binary );
  char * master_key   = read_alloc_string( stream , binary );
  char * value_string = read_alloc_string( stream , binary );
  bool select         = read_bool( stream , binary );
  {
    ecl_kw_type * ecl_kw;
    ecl_region_type * region;

    { 
      stringlist_type * key_list = stringlist_alloc_from_split( master_key , ":");
      ecl_file_type * ecl_file   = local_context_get_file( context , stringlist_iget(key_list , 0 ) );
      int key_nr = 0;

      if (stringlist_get_size( key_list ) == 3)
        util_sscanf_int( stringlist_iget( key_list , 2 ) , &key_nr );
    
      ecl_kw = ecl_file_iget_named_kw( ecl_file , stringlist_iget( key_list , 1 ) , key_nr);
      stringlist_free( key_list );
    }

    region = local_context_get_ecl_region( context , region_name );
    
    if (cmd == ECLREGION_SELECT_VALUE_EQUAL) {
      int value;
      util_sscanf_int( value_string , &value );
      if (select)
        ecl_region_select_equal( region , ecl_kw , value );
      else
        ecl_region_deselect_equal( region , ecl_kw , value);
    } else {
      double value;
      util_sscanf_double( value_string , &value );
      
      if (cmd == ECLREGION_SELECT_VALUE_LESS) {
        if (select)
          ecl_region_select_smaller( region , ecl_kw , value );
        else
          ecl_region_deselect_smaller( region , ecl_kw , value);
      } else if (cmd == ECLREGION_SELECT_VALUE_LESS) {
        if (select)
          ecl_region_select_larger( region , ecl_kw , value );
        else
          ecl_region_deselect_larger( region , ecl_kw , value);
      }
      
    }
  }
  free( master_key );
  free( value_string );
  free(region_name);
}


static void local_config_ECLREGION_SELECT_PLANE( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  double normal_vec[3];
  double p0[3];
  double sign;
  bool   select;
  ecl_region_type * region;
  char * region_name  = read_alloc_string( stream , binary );
  
  normal_vec[0] = read_double( stream , binary );
  normal_vec[1] = read_double( stream , binary );
  normal_vec[2] = read_double( stream , binary );
  
  p0[0]         = read_double( stream , binary );
  p0[1]         = read_double( stream , binary );
  p0[2]         = read_double( stream , binary );
  
  sign          = read_double( stream , binary);
  select        = read_bool( stream , binary );
  
  region = local_context_get_ecl_region( context , region_name );
  if (select) {
    if (sign > 0)
      ecl_region_select_above_plane( region , normal_vec , p0 );
    else
      ecl_region_select_below_plane( region , normal_vec , p0 );
  } else {
    if (sign > 0)
      ecl_region_deselect_above_plane( region , normal_vec , p0 );
    else
      ecl_region_deselect_below_plane( region , normal_vec , p0 );
  }

  free( region_name );
}


static void local_config_CREATE_POLYGON( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * polygon_name = read_alloc_string( stream , binary );
  local_context_add_polygon( context , polygon_name );
  {
    geo_polygon_type * polygon = local_context_get_polygon( context , polygon_name );
    int num_points   = read_int( stream , binary );
    
    if (num_points < 2)
      util_abort("%s: error when parsing CREATE_POLYGON - need at least 3 points in polygon\n",__func__);
    
    for (int i=0; i < num_points; i++) {
      double x = read_double( stream , binary );
      double y = read_double( stream , binary );
      
      geo_polygon_add_point( polygon , x , y );
    }
  }
  free( polygon_name );
}

static void local_config_LOAD_POLYGON( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * polygon_name = read_alloc_string( stream , binary );
  char * polygon_file = read_alloc_string( stream , binary );

  local_context_load_polygon( context , polygon_name , polygon_file );

  free( polygon_file );
  free( polygon_name );
}


static void local_config_ECLREGION_SELECT_IN_POLYGON( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  
  char * region_name  = read_alloc_string( stream , binary );
  char * polygon_name = read_alloc_string( stream , binary );
  bool select       = read_bool( stream , binary );
  {
    ecl_region_type * region;
    geo_polygon_type * polygon;

    polygon = local_context_get_polygon( context , polygon_name );
    region  = local_context_get_ecl_region( context , region_name );
    if (select) 
      ecl_region_select_inside_polygon( region , polygon );
    else
      ecl_region_select_inside_polygon( region , polygon );
  }
  free( polygon_name );
  free( region_name );
}


static void local_config_LOAD_SURFACE( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * surface_name = read_alloc_string( stream , binary );
  char * surface_file = read_alloc_string( stream , binary );
  
  local_context_load_surface( context , surface_name , surface_file );
  
  free( surface_file );
  free( surface_name );
}


static void local_config_CREATE_SURFACE_REGION( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * region_name       = read_alloc_string( stream , binary );  
  char * base_surface      = read_alloc_string( stream , binary );  
  bool preselect           = read_bool( stream , binary );                
  
  local_context_create_surface_region( context , base_surface , region_name ,  preselect);
  
  free( region_name );
  free( base_surface);
}


static void local_config_SURFACE_REGION_SELECT_IN_POLYGON( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  char * region_name       = read_alloc_string( stream , binary );
  char * polygon_name      = read_alloc_string( stream , binary );
  bool select              = read_bool( stream , binary );        

  geo_region_type * region   = local_context_get_surface_region( context , region_name );
  geo_polygon_type * polygon = local_context_get_polygon( context , polygon_name );

  if (select)
    geo_region_select_inside_polygon( region , polygon );
  else
    geo_region_deselect_inside_polygon( region , polygon );

}

static void local_config_SURFACE_REGION_SELECT_LINE( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {
  double sign;
  bool   select;
  double xcoords[2];
  double ycoords[2];
  char * region_name        = read_alloc_string( stream , binary );
  xcoords[0]                = read_double( stream  , binary );       
  ycoords[0]                = read_double( stream  , binary );       
  xcoords[1]                = read_double( stream  , binary );       
  ycoords[1]                = read_double( stream  , binary );       
  sign                      = read_double( stream , binary );
  select                    = read_bool( stream , binary );        
  
  geo_region_type * region   = local_context_get_surface_region( context , region_name );

  if (select) {
    if (sign > 0)
      geo_region_select_above_line( region , xcoords , ycoords );
    else
      geo_region_select_below_line( region , xcoords , ycoords );
  } else {
    if (sign > 0)
      geo_region_deselect_above_line( region , xcoords , ycoords );
    else
      geo_region_deselect_below_line( region , xcoords , ycoords );
  }
}



static void local_config_ADD_DATA_SURFACE( local_config_type * config , local_context_type * context , FILE * stream , bool binary) {

  char * dataset_name = read_alloc_string( stream , binary );
  char * surface_name = read_alloc_string( stream , binary );
  char * region_name  = read_alloc_string( stream , binary );

  {
    geo_region_type     * region  = local_context_get_surface_region( context , region_name );
    local_dataset_type  * dataset = local_config_get_dataset( config , dataset_name );
    local_dataset_add_node( dataset , surface_name );
    {
      active_list_type * active_list        = local_dataset_get_node_active_list( dataset , surface_name );
      const int_vector_type * region_active = geo_region_get_index_list( region );
      
      for (int i=0; i < int_vector_size( region_active ); i++) 
        active_list_add_index( active_list , int_vector_iget( region_active , i ) );

    }
  }

  free( surface_name );
  free( dataset_name );
  free( region_name );
}

/**
   CURRENTLYy the ensemble_config and enkf_obs objects are not used for
   anything. These should be used for input validation.
*/

static void local_config_load_file( local_config_type * local_config ,
                                    const ecl_grid_type * ecl_grid ,
                                    const ensemble_config_type * ensemble_config ,
                                    const enkf_obs_type * enkf_obs  ,
                                    const char * config_file) {
  bool binary = false;
  local_config_instruction_type cmd;

  hash_type * cmd_table = hash_alloc();
  local_context_type * context = local_context_alloc( ecl_grid );
  FILE * stream        = util_fopen( config_file , "r");

  local_config_init_cmd_table( cmd_table );

  while ( read_cmd( cmd_table , stream, binary , &cmd)) {
    switch(cmd) {
    case(CREATE_UPDATESTEP):
      local_config_CREATE_UPDATESTEP( local_config , context , stream , binary );
      break;
    case(CREATE_MINISTEP):
      local_config_CREATE_MINISTEP( local_config , context , stream , binary );
      break;
    case(CREATE_OBSSET):
      local_config_CREATE_OBSSET( local_config , context , stream , binary );
      break;
    case(ATTACH_MINISTEP):
      local_config_ATTACH_MINISTEP( local_config , context , stream , binary );
      break;
    case(CREATE_DATASET):
      local_config_CREATE_DATASET( local_config , context , stream , binary );
      break;
    case(COPY_DATASET):
      local_config_COPY_DATASET( local_config , context , stream , binary );
      break;
    case(COPY_OBSSET):
      local_config_COPY_OBSSET( local_config , context , stream , binary );
      break;
    case(ATTACH_DATASET):
      local_config_ATTACH_DATASET( local_config , context , stream , binary );
      break;
    case(ADD_DATA):
      local_config_ADD_DATA( local_config , context , stream , binary );
      break;
    case(ADD_OBS):
      local_config_ADD_OBS( local_config , context , stream , binary );
      break;
    case(ACTIVE_LIST_ADD_OBS_INDEX):
      local_config_ACTIVE_LIST_ADD_OBS_INDEX( local_config , context , stream , binary );
      break;
    case(ACTIVE_LIST_ADD_DATA_INDEX):
      local_config_ADD_DATA_INDEX( local_config , context , stream , binary );
      break;
    case(ACTIVE_LIST_ADD_MANY_OBS_INDEX):
      local_config_ACTIVE_LIST_ADD_MANY_OBS_INDEX( local_config , context , stream , binary );
      break;
    case(ACTIVE_LIST_ADD_MANY_DATA_INDEX):
      local_config_ACTIVE_LIST_ADD_MANY_DATA_INDEX( local_config , context , stream , binary );
      break;
    case(INSTALL_UPDATESTEP):
      local_config_INSTALL_UPDATESTEP( local_config , context , stream , binary );
      break;
    case(INSTALL_DEFAULT_UPDATESTEP):
      local_config_INSTALL_DEFAULT_UPDATESTEP( local_config , context , stream , binary );
      break;
    case(DEL_DATA):
      local_config_DEL_DATA( local_config , context , stream , binary );
      break;
    case(DEL_OBS):
      local_config_DEL_OBS( local_config , context , stream , binary );
      break;
    case(DATASET_DEL_ALL_DATA):
      local_config_DATASET_DEL_ALL_DATA( local_config , context , stream , binary );
      break;
    case(OBSSET_DEL_ALL_OBS):
      local_config_OBSSET_DEL_ALL_OBS( local_config , context , stream , binary );
      break;
    case(ADD_FIELD):
      local_config_ADD_FIELD( local_config , context , stream , binary );
      break;
    case(CREATE_ECLREGION):
      local_config_CREATE_ECLREGION( local_config , context , stream , binary );
      break;
    case(LOAD_FILE):
      local_config_LOAD_FILE( local_config , context , stream , binary );
      break;
    case( ECLREGION_SELECT_BOX ):  /* The coordinates in the box are inclusive in both upper and lower limit,
                                      and the counting starts at 1. */
      local_config_ECLREGION_SELECT_BOX( local_config , context , stream , binary );
      break;
    case( ECLREGION_SELECT_SLICE ):
      local_config_ECLREGION_SELECT_SLICE( local_config , context , stream , binary );
      break;
    case( ECLREGION_SELECT_ALL ):
      local_config_ECLREGION_SELECT_ALL( local_config , context , stream , binary );
      break;
    case( ECLREGION_SELECT_VALUE_LESS  ):
    case( ECLREGION_SELECT_VALUE_EQUAL ):
    case( ECLREGION_SELECT_VALUE_MORE  ):
      local_config_ECLREGION_SELECT_VALUE( local_config , context , stream , binary , cmd);
      break;
    case( ECLREGION_SELECT_PLANE ):
      local_config_ECLREGION_SELECT_PLANE( local_config , context , stream , binary );
      break;
    case(CREATE_POLYGON):
      local_config_CREATE_POLYGON( local_config , context , stream , binary );
      break;
    case(LOAD_POLYGON):
      local_config_LOAD_POLYGON( local_config , context , stream , binary );
      break;
    case(ECLREGION_SELECT_IN_POLYGON):
      local_config_ECLREGION_SELECT_IN_POLYGON( local_config , context , stream , binary );
      break;
    case(LOAD_SURFACE):
      local_config_LOAD_SURFACE( local_config , context , stream , binary );
      break;
    case(CREATE_SURFACE_REGION):
      local_config_CREATE_SURFACE_REGION( local_config , context , stream , binary );
      break;
    case(SURFACE_REGION_SELECT_IN_POLYGON):
      local_config_SURFACE_REGION_SELECT_IN_POLYGON( local_config , context , stream , binary );
      break;
    case(SURFACE_REGION_SELECT_LINE):
      local_config_SURFACE_REGION_SELECT_LINE( local_config , context , stream , binary );
      break;
    case(ADD_DATA_SURFACE):
      local_config_ADD_DATA_SURFACE( local_config , context , stream , binary );
      break;
    default:
      util_abort("%s: invalid command:%d \n",__func__ , cmd);
    }
  }
  fclose(stream);
  local_context_free( context );
  hash_free( cmd_table );
}



/*
  Should probably have a "modified" flag to ensure internal consistency
*/

void local_config_reload( local_config_type * local_config ,
                          const ecl_grid_type * ecl_grid ,
                          const ensemble_config_type * ensemble_config ,
                          const enkf_obs_type * enkf_obs  ,
                          const char * all_active_config_file ) {

  local_config_clear( local_config );
  if (all_active_config_file != NULL)
    local_config_load_file( local_config , ecl_grid , ensemble_config , enkf_obs , all_active_config_file );
  {
    int i;
    for (i = 0; i < stringlist_get_size( local_config->config_files ); i++)
      local_config_load_file( local_config , ecl_grid , ensemble_config , enkf_obs , stringlist_iget( local_config->config_files , i ) );
  }
}



void local_config_fprintf( const local_config_type * local_config , const char * config_file) {
  FILE * stream = util_mkdir_fopen( config_file , "w");

  /* Start with dumping all the ministep instances. */
  {
    hash_iter_type * hash_iter = hash_iter_alloc( local_config->ministep_storage );

    while (!hash_iter_is_complete( hash_iter )) {
      const local_ministep_type * ministep = hash_iter_get_next_value( hash_iter );
      local_ministep_fprintf( ministep , stream );
    }

    hash_iter_free( hash_iter );
  }


  /* Dumping all the reportstep instances as ATTACH_MINISTEP commands. */
  {
    hash_iter_type * hash_iter = hash_iter_alloc( local_config->updatestep_storage );

    while (!hash_iter_is_complete( hash_iter )) {
      const local_updatestep_type * updatestep = hash_iter_get_next_value( hash_iter );
      local_updatestep_fprintf( updatestep , stream );
    }

    hash_iter_free( hash_iter );
  }

  /* Writing out the updatestep / time */
  {
    int i;
    for (i=0; i < vector_get_size( local_config->updatestep ); i++) {
      const local_updatestep_type * updatestep = vector_iget_const( local_config->updatestep , i );
      if (updatestep != NULL)
        fprintf(stream , "%s %s %d %d \n", local_config_get_cmd_string( INSTALL_UPDATESTEP ) , local_updatestep_get_name( updatestep ) , i , i );
    }
  }

  /* Installing the default updatestep */
  if (local_config->default_updatestep != NULL)
    fprintf(stream , "%s %s\n", local_config_get_cmd_string( INSTALL_DEFAULT_UPDATESTEP ) , local_updatestep_get_name( local_config->default_updatestep ));

  fclose( stream );
}



void local_config_fprintf_config( const local_config_type * local_config , FILE * stream) {
  fprintf( stream , CONFIG_COMMENTLINE_FORMAT );
  fprintf( stream , CONFIG_COMMENT_FORMAT , "Here comes the config files used for setting up local analysis.");
  for (int i=0; i < stringlist_get_size( local_config->config_files ); i++) {
    fprintf(stream , CONFIG_KEY_FORMAT      , LOCAL_CONFIG_KEY );
    fprintf(stream , CONFIG_ENDVALUE_FORMAT , stringlist_iget( local_config->config_files , i ));
  }
}
