
Keywords for the local configuration file
=========================================



General overview
----------------

To create a configuration for localization you must "program" your own
configuration file, this file is then loaded from the ert/enkf proper
application. The 'commands' available in the local_config programming
language are listed below. 

An alterative way to 'program' the local config commands is by writing a Python script, and invoking it from a workflow.
Not all the commands available from the local config programming are supported for Python scripting. 


**Local config python script example:**

::

 from ert.enkf import ErtScript
 from ert.enkf import LocalConfig, LocalObsdata, LocalObsdataNode, LocalMinistep, LocalUpdateStep, LocalDataset, ActiveList
 from ert.ecl import EclGrid, EclRegion, Ecl3DKW, EclFile, EclInitFile, EclKW, EclTypeEnum

 class LocalConfigJob(ErtScript):
 
 
     def run(self):
     
         # This example can be used with the REEK data set from the ERT tutorial
 
         # Get the ert object
         ert = self.ert()
 
         # Get local config object
         local_config = ert.getLocalConfig()
 
         # Reset internal local config structure. From now you need to specify what to localize
         local_config.clear()
 
         # There is only one update step
         updatestep = local_config.getUpdatestep()
 
         # A ministep
         ministep   = local_config.createMinistep("MINISTEP" )
 
         # Add some dataset you want to localize here.
         dataset_multflt = local_config.createDataset("DATASET_MULTFLT")
 
         # Add some field and localize inside a box
         data_poro = local_config.createDataset("DATA_PORO")
         ecl_grid = local_config.getGrid()
         ecl_region = EclRegion(ecl_grid, False)
         ecl_region.select_box((0,0,0),(3,3,3))
         data_poro.addField("PORO", ecl_region)
 
 
         # Add some index from MULTFLT to the dataset
         dataset_multflt.addNode("MULTFLT")
         active_list = dataset_multflt.getActiveList("MULTFLT")
         active_list.addActiveIndex(0)
 
         # Add existing observations from WOPR:OP_1. Alternatively, use getObservations and filter the observations you want to use for this ministep.
         obsdata_wopr = local_config.createObsdata("WOPR:OP_1_10")
         for i in range(1,10):
             obsdata_wopr.addNode("OBS"+str(i))
 
         # Attach the created dataset and obsset to the ministep
         ministep.attachDataset(dataset_multflt)
         ministep.attachObsset(obsdata_wopr)
 
         # Then attach the ministep to the update step
         updatestep.attachMinistep(ministep)
 
         # Write a .csv file for debugging. The generated file can be imported into Excel for a better tabulation of the setup
         local_config.writeSummaryFile("tmp/summary_local_config.csv")
        




