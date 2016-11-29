
Keywords for the local configuration file
=========================================



General overview
----------------

To create a configuration for localization you must "program" your own
local config commands by writing a Python script, and invoking it from a workflow.


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
 
         # Write a .csv file for debugging. The generated file can be imported into Excel for better tabulation of the setup
         local_config.writeSummaryFile("tmp/summary_local_config.csv")

===========================================================================================   ==============================================================================================================================================
ERT script function                                                                           Purpose
===========================================================================================   ==============================================================================================================================================
:ref:`getObservations                  <all_obs>`                                             Get the observations currently imported, use to filter the observations to localize
:ref:`getGrid                          <ert_grid>`                                            Get the underlying grid use to define active cells in a field
:ref:`getUpdatestep                    <create_updatestep>`                                   Creates and gets default updatestep
:ref:`createMinistep                   <create_ministep>`                                     Creates ministep
:ref:`createDataset                    <create_dataset>`                                      Creates dataset
:ref:`copyDataset                      <copy_dataset>`                                        Deep copy of dataset
:ref:`createObsdata                    <create_obsset>`                                       Creates observation set
:ref:`copyObsdata                      <copy_obsset>`                                         Deep copy of observation set
:ref:`attachMinistep                   <attach_ministep>`                                     Attaches ministep to update step
:ref:`attachDataset                    <attach_dataset>`                                      Attaches dataset to mini step
:ref:`attachObsset                     <attach_obsset>`                                       Attaches observation set to mini step
:ref:`addNode                          <add_data>`                                            Adds data node to dataset
:ref:`del (data)                       <del_data>`                                            Deletes observation node from dataset
:ref:`addNode, addNodeAndRange         <add_obs>`                                             Adds observation node to observation set for all times or in a given time range
:ref:`del (obs)                        <del_obs>`                                             Deletes observation node from observation set
:ref:`clear                            <dataset_del_all_data>`                                Delete all the data keys from a dataset
:ref:`addActiveIndex (data)            <active_list_add_data_index>`                          Adds several data indices to the list of active indices
:ref:`addActiveIndex (obs)             <active_list_add_obs_index>`                           Adds several observation indices to the list of active indices
:ref:`addField                         <add_field>`                                           Adds field node to dataset
:ref:`EclGrid, EclInitFile             <load_file>`                                           Loads eclipse file in restart format
:ref:`EclRegion                        <create_eclregion>`                                    Creates a new region for use when defining active regions for fields
:ref:`select_active                    <eclregion_select_all>`                                Selects or deselects cells in a region
:ref:`select_equal                     <eclregion_select_value_equal>`                        Selects or deselects cells in a region equal to given value
:ref:`select_less                      <eclregion_select_value_less>`                         Selects or deselects cells in a region equal less than a given value
:ref:`select_more                      <eclregion_select_value_more>`                         Selects or deselects cells in a region equal greater than a given value
:ref:`select_box                       <eclregion_select_box>`                                Selects or deselects cells in a box
:ref:`select_islice, _jslice,_kslice   <eclregion_select_slice>`                              Selects or deselects cells in a slice
:ref:`select_below_plane               <eclregion_select_plane>`                              Selects or deselects cells in a half space defined by a plane
:ref:`select_inside_polygon            <eclregion_select_in_polygon>`                         Selects or deselects cells in region inside polygon
:ref:`Example create polygon           <create_polygon>`                                      Creates a geo-polygon based on coordinate list
:ref:`Example load polygon             <load_polygon>`                                        Loads polygon in Irap RMS format from file

===========================================================================================   ==============================================================================================================================================


.. ###########################################################################################################

.. _create_updatestep:
.. topic:: getUpdatestep

  | This function will create the default updatestep. 
  | Observe that you must get, otherwise it will not be able to do anything.
  
  *Example:*

  ::
  
    updatestep = local_config.getUpdatestep()

.. ###########################################################################################################

.. _all_obs:
.. topic:: getObservations

  | This function will retrieve ERT's observations
  
  *Example:*

  ::
  
    all_obs = local_config.getObservations()

.. ###########################################################################################################

.. _ert_grid:
.. topic:: getGrid

  | This function will retrieve ERT's grid
  
  *Example:*

  ::
  
    grid = local_config.getGrid()

.. ###########################################################################################################


.. _create_ministep:
.. topic:: createMinistep 

  | This function will create a new ministep with a given name and an optional analysis module. The default analysis module for this ministep is ERT's current analysis module.
  | A given observation set can be attached to a given ministep with attachObsset.The ministep is then ready for adding data. Before the ministep can be used you must attach it to an updatestep with the attachMinistep command 
  
  *Example:*

  ::
  
    ministep = local_config.createMinistep("MINISTEP")

  *Example:*

  ::

    analysis_config = ert.analysisConfig()
    std_enkf_analysis_module = analysis_config.getModule("STD_ENKF")
    ministep_using_std_enkf = local_config.createMinistep("MINISTEP", std_enkf_analysis_module)


.. ###########################################################################################################

