.. _ert_kw_full_doc:

Keywords for the configuration file
===================================

:ref:`Go to main ERT page <index>`


General overview
----------------
The enkf application is started with a single argument, which is the name of the configuration file to be used. The enkf configuration file serves several purposes, which are:

* Defining which ECLIPSE model to use, i.e. giving a data, grid and schedule file.
* Defining which observation file to use.
* Defining how to run simulations.
* Defining where to store results.
* Creating a parametrization of the ECLIPSE model. 

The configuration file is a plain text file, with one statement per line. The first word on each line is a keyword, which then is followed by a set of arguments that are unique to the particular keyword. Except for the DEFINE keyword, ordering of the keywords is not significant. Similarly to ECLIPSE data files, lines starting with "--" are treated as comments.

The keywords in the enkf configuration file can roughly be divded into two groups:

* Basic required keywords not related to parametrization. I.e., keywords giving the data, grid, schedule and observation file, defining how to run simulations and how to store results. These keywords are described in :ref:`Basic required keywords.<basic_required_keywords>`
* Basic optional keywords not related to parametrization. These keywords are described in :ref:`Basic optional keywords <basic_optional_keywords>`.
* Keywords related to parametrization of the ECLIPSE model. These keywords are described in :ref:`Parametrization keywords<parameterization_keywords>`.
* Advanced keywords not related to parametrization. These keywords are described in :ref:`Advanced optional keywords<advanced_optional_keywords>`. 


List of keywords
----------------

=====================================================================	======================================	==============================  ==============================================================================================================================================
Keyword name                                                        	Required by user?     			Default value         		Purpose
=====================================================================	======================================	============================== 	==============================================================================================================================================
:ref:`ADD_FIXED_LENGTH_SCHEDULE_KW <add_fixed_length_schedule_kw>`  	NO                                          				Supporting unknown SCHEDULE keywords.
:ref:`ADD_STATIC_KW <add_Static_kw>`                                	NO                                          				Add static ECLIPSE keyword that should be stored
:ref:`ANALYSIS_COPY <analysis_copy>`                                	NO                                          				Create new instance of analysis module
:ref:`ANALYSIS_LOAD <analysis_load>`                                	NO                                          				Load analysis module
:ref:`ANALYSIS_SET_VAR <analysis_set_var>`                          	NO                                          				Set analysis module internal state variable
:ref:`ANALYSIS_SELECT <analysis_select>`                            	NO                    			STD_ENKF    	          	Select analysis module to use in update
:ref:`CASE_TABLE <case_table>`                                      	NO                                          				For running sensitivities you can give the cases descriptive names
:ref:`CONTAINER <container>`                                        	NO                                          				...
:ref:`CUSTOM_KW <custom_kw>`                                        	NO                                          				Ability to load arbitrary values from the forward model.
:ref:`DATA_FILE <data_file>`                                        	YES                                         				Provide an ECLIPSE data file for the problem.
:ref:`DATA_KW <data_kw>`                                            	NO                                          				Replace strings in ECLIPSE .DATA files
:ref:`DBASE_TYPE <dbase_type>`                                      	NO                    			BLOCK_FS         	     	Which 'database' system should be used for storage
:ref:`DEFINE <define>`                                              	NO                                          				Define keywords with config scope
:ref:`DELETE_RUNPATH <delete_runpath>`                              	NO                                          				Explicitly tell ert to delete the runpath when a job is complete 
:ref:`ECLBASE <eclbase>`	                                    	YES					        			Define a name for the ECLIPSE simulations.
:ref:`END_DATE <end_date>`                                          	NO                                          				You can tell ERT how lon the simulations should be - for error check
:ref:`ENKF_ALPHA <enkf_alpha>`                                      	NO                    			1.50                  		Parameter controlling outlier behaviour in EnKF algorithm
:ref:`ENKF_BOOTSTRAP <enkf_bootstrap>`                              	NO                    			FALSE                 		Should we bootstrap the Kalman gain estimate
:ref:`ENKF_CROSS_VALIDATION <enkf_cross_validation>`                	NO                                          	...
:ref:`ENKF_CV_FOLDS <enkf_cv_folds>`                                	NO                    			10                    		Number of folds used in the Cross-Validation scheme
:ref:`ENKF_FORCE_NCOMP <enkf_force_ncomp>`                          	NO                    			FALSE                 		Should we want to use a spesific subspace dimension
:ref:`ENKF_KERNEL_REGRESSION <enkf_kernel_regression>`              	NO                    			FALSE
:ref:`ENKF_KERNEL_FUNCTION <enkf_kernel_function>`                  	NO                    			1
:ref:`ENKF_KERNEL_PARAM <enkf_kernel_param>`                        	NO                    			1
:ref:`ENKF_LOCAL_CV <enkf_local_cv>`                                	NO                    			FALSE                 		Should we estimate the subspace dimenseion using Cross-Validation
:ref:`ENKF_MERGE_OBSERVATIONS <enkf_merge_observations>`            	NO                    			FALSE                 		Should observations from many times be merged together
:ref:`ENKF_MODE <enkf_mode>`                                        	NO                    			STANDARD              		Which EnKF should be used
:ref:`ENKF_NCOMP <enkf_ncomp>`                                       	NO                    			1                     		Dimension of the reduced order subspace (If ENKF_FORCE_NCOMP = TRUE)
:ref:`ENKF_PEN_PRESS <enkf_pen_press>`                              	NO                    			FALSE                 		Should we want to use a penalised PRESS statistic in model selection? 
:ref:`ENKF_RERUN <enkf_rerun>`                                      	NO                    			FALSE                 		Should the simulations be restarted from time zero after each update. 
:ref:`ENKF_SCALING <enkf_scaling>`                                  	NO                    			TRUE           		       	Do we want to normalize the data ensemble to have unit variance? 
:ref:`ENKF_TRUNCATION <enfk_truncation>`                            	NO                    			0.99        	          	Cutoff used on singular value spectrum. 
:ref:`ENSPATH <enspath>`                                            	NO                    			storage     	          	Folder used for storage of simulation results. 
:ref:`EQUIL_INIT_FILE <equil_init_file>`                            	NO                                          				Use INIT_SECTION instead 
:ref:`FIELD <field>`                                                	NO                                          				Ads grid parameters
:ref:`FORWARD_MODEL <forward_model>`                                	NO                                          				Add the running of a job to the simulation forward model. 
:ref:`GEN_DATA <gen_data>`                                          	NO                                          				Specify a general type of data created/updated by the forward model.
:ref:`GEN_KW <gen_kw>`                                              	NO                                          				Add a scalar parameter. 
:ref:`GEN_KW_TAG_FORMAT <gen_kw_tag_format>`                        	NO                    			<%s>                  		Format used to add keys in the GEN_KW template files.
:ref:`GEN_KW_EXPORT_FILE <gen_kw_export_file>`                      	NO                    			parameter.txt         		Name of file to export GEN_KW parameters to. 
:ref:`GEN_PARAM <gen_param>`                                        	NO                                          				Add a general parameter. 
:ref:`GRID <grid>`                                                  	YES                                         				Provide an ECLIPSE grid for the reservoir model. 
:ref:`HISTORY_SOURCE <history_source>`                              	NO                    			REFCASE_HISTORY     	  	Source used for historical values.
:ref:`HOOK_WORKFLOW <hook_workflow>` 					NO 									Install a workflow to be run automatically.
:ref:`HOST_TYPE <host_type>`                                        	NO                                          
:ref:`IGNORE_SCHEDULE <ignore_schedule>`                            	NO                                          
:ref:`IMAGE_TYPE <image_type>`                                      	NO                    			png                   		The type of the images created when plotting.
:ref:`IMAGE_VIEWER <image_viewer>`                                  	NO                    			/usr/bin/display      		External program spawned to view images.
:ref:`INIT_SECTION <init_section>`                                  	YES                                         				Initialization code for the reservoir model.
:ref:`INSTALL_JOB <install_jobb>`                                   	NO                                          				Install a job for use in a forward model. 
:ref:`ITER_CASE <iter_Case>`                                        	NO                    			IES%d         	        	Case name format - iterated ensemble smoother
:ref:`ITER_COUNT <iter_count>`                                      	NO                    			4             	        	Number of iterations - iterated ensemble smoother 
:ref:`ITER_RETRY_COUNT <iter_retry_count>`                          	NO                    			4         	            	Number of retries for a iteration - iterated ensemble smoother 
:ref:`JOBNAME <jobname>`                                            	NO                                          				Name used for simulation files. An alternative to ECLBASE. 
:ref:`JOB_SCRIPT <job_script>`                                      	NO                                          				Python script managing the forward model. 
:ref:`LOAD_SEED <load_seed>`                                        	NO                                          				Load random seed from given file.
:ref:`LOAD_WORKFLOW <load_workflow>` 				    	NO                             						Load a workflow into ERT. 
:ref:`LOAD_WORKFLOW_JOB <load_workflow_job>`  			    	NO 									Load a workflow job into ERT. 
:ref:`LICENSE_PATH <licence_path>`  				    	NO 									A path where ert-licenses to e.g. RMS are stored. 
:ref:`LOCAL_CONFIG <load_config>` 			            	NO 									A file with configuration information for local analysis. 
:ref:`LOG_FILE <log_file>` 					    	NO 					log 				Name of log file 
:ref:`LOG_LEVEL <log_level>` 					    	NO 		 			1 				How much logging? 
:ref:`LSF_QUEUE <lsf_queue>` 					    	NO 					normal				Name of LSF queue. 
:ref:`LSF_RESOURCES <lsf_resources>` 				    	NO 
:ref:`LSF_SERVER <lsf_server>` 					    	NO 									Set server used when submitting LSF jobs. 
:ref:`MAX_ITER_COUNT <max_iter_count>` 				    	NO 									Maximal number of iterations - iterated ensemble smoother. 
:ref:`MAX_RESAMPLE <max_resample>`				    	NO 					1		 		How many times should ert resample & retry a simulation.
:ref:`MAX_RUNNING_LOCAL <max_running_local>` 				NO 									The maximum number of running jobs when running locally. 
:ref:`MAX_RUNNING_LSF <max_running_lsf>` 				NO 									The maximum number of simultaneous jobs submitted to LSF. 
:ref:`MAX_RUNNING_RSH <max_running_rsh>` 				NO 									The maximum number of running jobs when using RSH queue system. 
:ref:`MAX_RUNTIME <max_runtime>` 					NO 					0 				Set the maximum runtime in seconds for a realization. 
:ref:`MAX_SUBMIT <max_submit>` 						NO 					2 				How many times should the queue system retry a simulation. 
:ref:`MIN_REALIZATIONS <min_realizations>` 				NO 					0 				Set the number of minimum reservoir realizations to run before long running realizations are stopped. Keyword STOP_LONG_RUNNING must be set to TRUE when MIN_REALIZATIONS are set. 
:ref:`NUM_REALIZATIONS <num_realizations>` 				YES 									Set the number of reservoir realizations to use. 
:ref:`OBS_CONFIG <obs_config>` 						NO 									File specifying observations with uncertainties. 
:ref:`PLOT_DRIVER <plot_driver>` 					NO 					PLPLOT 				Which plotting system should be used.
:ref:`PLOT_ERRORBAR <plot_errorbar>` 					NO 					FALSE 				Should errorbars on observations be plotted? 
:ref:`PLOT_ERRORBAR_MAX <plot_errorbar_max>` 				NO 					25 				Show error bars if less than this number of observations. 
:ref:`PLOT_HEIGHT <plot_height>` 					NO 					768 				Pixel height of the plots. 
:ref:`PLOT_PATH  <plot_path>`						NO 					plots 				Path to where the plots are stored. 
:ref:`PLOT_REFCASE <plot_refcase>` 					NO 					TRUE 				TRUE (IF you want to plot the listed reference cases) FALSE if not. 
:ref:`PLOT_REFCASE_LIST <plot_refcase_list>` 				NO 									Deprecated. Use REFCASE_LIST instead. 
:ref:`PLOT_WIDTH <plot_width>` 						NO 					1024 				Pixel width of the plots. 
:ref:`PRE_CLEAR_RUNPATH <pre_clear_runpath>` 				NO 					FALSE 				Should the runpath be cleared before initializing? 
:ref:`QC_PATH <qc_path>` 						NO 					QC 				... 
:ref:`QUEUE_SYSTEM <queue_system>` 					NO 									System used for running simulation jobs. 
:ref:`REFCASE <refcase>` 						NO (see HISTORY_SOURCE and SUMMARY) 					Reference case used for observations and plotting. 
:ref:`REFCASE_LIST <refcase_list>` 					NO 									Full path to Eclipse .DATA files containing completed runs (which you can add to plots) 
:ref:`RERUN_PATH  <rerun_path>` 					NO 									...
:ref:`RERUN_START  <rerun_start>` 					NO 					0 				... 
:ref:`RFT_CONFIG  <rft_config>` 					NO 									Config file specifying wellnames and dates for rft-measurments. Used for plotting. The format has to be name day month year (ex. Q-2FI 02 08 1973), with a new entry on a new line. 
:ref:`RFTPATH <rftpath>`  						NO 					rft 				Path to where the rft well observations are stored 
:ref:`RSH_COMMAND  <rsh_command>` 					NO 									Command used for remote shell operations. 
:ref:`RSH_HOST <rsh_host>`  						NO 									Remote host used to run forward model. 
:ref:`RUNPATH <runoath>`  						NO 					simulations/realization%d 	Directory to run simulations
:ref:`RUN_TEMPLATE <run_template>`  					NO 									Install arbitrary files in the runpath directory.
:ref:`STD_SCALE_CORRELATED_OBS <std_scale_correlated_obs>`              NO                                      FALSE                           Try to estimate the correlations in the data to inflate the observation std.     
:ref:`SCHEDULE_FILE <schedule_file>`  					YES 									Provide an ECLIPSE schedule file for the problem. 
:ref:`SCHEDULE_PREDICTION_FILE <schedule_prediction_file>`  		NO 									Schedule prediction file. 
:ref:`SETENV <setenv>`  						NO 									You can modify the UNIX environment with SETENV calls. 
:ref:`SINGLE_NODE_UPDATE <single_node_update>`  			NO 					FALSE 				... 
:ref:`STD_CUTOFF <std_cutoff>`  					NO 					1e-6 				... 
:ref:`STOP_LONG_RUNNING <stop_long_running>`  				NO 					FALSE 				Stop long running realizations after minimum number of realizations (MIN_REALIZATIONS) have run. 
:ref:`STORE_SEED  <store_seed>` 					NO 									File where the random seed used is stored. 
:ref:`SUMMARY  <summary>` 						NO 									Add summary variables for internalization. 
:ref:`SURFACE <surface>`  						NO 									Surface parameter read from RMS IRAP file. 
:ref:`TORQUE_QUEUE  <torque_queue>` 					NO 									... 
:ref:`TIME_MAP  <time_map>`       					NO 									Ability to manually enter a list of dates to establish report step <-> dates mapping.
:ref:`UMASK <umask>`  							NO 									... 
:ref:`UPDATE_LOG_PATH  <update_log_path>` 				NO 					update_log 			Summary of the EnKF update steps are stored in this directory. 
:ref:`UPDATE_PATH  <update_path>` 					NO 									Modify a UNIX path variable like LD_LIBRARY_PATH. 
:ref:`UPDATE_RESULTS  <update_results>` 				NO 					FALSE 				... 
:ref:`WORKFLOW_JOB_DIRECTORY  <workflow_job_directory>` 		NO 									Directory containing workflow jobs. 
=====================================================================	======================================	============================== 	==============================================================================================================================================