List of keywords  
----------------
===========================================================================================  ===========================================================   ==============================================================================================================================================
Keyword name                                                                                 ERT script function                                           Purpose
===========================================================================================  ===========================================================   ==============================================================================================================================================
:ref:`CREATE_UPDATESTEP                <create_updatestep>`                                  getUpdatestep                                                 Creates/gets default updatestep
:ref:`CREATE_MINISTEP                  <create_ministep>`                                    createMinistep                                                Creates ministep
:ref:`CREATE_DATASET                   <create_dataset>`                                     createDataset                                                 Creates dataset
:ref:`COPY_DATASET                     <copy_dataset>`                                       copyDataset                                                   Deep copy of dataset
:ref:`CREATE_OBSSET                    <create_obsset>`                                      createObsdata                                                 Creates observation set
:ref:`COPY_OBSSET                      <copy_obsset>`                                        copyObsdata                                                   Deep copy of observation set
:ref:`ATTACH_MINISTEP                  <attach_ministep>`                                    attachMinistep                                                Attaches ministep to update step 
:ref:`ATTACH_DATASET                   <attach_dataset>`                                     attachDataset                                                 Attaches dataset to mini step
:ref:`ATTACH_OBSSET                    <attach_obsset>`                                      attachObsset                                                  Attaches observation set to mini step
:ref:`ADD_DATA                         <add_data>`                                           addNode                                                       Adds data node to dataset
:ref:`DEL_DATA                         <del_data>`                                           del                                                           Deletes observation node from dataset
:ref:`ADD_OBS                          <add_obs>`                                            addNode, addNodeAndRange                                      Adds observation node to observation set for all times or in a given time range
:ref:`DEL_OBS                          <del_obs>`                                            del                                                           Deletes observation node from observation set
:ref:`DATASET_DEL_ALL_DATA             <dataset_del_all_data>`                               clear                                                         Delete all the data keys from a dataset
:ref:`ACTIVE_LIST_ADD_DATA_INDEX       <active_list_add_data_index>`                         addActiveIndex                                                Adds data index to the list of active indices
:ref:`ACTIVE_LIST_ADD_OBS_INDEX        <active_list_add_obs_index>`                          addActiveIndex                                                Adds observation index to the list of active indices  
:ref:`ACTIVE_LIST_ADD_MANY_DATA_INDEX  <active_list_add_many_data_index>`                    addActiveIndex                                                Adds several data indices to the list of active indices
:ref:`ACTIVE_LIST_ADD_MANY_OBS_INDEX   <active_list_add_many_obs_index>`                     addActiveIndex                                                Adds several observation indinces to the list of active indices
:ref:`ADD_FIELD                        <add_field>`                                          addField                                                      Adds field node to dataset
:ref:`LOAD_FILE                        <load_file>`                                          EclGrid, EclInitFile,                                         Loads eclipse file in restart format
:ref:`CREATE_ECLREGION                 <create_eclregion>`                                   EclRegion                                                     Creates a new region for use when defining active regions for fields
:ref:`ECLREGION_SELECT_ALL             <eclregion_select_all>`                               select_active                                                 Selects or deselects cells in a region
:ref:`ECLREGION_SELECT_VALUE_EQUAL     <eclregion_select_value_equal>`                       select_equal                                                  Selects or deselects cells in a region equal to given value
:ref:`ECLREGION_SELECT_VALUE_LESS      <eclregion_select_value_less>`                        select_less                                                   Selects or deselects cells in a region equal less than a given value
:ref:`ECLREGION_SELECT_VALUE_MORE      <eclregion_select_value_more>`                        select_more                                                   Selects or deselects cells in a region equal greater than a given value
:ref:`ECLREGION_SELECT_BOX             <eclregion_select_box>`                               select_box                                                    Selects or deselects cells in a box
:ref:`ECLREGION_SELECT_SLICE           <eclregion_select_slice>`                             select_islice, select_jslice, select_kslice                   Selects or deselects cells in a slice
:ref:`ECLREGION_SELECT_PLANE           <eclregion_select_plane>`                             select_below_plane                                            Selects or deselects cells in a half space defined by a plane
:ref:`ECLREGION_SELECT_IN_POLYGON      <eclregion_select_in_polygon>`                        select_inside_polygon                                         Selects or deselects cells in region inside polygon
:ref:`CREATE_POLYGON                   <create_polygon>`                                     :ref:`Example <create_polygon>`                               Creates a geo-polygon based on coordinate list
:ref:`LOAD_POLYGON                     <load_polygon>`                                       :ref:`Example <load_polygon>`                                 Loads polygon in Irap RMS format from file
:ref:`LOAD_SURFACE                     <load_surface>`                                                                                                     Loads surface in Irap RMS format from file
:ref:`CREATE_SURFACE_REGION            <create_surface_region>`                                                                                            Creates region to select or deselect parts of a surface
:ref:`SURFACE_REGION_SELECT_IN_POLYGON <surface_region_select_in_polygon>`                                                                                 Creates region to select or deselect parts of a surface
:ref:`SURFACE_REGION_SELECT_LINE       <surface_region_select_line>`                                                                                       Selects or deselects parts of a surface in half space define by a line
:ref:`ADD_DATA_SURFACE                 <add_data_surface>`                                                                                                 Adds surface node to dataset with elements in a surface region
|                                                                                            getObservations                                               Get the observations currently imported. Use to filter the observations to localize.
|                                                                                            getGrid                                                       Get the underlying grid. Use to define active cells in a field.
===========================================================================================  ===========================================================   ==============================================================================================================================================

.. ###########################################################################################################

.. _create_updatestep:
.. topic:: CREATE_UPDATESTEP 

  | This function will create a updatestep with the name 'NAME_OF_UPDATESTEP'. 
  | Observe that you must add (at least) one ministep to the updatestep, otherwise it will not be able to do anything.
  | Currently supports only one update step. It is kept here due to historical reasons when it was possible to have several update steps.
  
  *Example:*

  ::

    -- Updatestep     
    CREATE_UPDATESTEP DEFAULT

   
  *Example:*

  ::
  
    updatestep = local_config.getUpdatestep()

