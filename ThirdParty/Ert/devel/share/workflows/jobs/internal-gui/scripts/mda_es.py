from math import sqrt
import os

from PyQt4.QtGui import QLabel

from ert.enkf import ErtPlugin, CancelPluginException, EnkfInitModeEnum, EnkfStateType, HookRuntime
from ert.enkf.hook_manager import HookManager
from ert.util import BoolVector
from ert_gui.ide.keywords.definitions import ProperNameFormatArgument, NumberListStringArgument
from ert_gui.models.mixins.connectorless import DefaultPathModel, DefaultNameFormatModel, StringModel
from ert_gui.widgets.custom_dialog import CustomDialog
from ert_gui.widgets.option_widget import OptionWidget
from ert_gui.widgets.path_chooser import PathChooser
from ert_gui.widgets.string_box import StringBox


class MDAEnsembleSmootherJob(ErtPlugin):
    """
    This plugin runs Multiple Data Assimilation Ensemble Smoother (MDA ES) with custom weights.
    """

    def getName(self):
        return "MDA Ensemble Smoother"

    def getDescription(self):
        return "Run Multiple Data Assimilation Ensemble Smoother (MDA ES) with custom iteration weights."

    def parseWeights(self, weights):
        """ :rtype: list of float """
        if os.path.exists(weights) and os.path.isfile(weights):
            return self.parseWeightsFromFile(weights)
        else:
            return self.parseWeightsFromString(weights)

    def run(self, target_case_format, weights):
        if not "%d" in target_case_format:
            raise UserWarning("The target case format requires a %d. For example: default_%d")

        weights = self.parseWeights(weights)
        weights = self.normalizeWeights(weights)

        iteration_count = len(weights)

        print("Running MDA ES for %d iterations with the following normalized weights: %s" % (iteration_count, ", ".join(str(weight) for weight in weights)))

        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        target_case_name = target_case_format % 0
        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_name)

        if not source_fs == target_fs:
            self.ert().getEnkfFsManager().switchFileSystem(target_fs)
            self.ert().getEnkfFsManager().initializeCurrentCaseFromExisting(source_fs, 0, EnkfStateType.ANALYZED)

        active_realization_mask = BoolVector(True, self.ert().getEnsembleSize())

        for iteration, weight in enumerate(weights):
            self.simulateAndPostProcess(target_case_format, active_realization_mask, iteration)
            self.update(target_case_format, iteration, weights[iteration])

        self.simulateAndPostProcess(target_case_format, active_realization_mask, iteration_count)

        return "MDA ES completed successfully!"

    def update(self, target_case_format, iteration, weight):
        self.checkIfCancelled()

        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        next_iteration = (iteration + 1)
        next_target_case_name = target_case_format % next_iteration
        target_fs = self.ert().getEnkfFsManager().getFileSystem(next_target_case_name)

        print("[%s] Analyzing iteration: %d with weight %f" % (next_target_case_name, next_iteration, weight))
        self.ert().analysisConfig().setGlobalStdScaling(weight)
        success = self.ert().getEnkfSimulationRunner().smootherUpdate(source_fs, target_fs)

        if not success:
            raise UserWarning("[%s] Analysis of simulation failed for iteration: %d!" % (next_target_case_name, next_iteration))


    def simulateAndPostProcess(self, target_case_format, active_realization_mask, iteration):
        self.checkIfCancelled()

        target_case_name = target_case_format % iteration

        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_name)
        self.ert().getEnkfFsManager().switchFileSystem(target_fs)

        print("[%s] Running simulation for iteration: %d" % (target_case_name, iteration))


        success = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_CONDITIONAL, iteration)

        if not success:
            self.checkSuccessCount(active_realization_mask)

        self.checkIfCancelled()

        print("[%s] Post processing for iteration: %d" % (target_case_name, iteration))
        self.ert().getEnkfSimulationRunner().runWorkflows(HookRuntime.POST_SIMULATION)


    def checkSuccessCount(self, active_realization_mask):
        min_realization_count = self.ert().analysisConfig().getMinRealisations()
        success_count = active_realization_mask.count()

        if min_realization_count > success_count:
            raise UserWarning("Simulation failed! Number of successful realizations less than MIN_REALIZATIONS %d < %d" % (success_count, min_realization_count))
        elif success_count == 0:
            raise UserWarning("Simulation failed! All realizations failed!")


    def getArguments(self, parent=None):
        description = "The MDA Ensemble Smoother requires some information before running:"
        dialog = CustomDialog("MDA Ensemble Smoother", description, parent)

        iterated_target_case_format_model = DefaultNameFormatModel(self.getDefaultTargetCaseFormat())
        iterated_target_case_format_box = StringBox(iterated_target_case_format_model, "Target case format", "config/simulation/iterated_target_case_format")
        iterated_target_case_format_box.setValidator(ProperNameFormatArgument())

        iteration_weights_path_model = DefaultPathModel("", must_exist=True)
        iteration_weights_path_chooser = PathChooser(iteration_weights_path_model, path_label="Iteration weights file")

        custom_iteration_weights_model = StringModel("1")
        custom_iteration_weights_box = StringBox(custom_iteration_weights_model, "Custom iteration weights", "config/simulation/iteration_weights")
        custom_iteration_weights_box.setValidator(NumberListStringArgument())

        option_widget = OptionWidget("Relative Weights")
        option_widget.addHelpedWidget("Custom", custom_iteration_weights_box)
        option_widget.addHelpedWidget("File", iteration_weights_path_chooser)

        dialog.addOption(iterated_target_case_format_box)
        dialog.addOption(option_widget)
        dialog.addSpace()
        dialog.addWidget(QLabel("Example Custom Relative Weights: '8,4,2,1'\n"
                                "This means MDA-ES will half the weight\n"
                                "applied to the Observation Errors from one\n"
                                "iteration to the next across 4 iterations."), "Note")

        dialog.addButtons()

        success = dialog.showAndTell()

        if success:
            optioned_widget = option_widget.getCurrentWidget()

            if optioned_widget == iteration_weights_path_chooser:
                weights = iteration_weights_path_model.getPath()
            elif optioned_widget == custom_iteration_weights_box:
                weights = custom_iteration_weights_model.getValue()
            else:
                weights = "1"

            return [iterated_target_case_format_model.getValue(), weights]

        raise CancelPluginException("User cancelled!")

    def getDefaultTargetCaseFormat(self):
        if self.ert().analysisConfig().getAnalysisIterConfig().caseFormatSet():
            return self.ert().analysisConfig().getAnalysisIterConfig().getCaseFormat()
        else:
            case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()
            return "%s_%%d" % case_name

    def parseWeightsFromFile(self, weights):
        result = []
        with open(weights, "r") as f:
            for line in f:
                result.append(float(line))
        return result

    def parseWeightsFromString(self, weights):
        elements = weights.split(",")
        result = []
        for element in elements:
            element = element.strip()
            result.append(float(element))

        return result

    def normalizeWeights(self, weights):
        """ :rtype: list of float """
        length = sqrt(sum((1.0 / x) * (1.0 / x) for x in weights))
        return [x * length for x in weights]
