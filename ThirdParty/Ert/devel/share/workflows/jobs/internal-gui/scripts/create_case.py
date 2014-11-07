from ert.enkf import ErtScript

class CreateCaseJob(ErtScript):

    def run(self, case_name):
        ert = self.ert()
        fs = ert.getEnkfFsManager().getFileSystem(case_name)
