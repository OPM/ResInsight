----------------------
Built in workflow jobs
----------------------
.. _built_in_workflow_jobs:

ERT comes with a list of default workflow jobs which invoke internal ERT functionality. The internal workflows include:

Jobs related to case management
-------------------------------

**SELECT_CASE**

The job SELECT_CASE can be used to change the currently selected case. The SELECT_CASE job should be used as:

::

	SELECT_CASE  newCase

if the case newCase does not exist it will be created.

**CREATE_CASE**

The job CREATE_CASE can be used to create a new case without selecting it. The CREATE_CASE job should be used as:

::

	CREATE_CASE  newCase


**INIT_CASE_FROM_EXISTING**

The job INIT_CASE_FROM_EXISTING can be used to initialize a case from an existing case. The argument to the workflow should be the name of the workflow you are initializing from; so to initialize the current case from the existing case "oldCase":

::

	INIT_CASE_FROM_EXISTING oldCase

By default the job will initialize the 'current case', but optionally you can give the name of a second case which should be initialized. In this example we will initialize "newCase" from "oldCase":

::

	INIT_CASE_FROM_EXISTING oldCase newCase

When giving the name of a second case as target for the initialization job the 'current' case will not be affected.


Jobs related to export
----------------------

**EXPORT_FIELD**

The EXPORT_FIELD workflow job exports field data to roff or grdecl format dependent on the extension of the export file argument.The job takes the following arguments:

#. Field to be exported
#. Filename for export file, must contain %d
#. Report_step
#. State
#. Realization range

The filename must contain a %d. This will be replaced with the realization number.

The state parameter is either FORECAST or ANALYZED, BOTH is not supported.

The realization range parameter is optional. Default is all realizations.


Example use of this job in a workflow:

::

	EXPORT_FIELD PERMZ path_to_export/filename%d.grdecl 0 FORECAST 0,2

**EXPORT_FIELD_RMS_ROFF**

The EXPORT_FIELD_RMS_ROFF workflow job exports field data to roff format. The job takes the following arguments:

#. Field to be exported
#. Filename for export file, must contain %d
#. Report_step
#. State
#. Realization range

The filename must contain a %d. This will be replaced with the realization number.

The state parameter is either FORECAST or ANALYZED, BOTH is not supported.

The realization range parameter is optional. Default is all realizations.


Example uses of this job in a workflow:

::

	EXPORT_FIELD_RMS_ROFF PERMZ path_to_export/filename%d.roff 0 FORECAST
	EXPORT_FIELD_RMS_ROFF PERMX path_to_export/filename%d 0 FORECAST 0-5 


**EXPORT_FIELD_ECL_GRDECL**

The EXPORT_FIELD_ECL_GRDECL workflow job exports field data to grdecl format. The job takes the following arguments:

#. Field to be exported
#. Filename for export file, must contain %d
#. Report_step
#. State
#. Realization range

The filename must contain a %d. This will be replaced with the realization number.

The state parameter is either FORECAST or ANALYZED, BOTH is not supported.

The realization range parameter is optional. Default is all realizations.


Example uses of this job in a workflow:

::

	EXPORT_FIELD_ECL_GRDECL PERMZ path_to_export/filename%d.grdecl 0 ANALYZED
	EXPORT_FIELD_ECL_GRDECL PERMX path_to_export/filename%d 0 ANALYZED 0-5 


**EXPORT_RUNPATH**

The EXPORT_RUNPATH workflow job writes the runpath file RUNPATH_FILE for the selected case.

The job can have no arguments, or one can set a range of realizations and a range of iterations as arguments.

Example uses of this job in a workflow:

::

	EXPORT_RUNPATH 

With no arguments, entries for all realizations are written to the runpath file. If the runpath supports iterations, entries for all realizations in iter0 are written to the runpath file.

::

	EXPORT_RUNPATH 0-5 | *

A range of realizations and a range of iterations can be given. "|" is used as a delimiter to separate realizations and iterations. "*" can be used to select all realizations or iterations. In the example above, entries for realizations 0-5 for all iterations are written to the runpath file. 