:ref:` <>`

Basic required keywords
-----------------------
.. _basic_required_keywords:

These keywords must be set to make the enkf function properly.

.. _data_file:
.. topic:: DATA_FILE

	| This is the name of ECLIPSE data file used to control the simulations. The data file should be prepared according to the guidelines given in Preparing an ECLIPSE reservoir model for use with enkf. 
	
	*Example:*

	::

		-- Load the data file called ECLIPSE.DATA
		DATA_FILE ECLIPSE.DATA




.. _eclbase:
.. topic:: ECLBASE

	| The ECLBASE keyword sets the basename used for the ECLIPSE simulations. It can (and should, for your convenience) contain a %d specifier, which will be replaced with the realization numbers when running ECLIPSE. Note that due to limitations in ECLIPSE, the ECLBASE string must be in strictly upper or lower case.

	*Example:*

	::
	
		-- Use MY_VERY_OWN_OIL_FIELD-0 etc. as basename.
		-- When ECLIPSE is running, the %d will be,
		-- replaced with realization number, giving:
		-- 
		-- MY_VERY_OWN_OIL_FIELD-0
		-- MY_VERY_OWN_OIL_FIELD-1
		-- MY_VERY_OWN_OIL_FIELD-2
		-- ...
		-- and so on.  
		ECLBASE MY_VERY_OWN_OIL_FIELD-%d

.. _jobname:
.. topic::  JOBNAME

	As an alternative to the ECLBASE keyword you can use the JOBNAME keyword; in particular in cases where your forward model does not include ECLIPSE at all that makes more sense. If JOBANME is used instead of ECLBASE the same rules of no-mixed-case apply. 

.. _grid:
.. topic:: GRID

	This is the name of an existing GRID/EGRID file for your ECLIPSE model. If you had to create a new grid file when preparing your ECLIPSE reservoir model for use with enkf, this should point to the new .EGRID file.

	*Example:*

	::
	
		-- Load the .EGRID file called MY_GRID.EGRID
  		GRID MY_GRID.EGRID


.. _init_section:
.. topic:: INIT_SECTION

	The INIT_SECTION keyword is used to handle initialization of the ECLIPSE run. See the documentation of the Initialization for more details on why this has to be done. The keyword can be used in two different ways:

	* If it is set to the name of an existing file, the contents of this file will be used for the initialization.
	* If it is set to the name of a non-existing file, it will be assumed that a file with this name in the simulation folder will be generated when simulations are submitted, either by the enkf application itself, or by some job installed by the user (see INSTALL_JOB). This generated file will then be used by ECLIPSE for initialization. 

	*Example A:*

	::
  	
		-- Use the contents of the file parameters/EQUIL.INC for initialization
  		INIT_SECTION params/EQUIL.INC

	*Example B:*

	::

	  	-- Use a generated file for the initialization
	  	INIT_SECTION MY_GENERATED_EQUIL_KEYWORD.INC


.. _num_realizations:
.. topic:: NUM_REALIZATIONS

	This is just the size of the ensemble, i.e. the number of realizations/members in the ensemble.

	*Example:*

	::

		-- Use 200 realizations/members
		NUM_REALIZATIONS 200


.. _schedule_file:
.. topic:: SCHEDULE_FILE

	This keyword should be the name a text file containing the SCHEDULE section of the ECLIPSE data file. It should be prepared in accordance with the guidelines given in Preparing an ECLIPSE reservoir model for use with enkf. This SCHEDULE section will be used to control the ECLIPSE simulations. You can optionally give a second filename, which is the name of file which will be written into the directories for running ECLIPSE.

	*Example:*

	::

		-- Parse MY_SCHEDULE.SCH, call the generated file ECLIPSE_SCHEDULE.SCH
		SCHEDULE_FILE MY_SCHEDULE.SCH ECLIPSE_SCHEDULE.SCH 

	Observe that the SCHEDULE_FILE keyword is only required when you need ERT to stop and restart your simulations; i.e. when you are using the EnKF algorithm. If you are only using ERT to your simulations; or using smoother update it is recommended to leave the SCHEDULE_FILE keyword out. In that case you must make sure that the ECLIPSE datafile correctly includes the SCHEDULE section. 


Basic optional keywords
-----------------------
.. _basic_optional_keywords:

These keywords are optional. However, they serve many useful purposes, and it is recommended that you read through this section to get a thorough idea of what's possible to do with the enkf application.

