Workflows and plugins
=====================

Contents

.. toctree::
   :maxdepth: 1

   workflows
   built_in

The Forward Model in ERT runs in the context of a single realization,
i.e. there is no communication between the different processes, and
the jobs are run outside of the main ERT process.

As an alternative to the forward model ERT has a system with
*workflows*. Using workflows you can automate cumbersome normal ERT
processes, and also invoke external programs. The workflows are run
serially on the workstation actually running ERT, and should not be
used for computationally heavy tasks.

   