.. _create_dataset:
.. topic:: createDataset

  | This function will create a new dataset with a given name, i.e. a collection of enkf_nodes which should be updated together. Before you can actually use a dataset you must attach it to a ministep with the attachDataset command.  
  

  *Example:*

  ::

    dataset_multflt = local_config.createDataset("DATASET_MULTFLT")    

.. ###########################################################################################################

.. _copy_dataset:
.. topic:: copyDataset

  | Will create a new local_obsset instance which is a copy of the
    source dataset; this is a deep copy where also the lowest level active_list instances are copied, and can then subsequently be updated independently of each other.


  *Example:*

  ::

    dataset_multflt_copy = local_config.copyDataset("DATASET_MULTFLT","DATASET_MULTFLT_COPY")   

.. ###########################################################################################################

.. _create_obsset:
.. topic:: createObsdata

  | This function will create an observation set, i.e. a collection of observation keys which will be used as the observations in one ministep. Before the obsset can be used it must be attached to a ministep with the attachDataset command.
  
  
  *Example:*

  ::

    obsset_obs_well = local_config.createObsdata("OBS_WELL")       


.. ###########################################################################################################

.. _copy_obsset:
.. topic:: copyObsdata

  | Will create a new local_obsset instance which is a copy of the
    source dataset; this is a deep copy where also the lowest level active_list instances are copied, and can then subsequently be updated independently of each other.
  

  *Example:*

  ::

    obsset_obs_well_copy = local_config.copyObsdata("OBS_WELL", "OBS_WELL_COPY")

.. ###########################################################################################################

.. _attach_ministep:
.. topic:: attachMinistep

  | This function will attach the ministep to the default updatestep.

  *Example:*

  ::

    update_step.attachMinistep(ministep)       


.. ###########################################################################################################

.. _attach_dataset:
.. topic:: attachDataset

  | Will attach the given dataset to the ministep.


  *Example:*

  ::

    ministep.attachDataset(dataset_multflt)       


.. ###########################################################################################################

.. _attach_obsset:
.. topic:: attachObsset

  | Will attach the given obsset to the ministep.
  
  *Example:*

  ::

    ministep.attachObsset(obsset_obs_well)       


.. ###########################################################################################################

.. _add_data:
.. topic:: addNode

  | This function will add the data KEY as one enkf node which should be updated in this dataset. If you do not manipulate the KEY further with addActiveIndex, the KEY will be added as 'ALL_ACTIVE', i.e. all elements will be updated.
  
  
  *Example:*

  ::

    dataset_multflt.addNode("MULTFLT")

.. ###########################################################################################################

.. _del_data:
.. topic:: del (data)

  | This function will delete the data 'KEY' from the dataset.
  
  
  *Example:*

  ::

    del dataset_multflt["MULTFLT"]


.. ###########################################################################################################

.. _add_obs:
.. topic:: addNode

  | This function will install the observation 'OBS_KEY' as an observation for this obsset - similarly to the addNode function.
  
  *Example:*

  ::
  
    -- The obsset has a time range
    obsset_obs_well.addNodeAndRange("WOPR:OBS_WELL", 0, 1)
    
    -- All times are active
    obsset_obs_well.addNode("WOPR:OBS_WELL")


.. ###########################################################################################################

.. _del_obs:
.. topic:: del (obs)

  | This function will delete the obs 'OBS_KEY' from the obsset 'NAME_OF_OBSSET'.
  
  
  *Example:*

  ::

    del obsset_obs_well["WOPR:OBS_WELL"]


.. ###########################################################################################################

.. _dataset_del_all_data:
.. topic:: clear

  | This function will delete all the data keys from the dataset.
  
  *Example:*

  ::

    dataset_multflt.clear()
    
.. ###########################################################################################################

.. _active_list_add_data_index:
.. topic:: addActiveIndex (data)

  | This function will say that the data with name 'DATA_KEY' in dataset with name 'DATASTEP_NAME' should have the index 'INDEX' active.
  
  
  *Example:*

  ::

    active_list = dataset_multflt.getActiveList("MULTFLT")
    active_list.addActiveIndex(0);

.. ###########################################################################################################

.. _active_list_add_obs_index:
.. topic:: addActiveIndex (obs)

  | This function will say that the observation with name 'OBS_KEY' in obsset with name 'OBSSET_NAME' should have the index 'INDEX' active.
  
  *Example:*

  ::

    active_list = obsset_obs_well.getActiveList("WOPR:OBS_WELL")
    active_list.addActiveIndex(0);


.. ###########################################################################################################

.. _add_field:
.. topic:: addField

  | This function will install the node with name 'FIELD_NAME' in the dataset 'DATASET_NAME'. It will in addition select all the (currently) active cells in the region 'ECLREGION_NAME' as active for this field/ministep combination. The ADD_FIELD command is actually a shortcut of:   ADD_DATA   DATASET  FIELD_NAME; followed by: ACTIVE_LIST_ADD_MANY_DATA_INDEX  <All the indices from the region>
  
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
.. topic:: EclGrid, EclInitFile

  | This function will load an ECLIPSE file in restart format (i.e. restart file or INIT file), the keywords in this file can then subsequently be used in ECLREGION_SELECT_VALUE_XXX commands below. The 'KEY' argument is a string which will be used later when we refer to the content of this file.
  
  *Example:*

  ::
  
    # Load Eclipse grid and init file
    ecl_grid = EclGrid("path/to/FULLMODEL.GRDECL")
    refinit_file = EclInitFile(grid , "path/to/somefile.init")      

