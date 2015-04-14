from collections import OrderedDict
import os
from ert.enkf import ErtScript, RealizationStateEnum, EnkfStateType
from ert.util import BoolVector

"""
This job exports misfit data into a chosen file or to the default gen_kw export file (parameters.txt)
"""

class ExportMisfitDataJob(ErtScript):

    def run(self, target_file=None):
        ert = self.ert()
        fs = ert.getEnkfFsManager().getCurrentFileSystem()

        if target_file is None:
            target_file = ert.getModelConfig().getGenKWExportFile()

        runpath_list = ert.getPostSimulationHook().getRunpathList()

        active_list = self.createActiveList(fs)

        for runpath_node in runpath_list:
            if runpath_node.realization in active_list:

                if not os.path.exists(runpath_node.runpath):
                    os.makedirs(runpath_node.runpath)

                target_path = os.path.join(runpath_node.runpath, target_file)

                parameters = self.parseTargetFile(target_path)

                misfit_sum = 0.0
                for obs_vector in ert.getObservations():
                    misfit = obs_vector.getTotalChi2(fs, runpath_node.realization, EnkfStateType.FORECAST)

                    key = "MISFIT:%s" % obs_vector.getObservationKey()
                    parameters[key] = misfit

                    misfit_sum += misfit

                parameters["MISFIT:TOTAL"] = misfit_sum

                self.dumpParametersToTargetFile(parameters, target_path)



    def parseTargetFile(self, target_path):
        parameters = OrderedDict()

        if os.path.exists(target_path) and os.path.isfile(target_path):
            with open(target_path, "r") as input_file:
                lines = input_file.readlines()

                for line in lines:
                    tokens = line.split()

                    if len(tokens) == 2:
                        parameters[tokens[0]] = tokens[1]
                    else:
                        raise UserWarning("The file '%s' contains errors. Expected format for each line: KEY VALUE" % target_path)

        return parameters


    def dumpParametersToTargetFile(self, parameters, target_path):
        with open(target_path, "w") as output:
            for key in parameters:
                output.write("%s %s\n" % (key, parameters[key]))


    def createActiveList(self, fs):
        state_map = fs.getStateMap()
        ens_mask = BoolVector(False, self.ert().getEnsembleSize())
        state_map.selectMatching(ens_mask, RealizationStateEnum.STATE_HAS_DATA)
        active_list = BoolVector.createActiveList(ens_mask)

        return active_list