.. _data_kw:
.. topic:: DATA_KW

	The keyword DATA_KW can be used for inserting strings into placeholders in the ECLIPSE data file. For instance, it can be used to insert include paths.

	*Example:*

	::

		-- Define the alias MY_PATH using DATA_KW. Any instances of <MY_PATH> (yes, with brackets)
		-- in the ECLIPSE data file will now be replaced with /mnt/my_own_disk/my_reservoir_model
		-- when running the ECLIPSE jobs.
		DATA_KW  MY_PATH  /mnt/my_own_disk/my_reservoir_model

	The DATA_KW keyword is of course optional. Note also that the enkf has some built in magic strings.

.. _delete_runpath:
.. topic:: DELETE_RUNPATH

	When the ert application is running it creates directories for
	the forward model simulations, one for each realization. When
	the simulations are done, ert will load the results into the
	internal database. By default the realization folders will be
	left intact after ert has loaded the results, but using the
	keyword DELETE_RUNPATH you can request to have (some of) the
	directories deleted after results have been loaded.

	*Example A:*

	::

		-- Delete simulation directories 0 to 99
		DELETE_RUNPATH 0-99

	*Example B:*

	::

		-- Delete simulation directories 0 to 10 as well as 12, 15 and 20.
		DELETE_RUNPATH 0 - 10, 12, 15, 20

	The DELETE_RUNPATH keyword is optional.


.. _end_date:
.. topic:: END_DATE

	When running a set of models from beginning to end ERT does
	not now in advance how long the simulation is supposed to be,
	it is therefor impossible beforehand to determine which
	restart file number should be used as target file, and the
	procedure used for EnKF runs can not be used to verify that an
	ECLIPSE simulation has run to the end.

	By using the END_DATE keyword you can tell ERT that the
	simulation should go at least up to the date given by
	END_DATE, otherwise they will be regarded as failed. The
	END_DATE does not need to correspond exactly to the end date
	of the simulation, it must just be set so that all simulations
	which go to or beyond END_DATE are regarded as successfull.

	*Example:*

	::
		END_DATE  10/10/2010

	With this END_DATE setting all simulations which have gone to
	at least 10.th of October 2010 are OK.


.. _enspath:
.. topic:: ENSPATH

	The ENSPATH should give the name of a folder that will be used
	for storage by the enkf application. Note that the contents of
	this folder is not intended for human inspection. By default,
	ENSPATH is set to "storage".

	*Example:*

	::

		-- Use internal storage in /mnt/my_big_enkf_disk
		ENSPATH /mnt/my_big_enkf_disk

	The ENSPATH keyword is optional.


.. _history_source:
.. topic:: HISTORY_SOURCE

	In the observation configuration file you can enter
	observations with the keyword HISTORY_OBSERVATION; this means
	that ERT will the observed 'true' values from the model
	history. Practically the historical values can be fetched
	either from the SCHEDULE file or from a reference case. What
	source to use for the historical values can be controlled with
	the HISTORY_SOURCE keyword. The different possible values for
	the HISTORY_SOURCE keyword are:

	
	REFCASE_HISTORY 
	        This is the default value for HISTORY_SOURCE,
		ERT will fetch the historical values from the *xxxH*
		keywords in the refcase summary, e.g. observations of
		WGOR:OP_1 is based the WGORH:OP_1 vector from the
		refcase summary.

	REFCASE_SIMULATED
		In this case the historical values are based on the simulated values from the refcase, this is mostly relevant when a you want compare with another case which serves as 'the truth'. 

	SCHEDULE
		Load historical values from the WCONHIST and WCONINJE keywords in the Schedule file. 


	When setting HISTORY_SOURCE to either REFCASE_SIMULATED or REFCASE_HISTORY you must also set the REFCASE variable to point to the ECLIPSE data file in an existing reference case (should be created with the same schedule file as you are using now).

	*Example:*

	::

		-- Use historic data from reference case
		HISTORY_SOURCE  REFCASE_HISTORY
		REFCASE         /somefolder/ECLIPSE.DATA

	The HISTORY_SOURCE keyword is optional.

.. _refcase:
.. topic:: REFCASE

	With the REFCASE key you can supply ert with a reference case which can be used for observations (see HISTORY_SOURCE), if you want to use wildcards with the SUMMARY keyword you also must supply a REFCASE keyword. The REFCASE keyword should just point to an existing ECLIPSE data file; ert will then look up and load the corresponding summary results.

	*Example:*

	::

		-- The REFCASE keyword points to the datafile of an existing ECLIPSE simulation. 
		REFCASE /path/to/somewhere/SIM_01_BASE.DATA


.. _install_job:
.. topic:: INSTALL_JOB

	The INSTALL_JOB keyword is used to learn the enkf application how to run external applications and scripts, i.e. defining a job. After a job has been defined with INSTALL_JOB, it can be used with the FORWARD_MODEL keyword. For example, if you have a script which generates relative permeability curves from a set of parameters, it can be added as a job, allowing you to do history matching and sensitivity analysis on the parameters defining the relative permeability curves.

	The INSTALL_JOB keyword takes two arguments, a job name and the name of a configuration file for that particular job.

	*Example:*

	::

		-- Define a Lomeland relative permeabilty job.
		-- The file jobs/lomeland.txt contains a detailed
		-- specification of the job.
		INSTALL_JOB LOMELAND jobs/lomeland.txt

	The configuration file used to specify an external job is easy to use and very flexible. It is documented in Customizing the simulation workflow in enkf.

	The INSTALL_JOB keyword is optional.

.. _obs_config:
.. topic:: OBS_CONFIG

	The OBS_CONFIG key should point to a file defining observations and associated uncertainties. The file should be in plain text and formatted according to the guidelines given in Creating an observation file for use with enkf.

	*Example:*

	::

		-- Use the observations in my_observations.txt
		OBS_CONFIG my_observations.txt

	The OBS_CONFIG keyword is optional, but for your own convenience, it is strongly recommended to provide an observation file.

.. _result_path:
.. topic:: RESULT_PATH

	The enkf application will print some simple tabulated results at each report step. The RESULT_PATH keyword should point to a folder where the tabulated results are to be written. It can contain a %d specifier, which will be replaced with the report step by enkf. The default value for RESULT_PATH is "results/step_%d".

	*Example:*

	::

		-- Changing RESULT_PATH
		RESULT_PATH my_nice_results/step-%d

	The RESULT_PATH keyword is optional.

.. _runpath:
.. topic:: RUNPATH

	The RUNPATH keyword should give the name of the folders where the ECLIPSE simulations are executed. It should contain at least one %d specifier, which will be replaced by the realization number when the enkf creates the folders. Optionally, it can contain one more %d specifier, which will be replaced by the iteration number.

	By default, RUNPATH is set to "simulations/realization-%d".

	*Example A:*

	::
		-- Giving a RUNPATH with just one %d specifer.
		RUNPATH /mnt/my_scratch_disk/realization-%d

	*Example B:*

	::

		-- Giving a RUNPATH with two %d specifers.
		RUNPATH /mnt/my_scratch_disk/realization-%d/iteration-%d

	The RUNPATH keyword is optional.


.. _runpath_file:
.. topic:: RUNPATH_FILE

When running workflows based on external scripts it is necessary to 'tell' the external script in some way or another were all the realisations are located in the filesystem. Since the number of realisations can be quite high this will easily overflow the commandline buffer; the solution which is used is therefor to let ert write a reagular file which looks like this::

  0   /path/to/realisation0   CASE0   iter
  1   /path/to/realisation1   CASE1   iter
  ...
  N   /path/to/realisationN   CASEN   iter

The path to this file can then be passed to the scripts using the
magic string <RUNPATH_FILE>. The RUNPATH_FILE will by default be
stored as .ert_runpath_list in the same directory as the configuration
file, but you can set it to something else with the RUNPATH_FILE key.

Keywords controlling the simulations
------------------------------------
.. _keywords_controlling_the_simulations:

.. _min_realizations:
.. topic:: MIN_REALIZATIONS

	MIN_REALIZATIONS is the minimum number of realizations that must have succeeded for the simulation to be regarded as a success.

	MIN_REALIZATIONS can also be used in combination with STOP_LONG_RUNNING, see the documentation for STOP_LONG_RUNNING for a description of this.

	*Example:*
	
	::

		MIN_REALIZATIONS  20

	The MIN_REALIZATIONS key can also be set as a percentage of NUM_REALIZATIONS

	::

		MIN_REALIZATIONS  10%

	The MIN_REALIZATIONS key is optional.


.. _stop_long_running:
.. topic:: STOP_LONG_RUNNING

	The STOP_LONG_RUNNING key is used in combination with the MIN_REALIZATIONS key to control the runtime of simulations. When STOP_LONG_RUNNING is set to TRUE, MIN_REALIZATIONS is the minimum number of realizations run before the simulation is stopped. After MIN_REALIZATIONS have succeded successfully, the realizatons left are allowed to run for 25% of the average runtime for successfull realizations, and then killed.

	*Example:*

	::

		-- Stop long running realizations after 20 realizations have succeeded
		MIN_REALIZATIONS  20
		STOP_LONG_RUNNING TRUE

	The STOP_LONG_RUNNING key is optional. The MIN_REALIZATIONS key must be set when STOP_LONG_RUNNING is set to TRUE.


.. _max_runtime:
.. topic:: MAX_RUNTIME

	The MAX_RUNTIME keyword is used to control the runtime of simulations. When MAX_RUNTIME is set, a job is only allowed to run for MAX_RUNTIME, given in seconds. A value of 0 means unlimited runtime.

	*Example:*

	::

		-- Let each realizations run for 50 seconds
		MAX_RUNTIME 50

	The MAX_RUNTIME key is optional. 


Parameterization keywords
-------------------------
.. _parameterization_keywords:

The keywords in this section are used to define a parametrization of the ECLIPSE model. I.e., defining which parameters to change in a sensitivity analysis and/or history matching project. For some parameters, it necessary to specify a prior distribution. See Prior distributions available in enkf for a complete list of available priors.