.. ###########################################################################################################

.. _create_eclregion:
.. topic:: EclRegion

  | This function will create a new region 'ECLREGION_NAME', which can subsequently be used when defining active regions for fields. The second argument, SELECT_ALL, is a boolean value. If this value is set to true the region will start with all cells selected, if set to false the region will start with no cells selected.
  
  *Example:*

  ::
   
    # Define Eclipse region    
    eclreg_poro = EclRegion(ecl_grid, False)

.. ###########################################################################################################

.. _eclregion_select_all:
.. topic:: select_active

  | Will select all the cells in the region (or deselect if SELECT == FALSE).
  

  *Example:*

  ::
         
    eclreg_poro.select_active()  


.. ###########################################################################################################

.. _eclregion_select_value_equal:
.. topic:: select_equal

  | This function will compare an ecl_kw instance loaded from file with a user supplied value, and select (or deselect) all cells which match this value. It is assumed that the ECLIPSE keyword is an INTEGER keyword, for float comparisons use the ECLREGION_SELECT_VALUE_LESS and ECLREGION_SELECT_VALUE_MORE functions.

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
.. topic:: select_less

  | This function will compare an ecl_kw instance loaded from disc with a numerical value, and select all cells which have numerical below the limiting value. The ecl_kw value should be a floating point value like e.g. PRESSURE or PORO. The arguments are just as for ECLREGION_SELECT_VALUE_EQUAL. 

  *Example:*

  ::
                          
    eclreg_poro.select_less(local_kw, 1) 
        
    
.. ###########################################################################################################

.. _eclregion_select_value_more:
.. topic:: select_more

  | This function will compare an ecl_kw instance loaded from disc with a numerical value, and select all cells which have numerical above the limiting value. The ecl_kw value should be a floating point value like e.g. PRESSURE or PORO. The arguments are just as for ECLREGION_SELECT_VALUE_EQUAL. 
  

  *Example:*

  ::
                          
    eclreg_poro.select_more(local_kw, 1)     
    
.. ###########################################################################################################

.. _eclregion_select_box:
.. topic:: select_box

  | This function will select (or deselect) all the cells in the box defined by the six coordinates i1 i2 j1 j2 k1 k2. The coordinates are inclusive, and the counting starts at 1.   
    

  *Example:*

  ::
                          
    eclreg_poro.select_box((0,2,4),(1,3,5))  
        


.. ###########################################################################################################

.. _eclregion_select_slice:
.. topic:: select_islice, _jslice,_kslice

  | This function will select a slice in the direction given by 'dir', which can 'x', 'y' or 'z'. Depending on the value of 'dir' the numbers n1 and n2 are interpreted as (i1 i2), (j1 j2) or (k1 k2) respectively. The numbers n1 and n2 are inclusice and the counting starts at 1. It is OK to use very high/low values to imply "the rest of the cells" in one direction.
     
  
  *Example:*

  ::
                          
    eclreg_poro.select_kslice(2,3)  


.. ###########################################################################################################

.. _eclregion_select_plane:
.. topic:: select_below_plane

  | Will select all points which have positive (sign > 0) distance to the plane defined by normal vector n = (nx,ny,nz) and point p = (px,py,pz). If sign < 0 all cells with negative distance to plane will be selected.
  
  *Example:*

  ::
     
    eclreg_poro.select_below_plane((1,1,1),(0,0,0))


.. ###########################################################################################################

.. _eclregion_select_in_polygon:
.. topic:: select_inside_polygon

  | Well select all the points which are inside the polygon with name 'POLYGON_NAME'. The polygon should have been created with command CREATE_POLYGON or loaded with command 'LOAD_POLYGON' first.
  
  
  
  *Example:*

  ::
  
    polygon = [(0,0) , (0,1) , (1,0)]
    eclreg_poro.select_inside_polygon(polygon)
    
.. ###########################################################################################################

.. _create_polygon:
.. topic:: Example create polygon

  | Will create a geo_polygon instance based on the coordinate list: (x1,y1), (x2,y2), (x3,y3), ... The polygon should not be explicitly closed - i.e. you should in general have (x1,y1) != (xn,yn). The polygon will be stored under the name 'POLYGON_NAME' - which should later be used when referring to the polygon in region select operations.
  


  *Example:*

  ::
  
    polygon = [(0,0) , (0,1) , (1,0)]    

.. ###########################################################################################################

.. _load_polygon:
.. topic:: Example load polygon

  | Will load a polygon instance from the file 'FILENAME' - the file should be in irap RMS format. The polygon will be stored under the name 'POLYGON_NAME' which can then later be used to refer to the polygon for e.g. select operations.  

    
  *Example:*

  ::
      
    polygon = []
    with open("polygon.ply","r") as fileH:
     for line in fileH.readlines():
       tmp = line.split()
       polygon.append( (float(tmp[0]) , float(tmp[1])))


