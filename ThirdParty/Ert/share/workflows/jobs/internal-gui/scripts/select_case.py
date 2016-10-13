from ert.enkf import ErtScript

class SelectCaseJob(ErtScript):

    def run(self, case_name):
        ert = self.ert()
        fs = ert.getEnkfFsManager().getFileSystem(case_name)
        ert.getEnkfFsManager().switchFileSystem(fs)

