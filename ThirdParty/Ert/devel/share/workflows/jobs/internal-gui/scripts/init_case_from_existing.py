from ert.enkf import ErtScript
from ert.enkf.enums.enkf_state_type_enum import EnkfStateType


class InitCaseFromExistingJob(ErtScript):

    def run(self, source_case, target_case=None):
        ert = self.ert()
        source_fs = ert.getEnkfFsManager().getFileSystem(source_case)

        if target_case is None:
            target_fs = ert.getEnkfFsManager().getCurrentFileSystem()

        else:
            target_fs = ert.getEnkfFsManager().getFileSystem(target_case)

        ert.getEnkfFsManager().initializeCaseFromExisting(source_fs, 0, EnkfStateType.ANALYZED, target_fs)