.. _field:
.. topic:: FIELD

	The FIELD keyword is used to parametrize quantities which have extent over the full grid. Both dynamic properties like pressure, and static properties like porosity, are implemented in terms of FIELD objects. When adding fields in the config file the syntax is a bit different for dynamic fields (typically solution data from ECLIPSE) and parameter fields like permeability and porosity.

	**Dynamic fields**

	To add a dynamic field the entry in the configuration file looks like this:

	::
		FIELD   <ID>   DYNAMIC  MIN:X  MAX:Y

	In this case ID is not an arbitrary string; it must coincide with the keyword name found in the ECLIPSE restart file, e.g. PRESSURE. Optionally, you can add a minimum and/or a maximum value with MIN:X and MAX:Y.

	*Example A:*

	::

		-- Adding pressure field (unbounded)
		FIELD PRESSURE DYNAMIC

	*Example B:*

	::

		-- Adding a bounded water saturation field
		FIELD SWAT DYNAMIC MIN:0.2 MAX:0.95

	**Parameter fields**

	A parameter field (e.g. porosity or permeability) is defined as follows:

	::

		FIELD  ID PARAMETER   <ECLIPSE_FILE>  INIT_FILES:/path/%d  MIN:X MAX:Y OUTPUT_TRANSFORM:FUNC INIT_TRANSFORM:FUNC  

	Here ID is again an arbitrary string, ECLIPSE_FILE is the name of the file the enkf will export this field to when running simulations. Note that there should be an IMPORT statement in the ECLIPSE data file corresponding to the name given with ECLIPSE_FILE. INIT_FILES is a filename (with an embedded %d) to load the initial field from. Can be RMS ROFF format, ECLIPSE restart format or ECLIPSE GRDECL format.

	The options MIN, MAX, INIT_TRANSFORM and OUTPUT_TRANSFORM are all optional. MIN and MAX are as for dynamic fields. OUTPUT_TRANSFORM is the name of a mathematical function which will be applied to the field before it is exported, and INIT_TRANSFORM is the name of a function which will be applied to the fields when they are loaded. [Just use INIT_TRANSFORM:XXX to get a list of available functions.]

	Regarding format of ECLIPSE_FILE: The default format for the parameter fields is binary format of the same type as used in the ECLIPSE restart files. This requires that the ECLIPSE datafile contains an IMPORT statement. The advantage with using a binary format is that the files are smaller, and reading/writing is faster than for plain text files. If you give the ECLIPSE_FILE with the extension .grdecl (arbitrary case), enkf will produce ordinary .grdecl files, which are loaded with an INCLUDE statement. This is probably what most users are used to beforehand - but we recomend the IMPORT form.

	**General fields**

	In addition to dynamic and parameter field there is also a general field, where you have fine grained control over input/output. Use of the general field type is only relevant for advanced features. The arguments for the general field type are as follows:

	::

		FIELD   ID  GENERAL    FILE_GENERATED_BY_ENKF  FILE_LOADED_BY_ENKF    <OPTIONS>

	The OPTIONS argument is the same as for the parameter field.

.. _gen_data:
.. topic:: GEN_DATA

	The GEN_DATA keyword is used when estimating data types which enkf does not know anything about. GEN_DATA is very similar to GEN_PARAM, but GEN_DATA is used for data which are updated/created by the forward model like e.g. seismic data. In the main configuration file the input for a GEN_DATA instance is as follows:

	::

		GEN_DATA  ID RESULT_FILE:yyy INPUT_FORMAT:xx  REPORT_STEPS:10,20  ECL_FILE:xxx  OUTPUT_FORMAT:xx  INIT_FILES:/path/files%d TEMPLATE:/template_file TEMPLATE_KEY:magic_string 

	The GEN_DATA keyword has many options; in many cases you can leave many of them off. We therefor list the required and the optional options separately:
	
	**Required GEN_DATA options**

	* RESULT_FILE - This if the name the file generated by the forward model and read by ERT. This filename _must_ have a %d as part of the name, that %d will be replaced by report step when loading.
	* INPUT_FORMAT - The format of the file written by the forward model (i.e. RESULT_FILE) and read by ERT, valid values are ASCII, BINARY_DOUBLE and BINARY_FLOAT.
	* REPORT_STEPS A list of the report step(s) where you expect the forward model to create a result file. I.e. if the forward model should create a result file for report steps 50 and 100 this setting should be: REPORT_STEPS:50,100. If you have observations of this GEN_DATA data the RESTART setting of the corresponding GENERAL_OBSERVATION must match one of the values given by REPORT_STEPS. 

	**Optional GEN_DATA options**

	* ECL_FILE - This is the name of file written by enkf to be read by the forward model.
	* OUTPUT_FORMAT - The format of the files written by enkf and read by the forward model, valid values are ASCII, BINARY_DOUBLE, BINARY_FLOAT and ASCII_TEMPLATE. If you use ASCII_TEMPLATE you must also supply values for TEMPLATE and TEMPLATE_KEY.
	* INIT_FILES - Format string with '%d' of files to load the initial data from. 

	*Example:*

	::

		GEN_DATA 4DWOC  INPUT_FORMAT:ASCII   RESULT_FILE:SimulatedWOC%d.txt   REPORT_STEPS:10,100

	Here we introduce a GEN_DATA instance with name 4DWOC. When the forward model has run it should create two files with name SimulatedWOC10.txt and SimulatedWOC100.txt. The result files are in ASCII format, ERT will look for these files and load the content. The files should be pure numbers - without any header.

	**Observe that the GEN_DATA RESULT_FILE setting must have a %d format specifier, that will be replaced with the report step..**


.. _custom_kw:
.. topic:: CUSTOM_KW

           The keyword CUSTOM_KW enables custom data key:value pairs
           to be stored in ERT storage.  Custom KW has many
           similarities to Gen KW and Gen Data but is fully defined by
           the user and contain only key_value pairs.

           *Example:*

           ::

              CUSTOM_KW GROUP_NAME <input_file>

              --GROUP_NAME
              This is similar to Gen KW where every keyword is prefixed with the GROUP_NAME like this: GROUP_NAME:KEYWORD

              --input_file
              This is the input file expected to be generated by a forward model.

              --Example
              CUSTOM_KW COMPOSITION composition.txt

           With this setup ERT will expect the file composition.txt to be present in the runpath.
           This file may look like this

           ::

              oil 0.5
              water 0.2
              gas 0.2
              unknown 0.1
              state good

           Every key-value pair must be a string followed by a space and a value.
           The value can either be a number or a string (all numbers are interpreted as floats).

           After a successful run, ERT will store the COMPOSITION
           Custom KW in its filesystem and will be available for every
           realization.  An export will present the values produced as:

           * COMPOSITION:oil
           * COMPOSITION:water
           * COMPOSITION:gas
           * COMPOSITION:unknown
           * COMPOSITION:state


.. _gen_kw:
.. topic:: GEN_KW

	The GEN_KW (abbreviation of general keyword) parameter is based on a template file and substitution. In the main config file a GEN_KW instance is defined as follows:

	::

		GEN_KW  ID  my_template.txt  my_eclipse_include.txt  my_priors.txt

	Here ID is an (arbitrary) unique string, my_template.txt is
	the name of a template file, my_eclipse_include.txt is the
	name of the file which is made for each member based on
	my_template.txt and my_priors.txt is a file containing a list
	of parametrized keywords and a prior distribution for
	each. Note that you must manually edit the ECLIPSE data file
	so that my_eclipse_include.txt is included.

	Let us consider an example where the GEN_KW parameter type is
	used to estimate pore volume multipliers. We would then
	declare a GEN_KW instance in the main enkf configuration file:

	::

		GEN_KW PAR_MULTPV multpv_template.txt multpv.txt multpv_priors.txt

	In the GRID or EDIT section of the ECLIPSE data file, we would
	insert the following include statement:

	::

		INCLUDE
		 'multpv.txt' /

	The template file multpv_template.txt would contain some
	parametrized ECLIPSE statements:

	::

		BOX
		 1 10 1 30 13 13 /
		MULTPV
		 300*<MULTPV_BOX1> /
		ENDBOX
	
		BOX
		 1 10 1 30 14 14 /
		MULTPV
		 300*<MULTPV_BOX2> /
		ENDBOX

	Here, <MULTPV_BOX1> and <MULTPV_BOX2> will act as magic
	strings. Note that the '<' '>' must be present around the
	magic strings. In this case, the parameter configuration file
	multpv_priors.txt could look like this:

	::

		MULTPV_BOX2 UNIFORM 0.98 1.03
		MULTPV_BOX1 UNIFORM 0.85 1.00

	In general, the first keyword on each line in the parameter
	configuration file defines a key, which when found in the
	template file enclosed in '<' and '>', is replaced with a
	value. The rest of the line defines a prior distribution for
	the key. See Prior distributions available in enkf for a list
	of available prior distributions.
	
	**Example: Using GEN_KW to estimate fault transmissibility multipliers**

	Previously enkf supported a datatype MULTFLT for estimating
	fault transmissibility multipliers. This has now been
	depreceated, as the functionality can be easily achieved with
	the help of GEN_KW. In th enkf config file:

	::

		GEN_KW  MY-FAULTS   MULTFLT.tmpl   MULTFLT.INC   MULTFLT.txt

	Here MY-FAULTS is the (arbitrary) key assigned to the fault
	multiplers, MULTFLT.tmpl is the template file, which can look
	like this:

	::

		MULTFLT
		 'FAULT1'   <FAULT1>  /
		 'FAULT2'   <FAULT2>  /
		/

	and finally the initial distribution of the parameters FAULT1
	and FAULT2 are defined in the file MULTFLT.txt:

	::

		FAULT1   LOGUNIF   0.00001   0.1
		FAULT2   UNIFORM   0.00      1.0

        The various prior distributions available for the ``GEN_KW``
        keyword are here :ref:`prior distributions available in ERT <prior_distributions>`

                
	Loading GEN_KW values from an external file

	The default use of the GEN_KW keyword is to let the ERT
	application sample random values for the elements in the
	GEN_KW instance, but it is also possible to tell ERT to load a
	precreated set of data files, this can for instance be used as
	a component in a experimental design based workflow. When
	using external files to initialize the GEN_KW instances you
	supply an extra keyword ``INIT_FILE:/path/to/priors/files%d``
	which tells where the prior files are:

	::

		GEN_KW  MY-FAULTS   MULTFLT.tmpl   MULTFLT.INC   MULTFLT.txt    INIT_FILES:priors/multflt/faults%d

	In the example above you must prepare files
	priors/multflt/faults0, priors/multflt/faults1,
	... priors/multflt/faultsn which ert will load when you
	initialize the case. The format of the GEN_KW input files can
	be of two varieties:

	1. The files can be plain ASCII text files with a list of numbers:

	::

		1.25
		2.67

	The numbers will be assigned to parameters in the order found in the MULTFLT.txt file.
	
	2. Alternatively values and keywords can be interleaved as in:

	::

		FAULT1 1.25
		FAULT2 2.56

	in this case the ordering can differ in the init files and the parameter file.
	
	The heritage of the ERT program is based on the EnKF algorithm, and the EnKF algorithm evolves around Gaussian variables - internally the GEN_KW variables are assumed to be samples from the N(0,1) distribution, and the distributions specified in the parameters file are based on transformations starting with a N(0,1) distributed variable. The slightly awkward consequence of this is that to let your sampled values pass through ERT unmodified you must configure the distribution NORMAL 0 1 in the parameter file; alternatively if you do not intend to update the GEN_KW variable you can use the distribution RAW.