.. ###########################################################################################################


.. _create_ministep:
.. topic:: CREATE_MINISTEP 

  | This function will create a new ministep with the name 'NAME_OF_MINISTEP'. A given OBSSET can be attached to a given ministep.The ministep is then ready for adding data. Before the ministep can be used you must attach it to an updatestep with the ATTACH_MINISTEP command 
  
  *Example:*

  ::

    -- Ministep in updatestep 
    CREATE_MINISTEP MINISTEP

  *Example:*

  ::
  
    ministep = local_config.createMinistep("MINISTEP")



.. ###########################################################################################################

.. _create_dataset:
.. topic:: CREATE_DATASET 

  | This function will create a new dataset, i.e. a collection of enkf_nodes which should be updated together. Before you can actually use a dataset you must attach it to a ministep with the ATTACH_DATASET command.  
  
  *Example:*

  ::

    -- Create a DATASET_MULTFLT dataset
    CREATE_DATASET DATASET_MULTFLT
    
  *Example:*

  ::

    dataset_multflt = local_config.createDataset("DATASET_MULTFLT")    

.. ###########################################################################################################

.. _copy_dataset:
.. topic:: COPY_DATASET 

  | Will create a new local_obsset instance which is a copy of the 'SRC_OBSSET'; this is a deep copy where also the lowest level active_list instances are copied, and can then subsequently be updated independently of each other.


  *Example:*

  ::

    -- Deep copy DATASET_MULTFLT dataset
    COPY_DATASET DATASET_MULTFLT COPY_DATASET_MULTFLT 

.. ###########################################################################################################

.. _create_obsset:
.. topic:: CREATE_OBSSET 

  | This function will create an observation set, i.e. a collection of observation keys which will be used as the observations in one ministep. Before the obsset can be used it must be attached to a ministep with the ATTACH_OBSSET command.
  
  
  *Example:*

  ::

    -- Create a OBS_WELL obsset
    CREATE_OBSSET OBS_WELL
    
  *Example:*

  ::

    obsset_obs_well = local_config.createObsdata("OBS_WELL")       


.. ###########################################################################################################

.. _copy_obsset:
.. topic:: COPY_OBSSET 

  | Will create a new local_obsset instance which is a copy of the 'SRC_OBSSET'; this is a deep copy where also the lowest level active_list instances are copied, and can then subsequently be updated independently of each other.
  

  *Example:*

  ::

    -- Deep copy OBS_WELL observation set
    COPY_OBSSET OBS_WELL COPY_OBS_WELL 

.. ###########################################################################################################

.. _attach_ministep:
.. topic:: ATTACH_MINISTEP 

  | This function will attach the ministep 'NAME_OF_MINISTEP' to the updatestep 'NAME_OF_UPDATESTEP'; one ministep can be attached to many updatesteps.

  *Example:*

  ::

    -- Attach MINISTEP to UPDATESTEP
    ATTACH_MINISTEP UPDATESTEP MINISTEP

  *Example:*

  ::

    update_step.attachMinistep(ministep)       


.. ###########################################################################################################

.. _attach_dataset:
.. topic:: ATTACH_DATASET 

  | Will attach the dataset 'NAME_OF_DATASET' to the ministep given by 'NAME_OF_MINISTEP'.

  *Example:*

  ::

    -- Attach DATASET_MULTFLT to MINISTEP
    ATTACH_MINISTEP MINISTEP DATASET_MULTFLT

  *Example:*

  ::

    ministep.attachDataset(dataset_multflt)       


.. ###########################################################################################################

.. _attach_obsset:
.. topic:: ATTACH_OBSSET 

  | Will attach the obsset 'NAME_OF_OBSSET' to the ministep given by 'NAME_OF_MINISTEP'.
  
  *Example:*

  ::

    -- Attach OBS_WELL to MINISTEP
    ATTACH_MINISTEP MINISTEP OBS_WELL

  *Example:*

  ::

    ministep.attachObsset(obsset_obs_well)       


.. ###########################################################################################################

.. _add_data:
.. topic:: ADD_DATA 

  | This function will install 'KEY' as one enkf node which should be updated in this dataset. If you do not manipulate the KEY further with the ACTIVE_LIST_ADD_DATA_INDEX function the KEY will be added as 'ALL_ACTIVE', i.e. all elements will be updated.
  
  
  *Example:*

  ::

    -- Add data node to data set
    ADD_DATA DATASET_MULTFLT MULTFLT

  *Example:*

  ::

    dataset_multflt.addNode("MULTFLT")

