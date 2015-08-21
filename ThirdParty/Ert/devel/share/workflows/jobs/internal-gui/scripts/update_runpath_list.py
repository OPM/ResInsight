from ert.enkf import ErtScript

"""
This job is useful if you are running a workflow that requires the qc_module runpath_list
to be populated but your are not running any simulations.
"""

class UpdateRunpathListJob(ErtScript):
    def run(self):
        ert = self.ert()

        realization_count = ert.getEnsembleSize()
        iteration = 0

        ecl_config = ert.eclConfig()
        model_config = ert.getModelConfig()
        basename_fmt = ecl_config.getEclBase()
        runpath_fmt = model_config.getRunpathAsString()
        qc_module = ert.getPostSimulationHook()

        runpath_list = qc_module.getRunpathList()

        runpath_list.clear()

        for realization_number in range(realization_count):

            if basename_fmt is not None:
                basename = basename_fmt % realization_number
            else:
                raise UserWarning("EclBase not set!")

            if model_config.runpathRequiresIterations():
                runpath = runpath_fmt % (realization_number, iteration)
            else:
                runpath = runpath_fmt % realization_number

            runpath_list.add(realization_number, iteration, runpath, basename)