.. _gen_param:
.. topic:: GEN_PARAM

	The GEN_PARAM parameter type is used to estimate parameters which do not really fit into any of the other categories. As an example, consider the following situation:

	Some external Software (e.g. Cohiba) makes a large vector of random numbers which will serve as input to the forward model. (It is no requirement that the parameter set is large, but if it only consists of a few parameters the GEN_KW type will be easier to use.)
	We want to update this parameter with enkf.
	In the main configuration file the input for a GEN_PARAM instance is as follows:

	::

		GEN_PARAM  ID  ECLIPSE_FILE  INPUT_FORMAT:xx  OUTPUT_FORMAT:xx  INIT_FILES:/path/to/init/files%d (TEMPLATE:/template_file KEY:magic_string)   

	here ID is the usual unique string identifying this instance and ECLIPSE_FILE is the name of the file which is written into the run directories. The three arguments GEN_PARAM, ID and ECLIPSE_FILE must be the three first arguments. In addition you must have three additional arguments, INPUT_FORMAT, OUTPUT_FORMAT and INIT_FILES. INPUT_FORMAT is the format of the files enkf should load to initialize, and OUTPUT_FORMAT is the format of the files enkf writes for the forward model. The valid values are:

	* ASCII - This is just text file with formatted numbers.
	* ASCII_TEMPLATE - An plain text file with formatted numbers, and an arbitrary header/footer.
	* BINARY_FLOAT - A vector of binary float numbers.
	* BINARY_DOUBLE - A vector of binary double numbers. 

	Regarding the different formats - observe the following:

	#. Except the format ASCII_TEMPLATE the files contain no header information.
	#. The format ASCII_TEMPLATE can only be used as output format.
	#. If you use the output format ASCII_TEMPLATE you must also supply a TEMPLATE:X and KEY:Y option. See documentation of this below.
	#. For the binary formats files generated by Fortran can not be used - can easily be supported on request.

	**Regarding templates:** If you use OUTPUT_FORMAT:ASCII_TEMPLATE you must also supply the arguments TEMPLATE:/template/file and KEY:MaGiCKEY. The template file is an arbitrary existing text file, and KEY is a magic string found in this file. When enkf is running the magic string is replaced with parameter data when the ECLIPSE_FILE is written to the directory where the simulation is run from. Consider for example the follwing configuration:

	::

		TEMPLATE:/some/file   KEY:Magic123

	The template file can look like this (only the Magic123 is special):

	::

		Header line1
		Header line2
		============
		Magic123
		============
		Footer line1
		Footer line2

	When enkf is running the string Magic123 is replaced with parameter values, and the resulting file will look like this:

	::

		Header line1
		Header line2
		============
		1.6723
		5.9731
		4.8881
		.....
		============
		Footer line1
		Footer line2

.. _surface:
.. topic:: SURFACE

	The SURFACE keyword can be used to work with surface from RMS in the irap format. The surface keyword is configured like this:

	::

		SURFACE TOP   OUTPUT_FILE:surf.irap   INIT_FILES:Surfaces/surf%d.irap   BASE_SURFACE:Surfaces/surf0.irap 

	The first argument, TOP in the example above, is the identifier you want to use for this surface in ert. The OUTPUT_FILE key is the name of surface file which ERT will generate for you, INIT_FILES points to a list of files which are used to initialize, and BASE_SURFACE must point to one existing surface file. When loading the surfaces ERT will check that all the headers are compatible. An example of a surface IRAP file is:

	::

		-996   511     50.000000     50.000000
		444229.9688   457179.9688  6809537.0000  6835037.0000
		260      -30.0000   444229.9688  6809537.0000
		0     0     0     0     0     0     0
		2735.7461    2734.8909    2736.9705    2737.4048    2736.2539    2737.0122
		2740.2644    2738.4014    2735.3770    2735.7327    2733.4944    2731.6448
		2731.5454    2731.4810    2730.4644    2730.5591    2729.8997    2726.2217
		2721.0996    2716.5913    2711.4338    2707.7791    2705.4504    2701.9187
		....

	The surface data will typically be fed into other programs like Cohiba or RMS. The data can be updated using e.g. the Smoother.

	**Initializing from the FORWARD MODEL**

	All the parameter types like FIELD,GEN_KW,GEN_PARAM and SURFACE can be initialized from the forward model. To achieve this you just add the setting FORWARD_INIT:True to the configuration. When using forward init the initialization will work like this:

	#. The explicit initialization from the case menu, or when you start a simulation, will be ignored.
	#. When the FORWARD_MODEL is complete ERT will try to initialize the node based on files created by the forward model. If the init fails the job as a whole will fail.
	#. If a node has been initialized, it will not be initialized again if you run again. [Should be possible to force this ....]

	When using FORWARD_INIT:True ERT will consider the INIT_FILES setting to find which file to initialize from. If the INIT_FILES setting contains a relative filename, it will be interpreted relativt to the runpath directory. In the example below we assume that RMS has created a file petro.grdecl which contains both the PERMX and the PORO fields in grdecl format; we wish to initialize PERMX and PORO nodes from these files:

	::

		FIELD   PORO  PARAMETER    poro.grdecl     INIT_FILES:petro.grdecl  FORWARD_INIT:True
		FIELD   PERMX PARAMETER    permx.grdecl    INIT_FILES:petro.grdecl  FORWARD_INIT:True

	Observe that forward model has created the file petro.grdecl and the nodes PORO and PERMX create the ECLIPSE input files poro.grdecl and permx.grdecl, to ensure that ECLIPSE finds the input files poro.grdecl and permx.grdecl the forward model should contain a job which will copy/convert petro.grdecl -> (poro.grdecl,permx.grdecl), this job should not overwrite existing versions of permx.grdecl and poro.grdecl. This extra hoops is not strictly needed in all cases, but strongly recommended to ensure that you have control over which data is used, and that everything is consistent in the case where the forward model is run again.


.. _summary:
.. topic:: SUMMARY

	The SUMMARY keyword is used to add variables from the ECLIPSE summary file to the parametrization. The keyword expects a string, which should have the format VAR:WGRNAME. Here, VAR should be a quantity, such as WOPR, WGOR, RPR or GWCT. Moreover, WGRNAME should refer to a well, group or region. If it is a field property, such as FOPT, WGRNAME need not be set to FIELD.

	*Example:*

	::

		-- Using the SUMMARY keyword to add diagnostic variables
		SUMMARY WOPR:MY_WELL
		SUMMARY RPR:8
		SUMMARY F*          -- Use of wildcards requires that you have entered a REFCASE.

	The SUMMARY keyword has limited support for '*' wildcards, if your key contains one or more '*' characters all matching variables from the refcase are selected. Observe that if your summary key contains wildcards you must supply a refcase with the REFCASE key - otherwise it will fail hard.

	**Note:** Properties added using the SUMMARY keyword are only diagnostic. I.e., they have no effect on the sensitivity analysis or history match. 


Keywords controlling the ES algorithm
-----------------------------------------
.. _keywords_controlling_the_es_algorithm:

.. _enkf_alpha:
.. topic:: ENKF_ALPHA 

	ENKF_ALPHA has some latex letters - need to be handled!!
	Scaling factor (double) used in outlier detection. Increasing this factor means that more observations will potentially be included in the assimilation. The default value is 1.50.

	Including outliers in the EnKF algorithm can dramatically increase the coupling between the ensemble members. It is therefore important to filter out these outlier data prior to data assimilation. An observation, \textstyle d^o_i, will be classified as an outlier if

	::

		|d^o_i - \bar{d}_i| > \mathrm{ENKF\_ALPHA} \left(s_{d_i} + \sigma_{d^o_i}\right), 

	where \textstyle\boldsymbol{d}^o is the vector of observed data, \textstyle\boldsymbol{\bar{d}} is the average of the forcasted data ensemble, \textstyle\boldsymbol{s_{d}} is the vector of estimated standard deviations for the forcasted data ensemble, and \textstyle\boldsymbol{s_{d}^o} is the vector standard deviations for the observation error (specified a priori). 


.. _enkf_bootstrap:
.. topic:: ENKF_BOOTSTRAP

	Boolean specifying if we want to resample the Kalman gain matrix in the update step. The purpose is to avoid that the ensemble covariance collapses. When this keyword is true each ensemble member will be updated based on a Kalman gain matrix estimated from a resampling with replacement of the full ensemble.

	In theory and in practice this has worked well when one uses a small number of ensemble members.


.. _enkf_cv_folds:
.. topic:: ENKF_CV_FOLDS

	Integer specifying how many folds we should use in the Cross-Validation (CV) scheme. Possible choices are the integers between 2 and the ensemble size (2-fold CV and leave-one-out CV respectively). However, a robust choice for the number of CV-folds is 5 or 10 (depending on the ensemble size).

	*Example:*

	::

		-- Setting the number of CV folds equal to 5 
		ENKF_CV_FOLDS 5

	Requires that the ENKF_LOCAL_CV keyword is set to TRUE


.. _enkf_force_ncomp:
.. topic:: ENKF_FORCE_NCOMP

	Bool specifying if we want to force the subspace dimension we want to use in the EnKF updating scheme (SVD-based) to a specific integer. This is an alternative to selecting the dimension using ENKF_TRUNCATION or ENKF_LOCAL_CV.

	*Example:*

	::

		-- Setting the the subspace dimension to 2
		ENKF_FORCE_NCOMP     TRUE
		ENKF_NCOMP              2