.. ###########################################################################################################

.. _del_data:
.. topic:: DEL_DATA 

  | This function will delete the data 'KEY' from the dataset 'NAME_OF_DATASET'.
  
  
  *Example:*

  ::

    -- Delete data node from data set
    DEL_DATA DATASET_MULTFLT MULTFLT

  *Example:*

  ::

    del dataset_multflt["MULTFLT"]


.. ###########################################################################################################

.. _add_obs:
.. topic:: ADD_OBS 

  | This function will install the observation 'OBS_KEY' as an observation for this obsset - similarly to the ADD_DATA function.
  
  
  *Example:*

  ::

    -- Add data node to observation set
    ADD_OBS OBS_WELL WOPR:OBS_WELL

  *Example:*

  ::
  
    -- The obsset has a time range
    obsset_obs_well.addNodeAndRange("WOPR:OBS_WELL", 0, 1)
    
    -- All times are active
    obsset_obs_well.addNode("WOPR:OBS_WELL")


.. ###########################################################################################################

.. _del_obs:
.. topic:: DEL_OBS 

  | This function will delete the obs 'OBS_KEY' from the obsset 'NAME_OF_OBSSET'.
  
  
  *Example:*

  ::

    -- Delete data node from observation set
    DEL_OBS OBS_WELL WOPR:OBS_WELL

  *Example:*

  ::

    del obsset_obs_well["WOPR:OBS_WELL"]


.. ###########################################################################################################

.. _dataset_del_all_data:
.. topic:: DATASET_DEL_ALL_DATA 

  | This function will delete all the data keys from the dataset 'NAME_OF_DATASET'.
  
  
  *Example:*

  ::

    -- Delete all data nodes from DATASET_MULTFLT
    DATASET_DEL_ALL_DATA DATASET_MULTFLT

  *Example:*

  ::

    dataset_multflt.clear()
    
.. ###########################################################################################################

.. _active_list_add_data_index:
.. topic:: ACTIVE_LIST_ADD_DATA_INDEX 

  | This function will say that the data with name 'DATA_KEY' in dataset with name 'DATASTEP_NAME' should have the index 'INDEX' active.
  
  
  *Example:*

  ::

    -- Add index 0 from data MULTFLT to dataset DATASET_MULTFLT 
    ACTIVE_LIST_ADD_DATA_INDEX DATASET_MULTFLT MULTFLT 0

  *Example:*

  ::

    active_list = dataset_multflt.getActiveList("MULTFLT")
    active_list.addActiveIndex(0);

.. ###########################################################################################################

.. _active_list_add_obs_index:
.. topic:: ACTIVE_LIST_ADD_OBS_INDEX 

  | This function will say that the observation with name 'OBS_KEY' in obsset with name 'OBSSET_NAME' should have the index 'INDEX' active.
  
  
  *Example:*

  ::

    -- Add index 0 from data WOPR:OBS_WELL to obsset OBS_WELL 
    ACTIVE_LIST_ADD_OBS_INDEX OBS_WELL WOPR:OBS_WELL 0

  *Example:*

  ::

    active_list = obsset_obs_well.getActiveList("WOPR:OBS_WELL")
    active_list.addActiveIndex(0);

.. ###########################################################################################################

.. _active_list_add_many_data_index:
.. topic:: ACTIVE_LIST_ADD_MANY_DATA_INDEX

  | This function is similar to ACTIVE_LIST_ADD_DATA_INDEX, but it will add many indices.
  
  
  *Example:*

  ::

    -- Add indices 0, 1 and 2 from data MULTFLT to dataset DATASET_MULTFLT 
    ACTIVE_LIST_ADD_MANY_DATA_INDEX DATASET_MULTFLT MULTFLT 0 1 2


.. ###########################################################################################################

.. _active_list_add_many_obs_index:
.. topic:: ACTIVE_LIST_ADD_MANY_OBS_INDEX

  | This function is similar to ACTIVE_LIST_ADD_OBS_INDEX, but it will add many indices.
  
  
  *Example:*

  ::

    -- Add index 0, 1 and 2 from data WOPR:OBS_WELL to obsset OBS_WELL 
    ACTIVE_LIST_ADD_MANY_OBS_INDEX OBS_WELL WOPR:OBS_WELL 0 1 2

    
    