Jobs related to analysis update
-------------------------------

**ANALYSIS_UPDATE**

This job will perform a update based on the current case, it is assumed that you have already completed the necessary simulations. By default the job will use all available data in the conditioning and store the updated parameters as the new initial parameters of the current case. However you can use optional argument to control which case the parameters go to, at which report step they are stored and also which report steps are considered when assembling the data. In the simplest form the ANALYSIS_UPDATE job looks like this:

::

	ANALYSIS_UPDATE 

In this case the initial parameters in the current case will be updated; using all available data in the conditioning process. In the example below we redirect the updated parameters to the new case NewCase:

::

	ANALYSIS_UPDATE NewCase

Optionally we can decide to update the parameters at a later stage, i.e. for instance at report step 100:

::

	ANALYSIS_UPDATE * 100

The '*' above means that we should update parameters in the current case. Finally we can limit the report steps used for data:

::

	ANALYSIS_UPDATE NewCaseII  0   10,20,30,40,100,120-200

In the last example 10,20,30,40,100,120-200 mean the report steps we are considering when updating. Observe that when we use the first argument to specify a new case the will be created if it does not exist, but not selected.


**ANALYSIS_ENKF_UPDATE**

The ANALYSIS_ENKF_UPDATE job will do an EnKF update at the current report step. The job requires the report step as the first argument:

::

	ANALYSIS_ENKF_UPDATE  10

by default the ENKF_UPDATE will use the observed data at the updatestep, but you can configure it use the report steps you like for data. In the example below the parameters at step 20 will be updated based on the observations at report step 0,5,10,15,16,17,18,19,20:

::

	ANALYSIS_ENKF_UPDATE  20  0,5,10,15-20 

The ANALYSIS_ENKF_UPDATE job is a special case of the ANALYSIS_UPDATE job, in principle the same can be achieved with the ENKF_UPDATE job.


Jobs related to running simulations - including updates
-------------------------------------------------------

**RUN_SMOOTHER**

The RUN_SMOOTHER job will run a simulation and perform an update. The updated parameters are default stored as the new initial parameters of the current case. Optionally the job can take 1 or 2 parameters. The case to store the updated parameters in can be specified as the first argument. A second argument can be specified to run a simulation with the updated parameters.


Run a simulation and an update. The updated parameters are stored as the new initial parameters of the current case:

::

	RUN_SMOOTHER


Run a simulation and an update. Store the updated parameters in the specified case. This case is created if it does not exist:

::

	RUN_SMOOTHER new_case


Run a simulation and an update. Store the updated parameters in the specified case, then run a simulation on this case:

::

	RUN_SMOOTHER new_case true


Run a simulation and an update. Store the updated parameters in the current case, then run a simulation again. Specify "*" to use the current case:

::

	RUN_SMOOTHER * true


**RUN_SMOOTHER_WITH_ITER**

This is exactly like the RUN_SMOOTHER job, but it has an additional first argumeent iter which can be used to control the iter number in the RUNPATH. When using the RUN_SMOOTHER job the iter number will be defaultetd to zero, and one in the optional rerun.

**ENSEMBLE_RUN**

The ENSEMBLE_RUN job will run a simulation, no update. The job take as optional arguments a range and/or list of which realizations to run.

::

	ENSEMBLE_RUN

::

	ENSEMBLE_RUN 1-5, 8


**LOAD_RESULTS**

The LOAD_RESULTS loads result from simulation(s). The job takes as optional arguments a range and/or list of which realizations to load results from. If no realizations are specified, results for all realizations are loaded.

::

	LOAD_RESULTS 

::

	LOAD_RESULTS 1-5, 8

In the case of multi iteration jobs, like e.g. the integrated smoother update, the LOAD_RESULTS job will load the results from iter==0. To control which iteration is loaded from you can use the LOAD_RESULTS_ITER job.


**LOAD_RESULTS_ITER**

The LOAD_RESULTS_ITER job is similar to the LOAD_RESULTS job, but it takes an additional first argument which is the iteration number to load from. This should be used when manually loading results from a multi iteration workflow:

::

	LOAD_RESULTS_ITER 

::

	LOAD_RESULTS_ITER 3 1-3, 8-10

Will load the realisations 1,2,3 and 8,9,10 from the fourth iteration (counting starts at zero).


**MDA_ES**

This workflow job (plugin) is used to run the *Multiple Data Assimilation Ensemble Smoother* :code:`MDA ES`.
Only two arguments are required to start the MDA ES process; target case format and iteration weights.
The weights implicitly indicate the number of iterations and the normalized global standard deviation scaling applied to the update step.

::

	MDA_ES target_case_%d observations/obs.txt

This command will use the weights specified in the obs.txt file. This file should have a single floating point number per line.
Alternatively the weights can be given as arguments as shown here.

::

	MDA_ES target_case_%d 8,4,2,1

This command will use the normalized version of the weights 8,4,2,1 and run for four iterations.
The prior will be in *target_case_0* and the results from the last iteration will be in *target_case_4*.
**Note: the weights must be listed with no spaces and separated with commas.**

If this is run as a plugin from Ertshell or the GUI a convenient user interface can be shown.


Jobs for ranking realizations
-----------------------------

**OBSERVATION_RANKING**

The OBSERVATION_RANKING job will rank realizations based on the delta between observed and simulated values for selected variables and time steps. The data for selected variables and time steps are summarized for both observed and simulated values, and then the simulated versus observed delta is used for ranking the realizations in increasing order. The job takes a name for the ranking as the first parameter, then the time steps, a "|" character and then variables to rank on. If no time steps and/or no variables are given, all time steps and variables are taken into account.


Rank the realizations on observation/simulation delta value for all WOPR data for time steps 0-20:

::

	OBSERVATION_RANKING Ranking1 0-20 | WOPR:*


Rank the simulations on observation/simulation delta value for all WOPR and WWCT data for time steps 1 and 10-50

::

	OBSERVATION_RANKING Ranking2 1, 10-50 | WOPR:* WWCT:*


Rank the realizations on observation/simulation delta value for WOPR:OP-1 data for all time steps

::

	OBSERVATION_RANKING Ranking3 | WOPR:OP-1

**DATA_RANKING**

The DATA_RANKING job will rank realizations in increasing or decreasing order on selected data value for a selected time step. The job takes as parameters the name of the ranking, the data key to rank on, increasing order and selected time steps. If no time step is given, the default is the last timestep.

Rank the realizations on PORO:1,2,3 on time step 0 in decreasing order

::

	DATA_RANKING Dataranking1 PORO:1,2,3 false 0


**EXPORT_RANKING**

The EXPORT_RANKING job exports ranking results to file. The job takes two parameters; the name of the ranking to export and the file to export to.

::

	EXPORT_RANKING Dataranking1 /tmp/dataranking1.txt


**INIT_MISFIT_TABLE**

Calculating the misfit for all observations and all timesteps can potentially be a bit timeconsuming, the results are therefor cached internally. If you need to force the recalculation of this cache you can use the INIT_MISFIT_TABLE job to initialize the misfit table that is used in observation ranking.

::

	INIT_MISFIT_TABLE


**STD_SCALE_CORRELATED_OBS**

The workflow job :code:`STD_SCALE_CORRELATED_OBS` is used to scale the
observation standard deviation in an attempt to reduce the effect of
correlations in the observed data. The job expects the observation
keys you want to consider as arguments:

::

	STD_SCALE_CORRELATED_OBS  WWCT:OP_1  WWCT:OP_2

In this example the observation uncertainty corresponding to
:code:`WWCT:OP_1` and :code:`WWCT:OP_2` will be scaled. Observe that
the :code:`STD_SCALE_CORRELATED_OBS` keyword will "flatten" in both
time and spatial direction. Wildcards are allow, i.e.

::

	STD_SCALE_CORRELATED_OBS  W*:OP_1

Will scale based on all the observations of well 'OP_1'. For more
advanced selections of observations, where you only want to scale
based on parts of the observation - spatially or temporaly you must
write your own plugin.