.. _enkf_local_cv:
.. topic:: ENKF_LOCAL_CV

	Boolean specifying if we want to select the subspace dimension in the SVD-based EnKF algorithm using Cross-Validation (CV) [1]. This is a more robust alternative to selecting the subspace dimension based on the estimated singular values (See ENKF_TRUNCATION), because the predictive power of the estimated Kalman gain matrix is taken into account.

	*Example:*

	::

		-- Select the subspace dimension using Cross-Validation
		ENKF_LOCAL_CV TRUE



.. _enkf_pen_press:
.. topic:: ENKF_PEN_PRESS

	Boolean specifying if we want to select the subspace dimension in the SVD-based EnKF algorithm using Cross-Validation (CV), and a penalised version of the predictive error sum of squares (PRESS) statistic [2]. This is recommended when overfitting is a severe problem (and when the number of ensemble members is small)

	*Example:*

	::

		-- Select the subspace dimension using Cross-Validation
		ENKF_LOCAL_CV TRUE

		-- Using penalised PRESS statistic
		ENKF_PEN_PRESS TRUE



.. _enkf_mode:
.. topic:: ENKF_MODE

	The ENKF_MODE keyword is used to select which EnKF algorithm to use. Use the value STANDARD for the original EnKF algorithm, or SQRT for the so-called square root scheme. The default value for ENKF_MODE is STANDARD.

	*Example A:*

	::

		-- Using the square root update
		ENKF_MODE SQRT

	*Example B:*

	::

		-- Using the standard update
		ENKF_MODE STANDARD

	The ENKF_MODE keyword is optional.


.. _enkf_merge_observations:
.. topic:: ENKF_MERGE_OBSERVATIONS

	If you use the ENKF_SCHED_FILE option to jump over several dates at a time you can choose whether you want to use all the observations in between, or just the final. If set to TRUE, all observations will be used. If set to FALSE, only the final observation is used. The default value for ENKF_MERGE_OBSERVATIONS is FALSE.

	*Example:*

	::

		-- Merge observations
		ENKF_MERGE_OBSERVATIONS TRUE


.. _enkf_ncomp:
.. topic:: ENKF_NCOMP

	Integer specifying the subspace dimension. Requires that ENKF_FORCE_NCOMP is TRUE.

.. _enkf_rerun:
.. topic:: ENKF_RERUN

	This is a boolean switch - TRUE or FALSE. Should the simulation start from time zero after each update.



.. _enkf_scaling:
.. topic:: ENKF_SCALING

	This is a boolean switch - TRUE (Default) or FALSE. If TRUE, we scale the data ensemble matrix to unit variance. This is generally recommended because the SVD-based EnKF algorithm is not scale invariant.


.. _enkf_truncation:
.. topic:: ENKF_TRUNCATION

	Truncation factor for the SVD-based EnKF algorithm (see Evensen, 2007). In this algorithm, the forecasted data will be projected into a low dimensional subspace before assimilation. This can substantially improve on the results obtained with the EnKF, especially if the data ensemble matrix is highly collinear (Saetrom and Omre, 2010). The subspace dimension, p, is selected such that

	::

	        \frac{\sum_{i=1}^{p} s_i^2}{\sum_{i=1}^r s_i^2} \geq \mathrm{ENKF\_TRUNCATION}, 

	where si is the ith singular value of the centered data ensemble matrix and r is the rank of this matrix. This criterion is similar to the explained variance criterion used in Principal Component Analysis (see e.g. Mardia et al. 1979).

	The default value of ENKF_TRUNCATION is 0.99. If ensemble collapse is a big problem, a smaller value should be used (e.g 0.90 or smaller). However, this does not guarantee that the problem of ensemble collapse will disappear. Note that setting the truncation factor to 1.00, will recover the Standard-EnKF algorithm if and only if the covariance matrix for the observation errors is proportional to the identity matrix.

        