.. ###########################################################################################################

.. _add_field:
.. topic:: ADD_FIELD   

  | This function will install the node with name 'FIELD_NAME' in the dataset 'DATASET_NAME'. It will in addition select all the (currently) active cells in the region 'ECLREGION_NAME' as active for this field/ministep combination. The ADD_FIELD command is actually a shortcut of:   ADD_DATA   DATASET  FIELD_NAME; followed by: ACTIVE_LIST_ADD_MANY_DATA_INDEX  <All the indices from the region>
  
  
  
  *Example:*

  ::

    -- Add data node PORO to data set DATA_PORO activating indices in ECLREG_PORO     
    ADD_FIELD   DATA_PORO   PORO    ECLREG_PORO

  *Example:*

  ::
  
    # Load Eclipse grid
    ecl_grid = EclGrid("path/to/LOCAL.GRDECL")
    
    with open("path/to/LOCAL.GRDECL","r") as fileH:
        local_kw = Ecl3DKW.read_grdecl(ecl_grid, fileH, "LOCAL")
        
    # Define Eclipse region    
    eclreg_poro = EclRegion(ecl_grid, False)
    eclreg_poro.select_more(local_kw, 1)  
    
    # Create dataset and add field to dataset
    data_poro = local_config.createDataset("DATA_PORO")
    data_poro.addField("PORO", eclreg_poro)        


.. ###########################################################################################################

.. _load_file:
.. topic:: LOAD_FILE    

  | This function will load an ECLIPSE file in restart format (i.e. restart file or INIT file), the keywords in this file can then subsequently be used in ECLREGION_SELECT_VALUE_XXX commands below. The 'KEY' argument is a string which will be used later when we refer to the content of this file
  
  
  
  
  *Example:*

  ::

    -- Load Eclipse init file     
    LOAD_FILE  REFINIT path/to/FULLMODEL.INIT
    
  *Example:*

  ::
  
    # Load Eclipse grid and init file
    ecl_grid = EclGrid("path/to/FULLMODEL.GRDECL")
    refinit_file = EclInitFile(grid , "path/to/somefile.init")      

.. ###########################################################################################################

.. _create_eclregion:
.. topic:: CREATE_ECLREGION

  | This function will create a new region 'ECLREGION_NAME', which can subsequently be used when defining active regions for fields. The second argument, SELECT_ALL, is a boolean value. If this value is set to true the region will start with all cells selected, if set to false the region will start with no cells selected.
  
  
  *Example:*

  ::

    -- New Eclipse region with all cells inactive       
    CREATE_ECLREGION  ECL_REGION FALSE

  *Example:*

  ::
   
    # Define Eclipse region    
    eclreg_poro = EclRegion(ecl_grid, False)

.. ###########################################################################################################

.. _eclregion_select_all:
.. topic:: ECLREGION_SELECT_ALL

  | Will select all the cells in the region (or deselect if SELECT == FALSE).
  
  
  
  
  
  *Example:*

  ::

    -- Select cells in region       
    ECLREGION_SELECT_ALL  ECL_REGION TRUE
    
    
  *Example:*

  ::
         
    eclreg_poro.select_active()  
    
       
    

.. ###########################################################################################################

.. _eclregion_select_value_equal:
.. topic:: ECLREGION_SELECT_VALUE_EQUAL

  | This function will compare an ecl_kw instance loaded from file with a user supplied value, and select (or deselect) all cells which match this value. It is assumed that the ECLIPSE keyword is an INTEGER keyword, for float comparisons use the ECLREGION_SELECT_VALUE_LESS and ECLREGION_SELECT_VALUE_MORE functions.
  
  
  
  
  
  
  *Example:*

  ::

    -- Select cells in region ECL_REGION equal to 0     
    ECLREGION_SELECT_VALUE_EQUAL  ECL_REGION ECL_REGION:LOCAL 0 TRUE
    

  *Example:*

  ::
                      
    # Load Eclipse grid
    ecl_grid = EclGrid("path/to/LOCAL.GRDECL")
    
    with open("path/to/LOCAL.GRDECL","r") as fileH:
        local_kw = Ecl3DKW.read_grdecl(ecl_grid, fileH, "LOCAL", ecl_type= EclTypeEnum.ECL_INT_TYPE)
        
    # Define Eclipse region    
    eclreg_poro = EclRegion(ecl_grid, False)
    eclreg_poro.select_equal(local_kw, 1)
    print 'GRID LOADED%s' % ecl_grid 
    print ecl_grid.getDims()
    print local_kw.header   
    
        

