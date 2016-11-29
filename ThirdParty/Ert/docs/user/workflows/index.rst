Workflows and plugins
=====================

Contents

.. toctree::
   :maxdepth: 1

   workflows
   plugins
   built_in

The Forward Model in ERT runs in the context of a single realization,
i.e. there is no communication between the different processes, and
the jobs are run outside of the main ERT process.

As an alternative to the forward model ERT has a system with
*workflows*. Using workflows way you can automate cumbersome normal
ERT processes, and also invoke external programs. The workflows are
run serially on the workstation actually running ERT, and should not
be used for computationally heavy tasks.

In addition to workflows ERT has a system for *plugins*, a plugin is
quite similar to a workflow job, but the plugin appears in a gui
dropdown, and can even have a gui of it's own. A plugin is written in
Python and has *full access* to ERT internals. The plugins hook into
the running Python process, and can only be invoked from the gui or
:code:`ertshell`.

   