.. _std_scale_correlated_obs:
.. topic:: STD_SCALE_CORRELATED_OBS

        With this kewyord you can instruct ERT to use the simulated
        data to estimate the correlations in the observations, and
        then inflate the observation standard deviation as a way to
        estimate the real information content in the observations. The
        method is based on PCA, the scaling factor is calculated as:

        ::

              \sqrt{\frac{N_{\sigma}}{N_{\mathrm{obs}}}

        where $N_{\sigma}$ is the number of singular components, at
        (fixed) truncation 0.95 and $N_{\mathrm{obs}}$ is the number
        of observations. The STD_SCALE_CORRELATED_OBS keyword will
        flatten all your observations, including temporal and spatial
        correlations. For more fine grained control you can use the
        STD_CALE_CORRELATED_OBS workflow job, or even write your own
        plugins.


        
.. _update_log_path:
.. topic:: UPDATE_LOG_PATH

	A summary of the data used for updates are stored in this directory.


**References**

* Evensen, G. (2007). "Data Assimilation, the Ensemble Kalman Filter", Springer.
* Mardia, K. V., Kent, J. T. and Bibby, J. M. (1979). "Multivariate Analysis", Academic Press.
* Saetrom, J. and Omre, H. (2010). "Ensemble Kalman filtering with shrinkage regression techniques", Computational Geosciences (online first). 


Analysis module
---------------
.. _analysis_module:

The final EnKF linear algebra is performed in an analysis module. The keywords to load, select and modify the analysis modules are documented here.

.. _analysis_load:
.. topic:: ANALYSIS_LOAD

	The ANALYSIS_LOAD key is the main key to load an analysis module:

	::

		ANALYSIS_LOAD ANAME  analysis.so

	The first argument ANAME is just an arbitrary unique name which you want to use to refer to the module later. The second argument is the name of the shared library file implementing the module, this can either be an absolute path as /path/to/my/module/ana.so or a relative file name as analysis.so. The module is loaded with dlopen() and the normal shared library search semantics applies.


.. _analysis_select:
.. topic:: ANALYSIS_SELECT

	This command is used to select which analysis module to actually use in the updates:

	::

		ANALYSIS_SELECT ANAME

	Here ANAME is the name you have assigned to the module when loading it with ANALYSIS_LOAD.


.. _analysis_set_var:
.. topic:: ANALYSIS_SET_VAR

	The analysis modules can have internal state, like e.g. truncation cutoff values, these values can be manipulated from the config file using the ANALYSIS_SET_VAR keyword:

	::

		ANALYSIS_SET_VAR  ANAME  ENKF_TRUNCATION  0.97

	To use this you must know which variables the module supports setting this way. If you try to set an unknown variable you will get an error message on stderr.


.. _analysis_copy:
.. topic:: ANALYSIS_COPY

	With the ANALYSIS_COPY keyword you can create a new instance of a module. This can be convenient if you want to run the same algorithm with the different settings:

	::

		ANALYSIS_LOAD   A1  analysis.so
		ANALYISIS_COPY  A1  A2

	We load a module analysis.so and assign the name A1; then we copy A1 -> A2. The module A1 and A2 are now 100% identical. We then set the truncation to two different values:

	::

		ANALYSIS_SET_VAR A1 ENKF_TRUNCATION 0.95
		ANALYSIS_SET_VAR A2 ENKF_TRUNCATION 0.98

**Developing analysis modules**

In the analysis module the update equations are formulated based on familiar matrix expressions, and no knowledge of the innards of the ERT program are required. Some more details of how modules work can be found here modules.txt. In principle a module is 'just' a shared library following some conventions, and if you are sufficiently savy with gcc you can build them manually, but along with the ert installation you should have utility script ert_module which can be used to build a module; just write ert_module without any arguments to get a brief usage description. 

Advanced optional keywords
--------------------------
.. _advanced_optional_keywords:

The keywords in this section, controls advanced features of the enkf application. Insight in the internals of the enkf application and/or ECLIPSE may be required to fully understand their effect. Moreover, many of these keywords are defined in the site configuration, and thus optional to set for the user, but required when installing the enkf application at a new site.


.. _add_fixed_length_schedule_kw:
.. topic:: ADD_FIXED_LENGTH_SCHEDULE_KW

	Real low level fix for some SCHEDULE parsing problems.


.. _add_static_kw:
.. topic:: ADD_STATIC_KW

	The restart files from ECLIPSE are organized by keywords, which are of three different types:

	#. Keywords containing the dynamic solution, e.g. pressure and saturations.
	#. Keywords containing various types of header information which is needed for a restart.
	#. Keywords containing various types of diagnostic information which is not needed for a restart.

	Keywords in category 2 and 3 are referred to as static keywords. To be able to restart ECLIPSE, the enkf application has to store the keywords in category 2, whereas keywords in category 3 can safely be dropped. To determine whether a particular keyword is in category 2 or 3 the enkf considers an internal list of keywords. The current list contains the keywords:

	::

		INTEHEAD LOGIHEAD DOUBHEAD IGRP SGRP XGRP ZGRP IWEL SWEL XWEL ZWEL 
		ICON SCON XCON HIDDEN STARTSOL PRESSURE SWAT SGAS RS RV ENDSOL ICAQNUM ICAQ IAAQ
		SCAQNUM SCAQ SAAQ ACAQNUM ACAQ XAAQ
		ISEG ILBS ILBR RSEG ISTHW ISTHG

	By using ADD_STATIC_KW you can dynamically add to this list. The magic string __ALL__ will add all static keywords. Use of the __ALL__ option is strongly discouraged, as it wastes a lot disk space.


.. _define:
.. topic:: DEFINE

	With the DEFINE keyword you can define key-value pairs which will be substituted in the rest of the configuration file. The DEFINE keyword expects two arguments: A key and a value to replace for that key. Later instances of the key enclosed in '<' and '>' will be substituted with the value. The value can consist of several strings, in that case they will be joined by one single space.

	*Example:*

	::

		-- Define ECLIPSE_PATH and ECLIPSE_BASE
		DEFINE  ECLIPSE_PATH  /path/to/eclipse/run
		DEFINE  ECLIPSE_BASE  STATF02
		DEFINE  KEY           VALUE1       VALUE2 VALUE3            VALUE4

		-- Set the GRID in terms of the ECLIPSE_PATH
		-- and ECLIPSE_BASE keys.
		GRID    <ECLIPSE_PATH>/<ECLIPSE_BASE>.EGRID

	Observe that when you refer to the keys later in the config file they must be enclosed in '<' and '>'. Furthermore, a key-value pair must be defined in the config file before it can be used. The final key define above KEY, will be replaced with VALUE1 VALUE2 VALUE3 VALUE4 - i.e. the extra spaces will be discarded.


.. _time_map:
.. topic:: TIME_MAP

        Normally the mapping between report steps and true dates is
        inferred by ERT indirectly by loading the ECLIPSE summary
        files. In cases where you do not have any ECLIPSE summary
        files you can use the TIME_MAP keyword to specify a file with
        dates which are used to establish this mapping:

	*Example:*

	::

		-- Load a list of dates from external file: "time_map.txt"
		TIME_MAP time_map.txt

	The format of the TIME_MAP file should just be a list of dates
	formatted as dd/mm/yyyy. The example file below has four dates:

	::

		01/01/2000
		01/07/2000
		01/01/2001
		01/07/2001

	

.. _schedule_prediction_file:
.. topic:: SCHEDULE_PREDICTION_FILE

	This is the name of a schedule prediction file. It can contain %d to get different files for different members. Observe that the ECLIPSE datafile should include only one schedule file, even if you are doing predictions. 


Keywords related to running the forward model
---------------------------------------------
.. _keywords_related_to_running_the_forward_model:



.. _forward_model:
.. topic:: FORWARD_MODEL

	The FORWARD_MODEL keyword is used to define how the simulations are executed. E.g., which version of ECLIPSE to use, which rel.perm script to run, which rock physics model to use etc. Jobs (i.e. programs and scripts) that are to be used in the FORWARD_MODEL keyword must be defined using the INSTALL_JOB keyword. A set of default jobs are available, and by default FORWARD_MODEL takes the value ECLIPSE100.

	The FORWARD_MODEL keyword expects a series of keywords, each defined with INSTALL_JOB. The enkf will execute the jobs in sequentially in the order they are entered. Note that the ENKF_SCHED_FILE keyword can be used to change the FORWARD_MODEL for sub-sequences of the run.

	*Example A:*

	::

		-- Suppose that "MY_RELPERM_SCRIPT" has been defined with
		-- the INSTALL_JOB keyword. This FORWARD_MODEL will execute
		-- "MY_RELPERM_SCRIPT" before ECLIPSE100.
		FORWARD_MODEL MY_RELPERM_SCRIPT ECLIPSE100

	*Example B:*

	::

		-- Suppose that "MY_RELPERM_SCRIPT" and "MY_ROCK_PHYSICS_MODEL" 
		-- has been defined with the INSTALL_JOB keyword. 
		-- This FORWARD_MODEL will execute "MY_RELPERM_SCRIPT", then 
		-- "ECLIPSE100" and in the end "MY_ROCK_PHYSICS_MODEL".
		FORWARD_MODEL MY_RELPERM_SCRIPT ECLIPSE100 MY_ROCK_PHYSICS_MODEL

	For advanced jobs you can pass string arguments to the job using a KEY=VALUE based approach, this is further described in: passing arguments. In available jobs in enkf you can see a list of the jobs which are available.


.. _job_script:
.. topic:: JOB_SCRIPT

	Running the forward model from enkf is a multi-level process which can be summarized as follows:

	#. A Python module called jobs.py is written and stored in the directory where the forward simulation is run. The jobs.py module contains a list of job-elements, where each element is a Python representation of the code entered when installing the job.
	#. The enkf application submits a Python script to the enkf queue system, this script then loads the jobs.py module to find out which programs to run, and how to run them.
	#. The job_script starts and monitors the individual jobs in the jobs.py module.

	The JOB_SCRIPT variable should point at the Python script which is managing the forward model. This should normally be set in the site wide configuration file.


.. _queue_system:
.. topic:: QUEUE_SYSTEM

	The keyword QUEUE_SYSTEM can be used to control where the
	simulation jobs are executed. It can take the values LSF,
	TORQUE, RSH and LOCAL.

	The LSF option will submit jobs to the LSF cluster at your
	location, and is recommended whenever LSF is available.

	The TORQUE option will submit jobs to the TORQUE a torque
	based system, using the commands qsub, qstat etc., if
	available.

	If you do not have access to LSF or TORQUE you can submit to
	your local workstation using the LOCAL option and to homemade
	cluster of workstations using the RSH option. All of the queue
	systems can be further configured, see separate sections.

	*Example:*

	::

		-- Tell ert to use the LSF cluster.
		QUEUE_SYSTEM LSF

	The QUEUE_SYSTEM keyword is optional, and usually defaults to
	LSF (this is site dependent).

Configuring LSF access
----------------------
.. _configuring_lsf_access:

The LSF system is the most useful of the queue alternatives, and also
the alternative with most options. The most important options are
related to how ert should submit jobs to the LSF system. Essentially
there are two methods ert can use when submitting jobs to the LSF
system:

#. For workstations which have direct access to LSF ert can submit
   directly with no further configuration. This is preferred solution,
   but unfortunately not very common.
#. Alternatively ert can issue shell commands to bsub/bjobs/bkill to
   submit jobs. These shell commands can be issued on the current
   workstation, or alternatively on a remote workstation using ssh.

The main switch between alternatives 1 and 2 above is the LSF_SERVER
option.

.. _lsf_server:
.. topic:: LSF_SERVER

	By using the LSF_SERVER option you essentially tell ert two
	things about how jobs should be submitted to LSF:

	#. You tell ert that jobs should be submitted using shell
           commands.
	#. You tell ert which server should be used when submitting

	So when your configuration file has the setting:

	::

		LSF_SERVER   be-grid01

	ert will use ssh to submit your jobs using shell commands on
	the server be-grid01. For this to work you must have
	passwordless ssh to the server be-grid01. If you give the
	special server name LOCAL ert will submit using shell commands
	on the current workstation.

	**bsub/bjobs/bkill options**

	By default ert will use the shell commands bsub,bjobs and
	bkill to interact with the queue system, i.e. whatever
	binaries are first in your PATH will be used. For fine grained
	control of the shell based submission you can tell ert which
	programs to use:

	::

		QUEUE_OPTION   LSF  BJOBS_CMD  /path/to/my/bjobs
		QUEUE_OPTION   LSF  BSUB_CMD   /path/to/my/bsub 

	*Example 1*

	::

		LSF_SERVER    be-grid01
		QUEUE_OPTION  LSF     BJOBS_CMD   /path/to/my/bjobs
		QUEUE_OPTION  LSF     BSUB_CMD    /path/to/my/bsub

	In this example we tell ert to submit jobs from the
	workstation be-grid01 using custom binaries for bsub and
	bjobs.

	*Example 2*

	::

		LSF_SERVER   LOCAL

	In this example we will submit on the current workstation,
	without using ssh first, and we will use the default bsub and
	bjobs executables. The remaining LSF options apply
	irrespective of which method has been used to submit the jobs.


.. _lsf_queue:
.. topic:: LSF_QUEUE

	The name of the LSF queue you are running ECLIPSE simulations in.


.. _max_running_lsf:
.. topic:: MAX_RUNNING_LSF

	The keyword MAX_RUNNING_LSF controls the maximum number of
	simultaneous jobs submitted to the LSF (Load Sharing Facility)
	queue when using the LSF option in QUEUE_SYSTEM.

	*Example:*

	::

		-- Submit no more than 30 simultaneous jobs
		-- to the LSF cluster.
		MAX_RUNNING_LSF 30




Configuring TORQUE access
-------------------------
.. _configuring_torque_access:

The TORQUE system is the only available system on some clusters. The
most important options are related to how ert should submit jobs to
the TORQUE system.

* Currently, the TORQUE option only works when the machine you are
  logged into have direct access to the queue system. ert then submit
  directly with no further configuration.

The most basic invocation is in other words:

::

	QUEUE_SYSTEM TORQUE

**qsub/qstat/qdel options**

By default ert will use the shell commands qsub,qstat and qdel to
interact with the queue system, i.e. whatever binaries are first in
your PATH will be used. For fine grained control of the shell based
submission you can tell ert which programs to use:

::

	QUEUE_SYSTEM TORQUE
	QUEUE_OPTION TORQUE QSUB_CMD /path/to/my/qsub
	QUEUE_OPTION TORQUE QSTAT_CMD /path/to/my/qstat 
	QUEUE_OPTION TORQUE QDEL_CMD /path/to/my/qdel 

In this example we tell ert to submit jobs using custom binaries for
bsub and bjobs.

**Name of queue**

The name of the TORQUE queue you are running ECLIPSE simulations in.

::

	QUEUE_OPTION TORQUE QUEUE name_of_queue

**Name of cluster (label)**

The name of the TORQUE cluster you are running ECLIPSE simulations
in. This might be a label (serveral clusters), or a single one, as in
this example baloo.

::

	QUEUE_OPTION TORQUE CLUSTER_LABEL baloo

**Max running jobs**

The queue option MAX_RUNNING controls the maximum number of
simultaneous jobs submitted to the queue when using (in this case) the
TORQUE option in QUEUE_SYSTEM.

::
  
	QUEUE_SYSTEM TORQUE
	-- Submit no more than 30 simultaneous jobs
	-- to the TORQUE cluster.
	QUEUE_OPTION TORQUE MAX_RUNNING 30

**Queue options controlling number of nodes and CPUs**

When using TORQUE, you must specify how many nodes a single job is
should to use, and how many CPUs per node. The default setup in ert
will use one node and one CPU. These options are called NUM_NODES and
NUM_CPUS_PER_NODE.

If the numbers specified is higher than supported by the cluster
(i.e. use 32 CPUs, but no node has more than 16), the job will not
start.

If you wish to increase these number, the program running (typically
ECLIPSE) will usually also have to be told to correspondingly use more
processing units (keyword PARALLEL)

::
	
	QUEUE_SYSTEM TORQUE
	-- Use more nodes and CPUs
	-- in the TORQUE cluster per job submitted
	-- This should (in theory) allow for 24 processing
	-- units to be used by eg. ECLIPSE
	QUEUE_OPTION TORQUE NUM_NODES 3
	QUEUE_OPTION TORQUE NUM_CPUS_PER_NODE 8

**Keep output from qsub**

Sometimes the error messages from qsub can be useful, if something is
seriously wrong with the environment or setup. To keep this output
(stored in your home folder), use this:

::

	QUEUE_OPTION TORQUE KEEP_QSUB_OUTPUT 1


** Slow submit to torque **

To be more gentle with the torqueue system you can instruct the driver
to sleep for every submit request. The argument to the SUBMIT_SLEEP is
the number of seconds to sleep for every submit, can be a fraction
like 0.5.

::

   QUEUE_OPTION TORQUE SUBMIT_SLEEP 0.25


** Torque debug log **

You can ask the torqueu driver to store a debug log of the jobs
submitted, and the resulting job id. This is done with the queue
option DEBUG_OUTPUT:

::
   
   QUEUE_OPTION TORQUE DEBUG_OUTPUT torque_log.txt



Configuring the LOCAL queue
---------------------------
.. _configuring_the_local_queue:


.. _max_running_local:
.. topic:: MAX_RUNNING_LOCAL

	The keyword MAX_RUNNING_LOCAL controls the maximum number of simultaneous jobs running when using the LOCAL option in QUEUE_SYSTEM. It is strongly recommended to not let MAX_RUNNING_LOCAL exceed the number of processors on the workstation used.

	*Example:*

	::

		-- No more than 3 simultaneous jobs
		MAX_RUNNING_LOCAL 3


Configuring the RSH queue
-------------------------
.. _configuring_the_rsh_queue:

.. _rsh_host:
.. topic:: RSH_HOST

	You can run the forward model in enkf on workstations using remote-shell commands. To use the RSH queue system you must first set a list of computers which enkf can use for running jobs:

	::

		RSH_HOST   computer1:2  computer2:2   large_computer:8

	Here you tell enkf that you can run on three different computers: computer1, computer2 and large_computer. The two first computers can accept two jobs from enkf, and the last can take eight jobs. Observe the following when using RSH:

	You must have passwordless login to the computers listed in RSH_HOST otherwise it will fail hard. enkf will not consider total load on the various computers; if have said it can take two jobs, it will get two jobs, irrespective of the existing load.

.. _rsh_command:
.. topic:: RSH_COMMAND

	This is the name of the executable used to invoke remote shell operations. Will typically be either rsh or ssh. The command given to RSH_COMMAND must either be in PATH or an absolute path.

	::

		MAX_RUNNING_RSH

	The keyword MAX_RUNNING_RSH controls the maximum number of simultaneous jobs running when using the RSH option in QUEUE_SYSTEM. It MAX_RUNNING_RSH exceeds the total capacity defined in RSH_HOST, it will automatically be truncated to that capacity.

	*Example:*

	::

		-- No more than 10 simultaneous jobs
		-- running via RSH.
		MAX_RUNNING_RSH 10



Keywords related to plotting
----------------------------
.. _keywords_related_to_plotting:


.. _image_viewer:
.. topic:: IMAGE_VIEWER

	The enkf application has some limited plotting capabilities. The plotting is based on creating a graphics file (currently a png file) and then viewing that file with an external application. The current default image viewer is a program called /usr/bin/display, but you can set IMAGE_VIEWER to point to another binary if that is desired. In particular it can be interesting to set as

	::

		IMAGE_VIEWER  /d/proj/bg/enkf/bin/noplot.sh

	then the plot files will be created, but they will not be flashing in your face (which can be a bit annoying).


.. _image_type:
.. topic:: IMAGE_TYPE

	This switch control the type of the plot figures/images created by the PLPLOT plot driver. It is by default set to png which works fine, but you can probably(??) use other popular graphics formats like gif and jpg as well.


.. _plot_driver:
.. topic:: PLOT_DRIVER

	This is the name of the sub system used for creating plots. The default system is called 'PLPLOT' - all the other options regarding plotting are sub options which are only relevant when you are using PLPLOT. In addition to PLPLOT you can chose the value 'TEXT'; this will actually not produce any plots, just textfiles which can be used for plotting with your favorite plotting program. This is particularly relevant if you have some special requirements to the plots.


.. _plot_errorbar:
.. topic:: PLOT_ERRORBAR

	Should errorbars on the observations be plotted?


.. _plot_errorbar_max:
.. topic:: PLOT_ERRORBAR_MAX

	When plotting summary vectors for which observations have been 'installed' with the OBS_CONFIG keyword, ert will plot the observed values. If you have less than PLOT_ERRORBAR_MAX observations ert will use errorbars to show the observed values, otherwise it will use two dashed lines indicating +/- one standard deviation. This option is only meaningful when PLOT_PLOT_ERRORBAR is activated.

	To ensure that you always get errorbars you can set PLOT_ERRORBAR_MAX to a very large value, on the other hand setting PLOT_ERRORBAR_MAX to 0 will ensure that ert always plots observation uncertainty using dashed lines of +/- one standard deviation.

	The setting here will also affect the output when you are using the TEXT driver to plot.


.. _plot_height:
.. topic:: PLOT_HEIGHT

	When the PLPLOT driver creates a plot file, it will have the height (in pixels) given by the PLOT_HEIGHT keyword. The default value for PLOT_HEIGHT is 768 pixels.


.. _plot_refcase:
.. topic:: PLOT_REFCASE

	Boolean variable which is TRUE if you want to add the refcases to the plots.

	*Example:*

	::

		PLOT_REFCASE TRUE



.. refcase_list:
.. topic:: REFCASE_LIST

	Provide one or more Eclipse .DATA files for a refcase to be added in the plots. This refcase will be plotted in different colours. The summary files related to the refcase should be in the same folder as the refcase.

	*Example:*

	::

		REFCASE_LIST /path/to/refcase1/file1.DATA /path/to/refcase2/file2.DATA



.. _plot_path:
.. topic:: PLOT_PATH

	The plotting engine creates 'files' with plots, they are stored in a directory. You can tell what that directory should be. Observe that the current 'casename' will automatically be appended to the plot path.


.. plot_width:
.. topic:: PLOT_WIDTH

	When the PLPLOT driver creates a plot file, it will have the width (in pixels) given by the PLOT_WIDTH keyword. The default value for PLOT_WIDTH is 1024 pixels. To create plots of half the size you use:

	::

		PLOT_HEIGHT   384
		PLOT_WIDTH    512



.. _rft_config:
.. topic:: RFT_CONFIG

	RFT_CONFIGS argument is a file with the name of the rfts followed by date (day month year) Ex.

	::

		RFT_CONFIG  ../models/wells/rft/WELLNAME_AND_RFT_TIME.txt

	Where the contents of the file is something like

	::

		be-linapp16(inmyr) -/models/wells/rft 34> more WELLNAME_AND_RFT_TIME.txt
		A-1HP  06 05 1993
		A-9HW  31 07 1993
		C-1HP  11 12 2007
		C-5HP  21 12 1999
		C-6HR  09 11 1999
		D-4HP  10 07 2003
		K-3HW  09 02 2003
		K-6HW  08 11 2002
		K-7HW  21 04 2005
		D-6HP  22 04 2006



.. _rftpath:
.. topic:: RFTPATH


	RFTPATHs argument is the path to where the rft-files are located

	::

		RFTPATH  ../models/wells/rft/




.. _hook_workflow:
.. topic:: HOOK_WORKFLOW

With the keyword :code:`HOOK_WORKFLOW` you can configure workflow
'hooks'; meaning workflows which will be run automatically at certain
points during ERTs execution. Currently there are two points in ERTs
flow of execution where you can hook in a workflow, either just before
the simulations start, :code:`PRE_SIMULATION` - or after all the
simulations have completed :code:`POST_SIMULATION`. The
:code:`POST_SIMULATION` hook is typically used to trigger QC
workflows:

::

   HOOK_WORKFLOW initWFLOW  PRE_SIMULATION
   HOOK_WORKFLOW QC_WFLOW1  POST_SIMULATION
   HOOK_WORKFLOW QC_WFLOW2  POST_SIMULATION

In this example the the workflow :code:`initWFLOW` will run after all
the simulation directiories have been created, just before the forward
model is submitted to the queue. When all the simulations are complete
the two workflows :code:`QC_WFLOW1` and :code:`QC_WFLOW2` will be
run. Observe that the workflows being 'hooked in' with the
:code:`HOOK_WORKFLOW` must be loaded with the :code:`LOAD_WORKFLOW`
keyword.


Manipulating the Unix environment
---------------------------------
.. _manipulating_the_unix_environment:

The two keywords SETENV and UPDATE_PATH can be used to manipulate the Unix environment of the ERT process, tha manipulations only apply to the running ERT instance, and are not applied to the shell.


.. _setenv:
.. topic:: SETENV

	You can use the SETENV keyword to alter the unix environment enkf is running in. This is probably most relevant for setting up the environment for the external jobs invoked by enkf.

	*Example:*

	::

		-- Setting up LSF
		SETENV  LSF_BINDIR      /prog/LSF/7.0/linux2.6-glibc2.3-x86_64/bin
		SETENV  LSF_LIBDIR      /prog/LSF/7.0/linux2.6-glibc2.3-x86_64/lib
		SETENV  LSF_UIDDIR      /prog/LSF/7.0/linux2.6-glibc2.3-x86_64/lib/uid
		SETENV  LSF_SERVERDIR   /prog/LSF/7.0/linux2.6-glibc2.3-x86_64/etc
		SETENV  LSF_ENVDIR      /prog/LSF/conf

	Observe that the SETENV command is not as powerful as the corresponding shell utility. In particular you can not use $VAR to refer to the existing value of an environment variable. To add elements to the PATH variable it is easier to use the UPDATE_PATH keyword.


.. _update_path:
.. topic:: UPDATE_PATH

	The UPDATE_PATH keyword will prepend a new element to an existing PATH variable. I.e. the config

	::

		UPDATE_PATH   PATH  /some/funky/path/bin

	will be equivalent to the shell command:

	::

		setenv PATH /some/funky/path/bin:$PATH

	The whole thing is just a workaround because we can not use $PATH.