.. ###########################################################################################################

.. _eclregion_select_value_less:
.. topic:: ECLREGION_SELECT_VALUE_LESS 

  | This function will compare an ecl_kw instance loaded from disc with a numerical value, and select all cells which have numerical below the limiting value. The ecl_kw value should be a floating point value like e.g. PRESSURE or PORO. The arguments are just as for ECLREGION_SELECT_VALUE_EQUAL. 
  
  
  
  
  
  *Example:*

  ::

    -- Select cells in region ECL_REGION less than 1     
    ECLREGION_SELECT_VALUE_LESS  ECL_REGION ECL_REGION:LOCAL 1 TRUE
    
  *Example:*

  ::
                          
    eclreg_poro.select_less(local_kw, 1) 
        
    
.. ###########################################################################################################

.. _eclregion_select_value_more:
.. topic:: ECLREGION_SELECT_VALUE_MORE 

  | This function will compare an ecl_kw instance loaded from disc with a numerical value, and select all cells which have numerical above the limiting value. The ecl_kw value should be a floating point value like e.g. PRESSURE or PORO. The arguments are just as for ECLREGION_SELECT_VALUE_EQUAL. 
  
  
  
  
  
  *Example:*

  ::

    -- Select cells in region ECL_REGION greater than 0     
    ECLREGION_SELECT_VALUE_MORE  ECL_REGION ECL_REGION:LOCAL 0 TRUE
    
  *Example:*

  ::
                          
    eclreg_poro.select_more(local_kw, 1)     
    
.. ###########################################################################################################

.. _eclregion_select_box:
.. topic:: ECLREGION_SELECT_BOX        

  | This function will select (or deselect) all the cells in the box defined by the six coordinates i1 i2 j1 j2 k1 k2. The coordinates are inclusive, and the counting starts at 1.   
    
  
  
  
  *Example:*

  ::

    -- Select cells in box [0,1] x [2,3] x [4,5]      
    ECLREGION_SELECT_BOX  ECL_REGION 0 1 2 3 4 5 TRUE
    
  *Example:*

  ::
                          
    eclreg_poro.select_box((0,2,4),(1,3,5))  
        


.. ###########################################################################################################

.. _eclregion_select_slice:
.. topic:: ECLREGION_SELECT_SLICE      

  | This function will select a slice in the direction given by 'dir', which can 'x', 'y' or 'z'. Depending on the value of 'dir' the numbers n1 and n2 are interpreted as (i1 i2), (j1 j2) or (k1 k2) respectively. The numbers n1 and n2 are inclusice and the counting starts at 1. It is OK to use very high/low values to imply "the rest of the cells" in one direction.
     
  
  
  *Example:*

  ::

    -- Select layer from z=2 to z=3      
    ECLREGION_SELECT_SLICE  ECL_REGION z 2 3 TRUE

  *Example:*

  ::
                          
    eclreg_poro.select_kslice(2,3)  


.. ###########################################################################################################

.. _eclregion_select_plane:
.. topic:: ECLREGION_SELECT_PLANE 

  | Will select all points which have positive (sign > 0) distance to the plane defined by normal vector n = (nx,ny,nz) and point p = (px,py,pz). If sign < 0 all cells with negative distance to plane will be selected.
  
  
  *Example:*

  ::

    -- Select half space defined by plane perpendicular to vector [1 1 1]       
    ECLREGION_SELECT_PLANE  ECL_REGION 1 1 1 0 0 0 -1 TRUE

  *Example:*

  ::
     
    eclreg_poro.select_below_plane((1,1,1),(0,0,0))


.. ###########################################################################################################

.. _eclregion_select_in_polygon:
.. topic:: ECLREGION_SELECT_IN_POLYGON 

  | Well select all the points which are inside the polygon with name 'POLYGON_NAME'. The polygon should have been created with command CREATE_POLYGON or loaded with command 'LOAD_POLYGON' first.
  
  
  
  
  *Example:*

  ::

    -- Select region inside polygon in xy plane      
    ECLREGION_SELECT_IN_POLYGON  POLYGON TRUE

  *Example:*

  ::
  
    polygon = [(0,0) , (0,1) , (1,0)]
    eclreg_poro.select_inside_polygon(polygon)
    
