#  Copyright (C) 2016 Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
#  A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.
from ert.enkf.enums import EnkfInitModeEnum
from ert.enkf.enums import HookRuntime

from ert_gui.simulation.models import BaseRunModel, ErtRunError


class MultipleDataAssimilation(BaseRunModel):
    """
    Run Multiple Data Assimilation (MDA) Ensemble Smoother with custom weights.
    """

    def __init__(self):
        super(MultipleDataAssimilation, self).__init__(name="Multiple Data Assimilation (ES MDA)", phase_count=2)
        self.weights = "3, 2, 1" # default value

    def getWeights(self):
        return self.weights

    def setWeights(self, weights):
        str_weights = str(weights)
        print("Weights changed: %s" % str_weights)
        self.weights = str_weights

    def setAnalysisModule(self, module_name):
        module_load_success = self.ert().analysisConfig().selectModule(module_name)

        if not module_load_success:
            raise ErtRunError("Unable to load analysis module '%s'!" % module_name)


    def runSimulations(self, arguments):
        weights = self.parseWeights(self.weights)
        iteration_count = len(weights)

        self.setAnalysisModule(arguments["analysis_module"])

        print("Running MDA ES for %s  iterations\t%s" % (iteration_count, ", ".join(str(weight) for weight in weights)))
        weights = self.normalizeWeights(weights)

        weight_string = ", ".join(str(round(weight,3)) for weight in weights)
        print("Running MDA ES on (weights normalized)\t%s" % weight_string)


        self.setPhaseCount(iteration_count+2) # pre + post + weights

        target_case_format = arguments["target_case"]

        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        target_case_name = target_case_format % 0
        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_name)

        if not source_fs == target_fs:
            self.ert().getEnkfFsManager().switchFileSystem(target_fs)
            self.ert().getEnkfFsManager().initializeCurrentCaseFromExisting(source_fs, 0)

        active_realization_mask = arguments["active_realizations"]


        phase_string = "Running MDA ES %d iteration%s." % (iteration_count, ('s' if (iteration_count != 1) else ''))
        self.setPhaseName(phase_string, indeterminate=True)

        self.ert().getEnkfSimulationRunner().createRunPath(active_realization_mask, 1)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.PRE_SIMULATION )


        for iteration, weight in enumerate(weights):
            num_successful_realizations = self.simulateAndPostProcess(target_case_format, active_realization_mask, iteration)

            # We exit because the user has pressed 'Kill all simulations'.
            if self.userExitCalled( ):
                self.setPhase(iteration_count + 2, "Simulations stopped")
                return

            # We exit if there are too few realisations left for updating.
            self.checkHaveSufficientRealizations(num_successful_realizations)

            self.update(target_case_format, iteration, weights[iteration])

        self.setPhaseName("Post processing...", indeterminate=True)
        self.simulateAndPostProcess(target_case_format, active_realization_mask, iteration_count)

        self.setPhase(iteration_count + 2, "Simulations completed.")


    def update(self, target_case_format, iteration, weight):
        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        next_iteration = (iteration + 1)
        next_target_case_name = target_case_format % next_iteration
        target_fs = self.ert().getEnkfFsManager().getFileSystem(next_target_case_name)

        phase_string = "Analyzing iteration: %d with weight %f" % (next_iteration, weight)
        self.setPhase(self.currentPhase() + 1, phase_string, indeterminate=True)

        es_update = self.ert().getESUpdate( ) 
        es_update.setGlobalStdScaling(weight)
        success = es_update.smootherUpdate(source_fs, target_fs)
        
        if not success:
            raise UserWarning("Analysis of simulation failed for iteration: %d!" % next_iteration)


    def simulateAndPostProcess(self, target_case_format, active_realization_mask, iteration):
        target_case_name = target_case_format % iteration

        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_name)
        self.ert().getEnkfFsManager().switchFileSystem(target_fs)

        phase_string = "Running simulation for iteration: %d" % iteration
        self.setPhaseName(phase_string, indeterminate=True)
        self.ert().getEnkfSimulationRunner().createRunPath(active_realization_mask, iteration)

        phase_string = "Pre processing for iteration: %d" % iteration
        self.setPhaseName(phase_string)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.PRE_SIMULATION )

        phase_string = "Running forecast for iteration: %d" % iteration
        self.setPhaseName(phase_string, indeterminate=False)
        num_successful_realizations = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_CONDITIONAL, iteration)
        
        phase_string = "Post processing for iteration: %d" % iteration
        self.setPhaseName(phase_string, indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows(HookRuntime.POST_SIMULATION)

        return num_successful_realizations


    @staticmethod
    def normalizeWeights(weights):
        """ :rtype: list of float """
        if not weights:
            return []
        weights = [weight for weight in weights if abs(weight) != 0.0]
        from math import sqrt
        length = sqrt(sum((1.0 / x) * (1.0 / x) for x in weights))
        return [x * length for x in weights]


    @staticmethod
    def parseWeights(weights):
        if not weights:
            return []

        elements = weights.split(",")
        elements = [element.strip() for element in elements if not element.strip() == ""]

        result = []
        for element in elements:
            try:
                f = float(element)
                if f == 0:
                    print('Warning: 0 weight, will ignore')
                else:
                    result.append(f)
            except ValueError:
                raise ValueError('Warning: cannot parse weight %s' % element)

        return result
