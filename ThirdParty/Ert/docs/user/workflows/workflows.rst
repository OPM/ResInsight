.. _workflows:

Configuring workflows in ERT consists of two steps: *installing the
jobs* which should be available for ERT to use in workflows, and then
subsequently assemble one or more jobs, with arguments, in a
workflow. You can use predefined workflow jobs, or create your
own. There are no predefined complete workflows.



Workflow jobs
=============

The workflow jobs are quite similar to the jobs in the forward model,
in particular the jobs are described by a configuration file which
resembles the one used by the forward model jobs. The workflow jobs
can be of two fundamentally different types - *external* and *internal*.


External workflow jobs
----------------------

These jobs invoke an external program/script to do the job, this is
very similar to the jobs of the forward model, but instead of running
as separate jobs on the cluster - one for each realization, the
workflow jobs will be invoked on the workstation running ert, and
typically go through all the realizations in one loop.

The executable invoked by the workflow job can be an executable you
have written yourself - in any language, or it can be an existing
Linux command like e.g. :code:`cp` or :code:`mv`.

Internal workflow jobs
----------------------

These jobs invoke a function in the address space of the ERT program
itself; i.e. they are run as part of the running ERT process - and can
in principle do anything that ERT can do itself. There are two two
varieties of the internal workflow jobs:


Invoke a pre exported function
..............................

This is the simplest, where you can invoke a a predefined ERT
function. The function must already have been marked as *exported* in
the ert code base. The list of predefined workflow jobs based on this
method can be found here: :ref:`built_in_workflow_jobs`. Marking a new
function as exported is quite simple, but it requires changes to the
core code and a new version must be installed.

.. _ert_script:
Run a Python Script
...................

If you are using one of the Python based frontends, *gert* or
*erthsell*, you can write your own Python script which is run as part
of the existing process. By using the full ert Python api you get
access to powerful customization/automization features. Below is an
example of :code:`ErtScript` which calculates the misfit for all
observations and prints the result to a text file. All Python scripts
of this kind must:

  1. Be implemented as a class which iherits from :code:`ErtScript`
  2. Have a method :code:`run(self)` which does the actual job


.. code:: python

    from ert.util import DoubleVector
    from ert.enkf import ErtScript

    class ExportMisfit(ErtScript):

        def run(self):
            # Get a handle to running ert instance
            ert = self.ert()


            # Get a handle to the case / filesystem we are interested in;
            # this should ideally come as an argument - not just use current.
            fs = ert.getEnkfFsManager().getCurrentFileSystem()


            # How many realisations:
            ens_size = ert.getEnsembleSize( )

            
            # Get a handle to all the observations
            observations = ert.getObservations()

            
            # Iterate through all the observations; each element in this
            # iteration corresponds to one key in the observations file.
            for obs in observations:
                misfit = DoubleVector()
                for iens in range(ens_size):
                    chi2 = obs.getTotalChi2( fs , iens )
                    misfit[iens] = chi2

                permutation = misfit.permutationSort( )

                print " #      Realisation     Misfit:%s" % obs.getObservationKey()
                print "-----------------------------------"
                for index in range(len(misfit)):
                    iens = permutation[index]
                    print "%2d     %2d            %10.5f" % (index , iens , misfit[iens])

                print "-----------------------------------\n"




Configuring workflow jobs
-------------------------

Workflow jobs are configured with a small configuration file much like
the configuration file used to install forward model jobs. The
keywords used in the configuration file are in two *clases* - those
related to how the job should located/run and the arguments which
should passed from the workflow to the job.


Configure an internal job
.........................

When configuring an internal workflow job the keyword :code:`INTERNAL`
is given the value :code:`TRUE` to indicate that this is an internal
job. In addition you give the name of the C function you wish to
invoke. By default the workflow job will search for the function
symbol in the current process space, but by passing the :code:`MODULE`
keyword you can request the loading of an external shared library:

::

    INTERNAL  TRUE                     -- The job will call an internal function of the current running ERT instance.               
    FUNCTION  enkf_main_plot_all       -- Name of the ERT function we are calling; must be marked exportable.
    MODULE    /name/of/shared/library  -- Very optional - to load an extra shared library.


Configure a an internal job: Python
...................................

If you wish to implement your job as a Python class, derived from
:code:`ErtScript` you should use the :code:`SCRIPT` keyword instead of
:code:`FUNCTION`, to point to an existing Python script:

::

   INTERNAL  TRUE                     -- The job will call an internal function of the current running ERT instance.               
   SCRIPT sripts/my_script.py         -- An existing Python script

Observe that the important thing here is the fact that we are writing
an *internal* Python script; if you are writing an external script to
loop through all your realization folders that will typically be an
*external* script, and in that case the implementation language -
i.e. Python, Perl, C++, F77 ... has no relevance.


Configure an external job
.........................

An *external* job is a workflow job which is implemented in an
external executable, i.e. typically a script written in for instance
Python. When configuring an external job the most important keyword is
:code:`EXECUTABLE` which is used to give the path to the external
executable:

::

    INTERNAL   FALSE                    -- This is the default - not necessary to include.               
    EXECUTABLE path/to/program          -- Path to a program/script which will be invoked by the job.


Configuring the arguments
.........................

In addition to the INTERNAL, FUNCTION, MODULE and EXECUTABLE keys
which are used to configure what the job should do there are some keys
which can be used to configure the number of arguments and their
type. These arguments apply to both internal and external jobs:

::

	MIN_ARG    2                 -- The job should have at least 2 arguments.
	MAX_ARG    3                 -- The job should have maximum 3 arguments.
	ARG_TYPE   0    INT          -- The first argument should be an integer
	ARG_TYPE   1    FLOAT        -- The second argument should be a float value
	ARG_TYPE   2    STRING       -- The third argument should be a string - the default.

The MIN_ARG,MAX_ARG and ARG_TYPE arguments are used to validate workflows.


**Example 1 : Plot variables**

::

	-- FILE: PLOT --
	INTERNAL  TRUE   
	FUNCTION  ert_tui_plot_JOB
	MIN_ARG   1

This job will use the ERT internal function ert_tui_plot_JOB to plot
an ensemble of an arbitrary ERT variable. The job needs at least one
argument; there is no upper limit on the number of arguments.


**Example 2 : Run external script**

::

	-- FILE: ECL_HIST --
	EXECUTABLE  Script/ecl_hist.py
	MIN_ARG     3

This job will invoke the external script Script/ecl_host.py; the
script should have at least three commandline arguments. The path to
the script, Script/ecl_hist.py is interpreted relative to the location
of the configuration file.


Loading workflow jobs into ERT
------------------------------

Before the jobs can be used in workflows they must be 'loaded' into
ERT. This is done with two different ERT keywords:

::

	LOAD_WORKFLOW_JOB     jobConfigFile     JobName   

The LOAD_WORKFLOW_JOB keyword will load one workflow job. The name of
the job is optional, if not provided the job will get name from the
configuration file. Alternatively you can use the command
WORKFLOW_JOB_DIRECTORY which will load all the jobs in a
directory. The command:

::

	WORKFLOW_JOB_DIRECTORY /path/to/jobs

will load all the workflow jobs in the /path/to/jobs
directory. Observe that all the files in the /path/to/jobs directory
should be job configuration files. The jobs loaded in this way will
all get the name of the file as the name of the job. The
:code:`WORKFLOW_OB_DIRECTORY` keyword will *not* load configuration
files recursively.




Complete Workflows
==================

A workflow is a list of calls to jobs, with additional arguments. The
job name should be the first element on each line. Based on the two
jobs PLOT and ECL_HIST we can create a small workflow example:

::

	PLOT      WWCT:OP_1   WWCT:OP_3  PRESSURE:10,10,10
	PLOT      FGPT        FOPT       
	ECL_HIST  <RUNPATH_FILE>   <QC_PATH>/<ERTCASE>/wwct_hist   WWCT:OP_1  WWCT:OP_2

In this workflow we create plots of the nodes
WWCT:OP_1;WWCT:OP_3,PRESSURE:10,10,10,FGPT and FOPT. The plot job we
have created in this example is completely general, if we limited
ourselves to ECLIPSE summary variables we could get wildcard
support. Then we invoke the ECL_HIST example job to create a
histogram. See below for documentation of <RUNPATH_FILE>,<QC_PATH> and
<ERTCASE>.


Loading workflows
-----------------

Workflows are loaded with the configuration option LOAD_WORKFLOW:

::

	LOAD_WORKFLOW  /path/to/workflow/WFLOW1
	LOAD_WORKFLOW  /path/to/workflow/workflow2  WFLOW2

The LOAD_WORKFLOW takes the path to a workflow file as the first
argument. By default the workflow will be labeled with the filename
internally in ERT, but optionally you can supply a second extra
argument which will be used as name for the workflow. Alternatively
you can load a workflow interactively.


Automatically run workflows : HOOK_WORKFLOW
-------------------------------------------
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



Locating the realisations: <RUNPATH_FILE>
-----------------------------------------

Context must be passed between the main ERT process and the script
through the use of string substitution, in particular the 'magic' key
<RUNPATH_FILE> has been introduced for this purpose.

Many of the external workflow jobs involve looping over all the
realisations in a construction like this:

::

	for each realisation:
	    // Do something for realisation
	summarize()

When running an external job in a workflow there is no direct transfer
of information between the main ERT process and the external
script. We therefor must have a convention for transfering the
information of which realisations we have simulated on, and where they
are located in the filesystem. This is done through a file which looks
like this:

::

	0   /path/to/real0  CASE_0000
	1   /path/to/real1  CASE_0001
	...
	9   /path/to/real9  CASE_0009

The name and location of this file is available as the magical string
<RUNPATH_FILE> and that is typically used as the first argument to
external workflow jobs which should iterate over all realisations. The
realisations referred to in the <RUNPATH_FILE> are meant to be last
simulations you have run; the file is updated every time you run
simulations. This implies that it is (currently) not so convenient to
alter which directories should be used when running a workflow.