.. ###########################################################################################################

.. _create_polygon:
.. topic:: CREATE_POLYGON  

  | Will create a geo_polygon instance based on the coordinate list: (x1,y1), (x2,y2), (x3,y3), ... The polygon should not be explicitly closed - i.e. you should in general have (x1,y1) != (xn,yn). The polygon will be stored under the name 'POLYGON_NAME' - which should later be used when referring to the polygon in region select operations.
  

  
  *Example:*

  ::

    -- Create polygon in xy plane      
    CREATE_POLYGON  POLYGON 0 0 0 1 1 0 TRUE
    
  *Example:*

  ::
  
    polygon = [(0,0) , (0,1) , (1,0)]    

.. ###########################################################################################################

.. _load_polygon:
.. topic:: LOAD_POLYGON  

  | Will load a polygon instance from the file 'FILENAME' - the file should be in irap RMS format. The polygon will be stored under the name 'POLYGON_NAME' which can then later be used to refer to the polygon for e.g. select operations.  

    
  
  
  
  *Example:*

  ::

    -- Load polygon from RMS file      
    LOAD_POLYGON  path/to/polygon.irap
    
  *Example:*

  ::
      
    polygon = []
    with open("polygon.ply","r") as fileH:
     for line in fileH.readlines():
       tmp = line.split()
       polygon.append( (float(tmp[0]) , float(tmp[1])))

.. ###########################################################################################################

.. _load_surface:
.. topic:: LOAD_SURFACE  

  | Will load an irap surface from file 'SURFACE_FILE'. The surface will be stored internally as 'SURFACE_NAME' - this function is mainly needed to have a base surface available for the CREATE_SURFACE_REGION command.
    

    
  
  
  
  *Example:*

  ::

    -- Load Irap RMS surface from file      
    LOAD_SURFACE  path/to/surface.irap

.. ###########################################################################################################

.. _create_surface_region:
.. topic:: CREATE_SURFACE_REGION 

  | Will create a new surface region object which can be used to select and deselect parts of a surface. The region will be called 'REGION_NAME' and it will be based on the surface given by 'BASE_SURFACE'. 'PRESELECT' is a boolean 'TRUE' or 'FALSE' which determines whether the region is created with all points selected, or no points selected.
  
    
  
  *Example:*

  ::

    -- Create surface region in xy plane      
    CREATE_SURFACE_REGION  SURF_REGION BASE_SURFACE TRUE

.. ###########################################################################################################

.. _surface_region_select_in_polygon:
.. topic:: SURFACE_REGION_SELECT_IN_POLYGON 

  | Well select all the points which are inside the polygon with name 'POLYGON_NAME'. The polygon should have been created with command CREATE_POLYGON or loaded with command 'LOAD_POLYGON' first.
  
  
     
    
  
  
  
  *Example:*

  ::

    -- Select surface region inside polygon      
    SURFACE_REGION_SELECT_IN_POLYGON SURF_REGION TRIANGLE TRUE
            
            
.. ###########################################################################################################

.. _surface_region_select_line:
.. topic:: SURFACE_REGION_SELECT_LINE  

  | Well select|deselect all the points which are above|below the line: (x1,y1) -> (x2,y2). If SIGN is positive the select will apply to all points with a positive (right hand system) distance to the line; if SIGN is negative the selector will apply to all points with a negative distance to the line.
  
  
  
     
    
  
  
  
  *Example:*

  ::

    -- Select surface region inside a half space defined by a line from [0,0] to [1,1]     
    SURFACE_REGION_SELECT_LINE SURF_REGION 0 0 1 1 -1 TRUE
    
.. ###########################################################################################################

.. _add_data_surface:
.. topic:: ADD_DATA_SURFACE   

  | Will add the node 'SURFACE_NAME' (not one of the loaded surfaces, but an enkf_node object) to the dataset 'DATASET_NAME'. Only the elements in the region 'REGION_NAME' will be added. Typically SURFACE_REGION_SELECT_xxxx has been used first to build a suitable region selection.
  
  
  
  
     
    
  
  
  
  *Example:*

  ::

    -- Add EnKF node object to dataset DATA_MULTFLT, with elements in SURF_REGION from BASE_SURFACE      
    ADD_DATA_SURFACE DATA_MULTFLT BASE_SURFACE SURF_REGION
    
    

.. ###########################################################################################################


                
