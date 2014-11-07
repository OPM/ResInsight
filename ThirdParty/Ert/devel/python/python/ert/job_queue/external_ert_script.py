from subprocess import Popen
from ert.job_queue import ErtScript


class ExternalErtScript(ErtScript):

    def __init__(self, ert, executable):
        super(ExternalErtScript, self).__init__(ert)

        self.__executable = executable
        self.__job = None

    def run(self, *args):
        command = [self.__executable]
        command.extend([str(arg) for arg in args])
        self.__job = Popen(command)
        self.__job.wait() # This should not be here?
        return None

    def cancel(self):
        super(ExternalErtScript, self).cancel()
        if self.__job is not None:
            self.__job.terminate()

            self.__job.kill()
