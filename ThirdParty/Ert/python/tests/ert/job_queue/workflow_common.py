import os
import stat

class WorkflowCommon(object):

    @staticmethod
    def createExternalDumpJob():
        with open("dump_job", "w") as f:
            f.write("INTERNAL FALSE\n")
            f.write("EXECUTABLE dump.py\n")
            f.write("MIN_ARG 2\n")
            f.write("MAX_ARG 2\n")
            f.write("ARG_TYPE 0 STRING\n")


        with open("dump.py", "w") as f:
            f.write("#!/usr/bin/env python\n")
            f.write("import sys\n")
            f.write("f = open('%s' % sys.argv[1], 'w')\n")
            f.write("f.write('%s' % sys.argv[2])\n")
            f.write("f.close()\n")

        st = os.stat("dump.py")
        os.chmod("dump.py", st.st_mode | stat.S_IEXEC) # | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH

        with open("dump_workflow", "w") as f:
            f.write("DUMP dump1 dump_text_1\n")
            f.write("DUMP dump2 dump_<PARAM>_2\n")


    @staticmethod
    def createInternalFunctionJob():
        with open("select_case_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("FUNCTION enkf_main_select_case_JOB\n")
            f.write("MIN_ARG 1\n")
            f.write("MAX_ARG 1\n")
            f.write("ARG_TYPE 0 STRING\n")

        with open("printf_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("FUNCTION printf\n")
            f.write("MIN_ARG 4\n")
            f.write("MAX_ARG 5\n")
            f.write("ARG_TYPE 0 STRING\n")
            f.write("ARG_TYPE 1 INT\n")
            f.write("ARG_TYPE 2 FLOAT\n")
            f.write("ARG_TYPE 3 BOOL\n")
            f.write("ARG_TYPE 4 STRING\n")


        with open("compare_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("FUNCTION strcmp\n")
            f.write("MIN_ARG 2\n")
            f.write("MAX_ARG 2\n")
            f.write("ARG_TYPE 0 STRING\n")
            f.write("ARG_TYPE 1 STRING\n")


    @staticmethod
    def createErtScriptsJob():
        with open("subtract_script.py", "w") as f:
            f.write("from ert.job_queue import ErtScript\n")
            f.write("\n")
            f.write("class SubtractScript(ErtScript):\n")
            f.write("    def run(self, arg1, arg2):\n")
            f.write("        return arg1 - arg2\n")


        with open("subtract_script_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("SCRIPT subtract_script.py\n")
            f.write("MIN_ARG 2\n")
            f.write("MAX_ARG 2\n")
            f.write("ARG_TYPE 0 FLOAT\n")
            f.write("ARG_TYPE 1 FLOAT\n")


    @staticmethod
    def createWaitJob():
        with open("wait_job.py", "w") as f:
            f.write("from ert.job_queue import ErtScript\n")
            f.write("import time\n")
            f.write("\n")
            f.write("class WaitScript(ErtScript):\n")
            f.write("    def dump(self, filename, content):\n")
            f.write("        with open(filename, 'w') as f:\n")
            f.write("            f.write(content)\n")
            f.write("\n")
            f.write("    def run(self, number, wait_time):\n")
            f.write("        self.dump('wait_started_%d' % number, 'text')\n")
            f.write("        start = time.time()\n")
            f.write("        diff = 0\n")
            f.write("        while not self.isCancelled() and diff < wait_time: \n")
            f.write("           time.sleep(0.2)\n")
            f.write("           diff = time.time() - start\n")
            f.write("\n")
            f.write("        if self.isCancelled():\n")
            f.write("            self.dump('wait_cancelled_%d' % number, 'text')\n")
            f.write("        else:\n")
            f.write("            self.dump('wait_finished_%d' % number, 'text')\n")
            f.write("\n")
            f.write("        return None\n")


        with open("external_wait_job.sh", "w") as f:
            f.write("#!/usr/bin/env bash\n")
            f.write("echo \"text\" > wait_started_$1\n")
            f.write("sleep $2\n")
            f.write("echo \"text\" > wait_finished_$1\n")

        st = os.stat("external_wait_job.sh")
        os.chmod("external_wait_job.sh", st.st_mode | stat.S_IEXEC) # | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH

        with open("wait_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("SCRIPT wait_job.py\n")
            f.write("MIN_ARG 2\n")
            f.write("MAX_ARG 2\n")
            f.write("ARG_TYPE 0 INT\n")
            f.write("ARG_TYPE 1 INT\n")

        with open("external_wait_job", "w") as f:
            f.write("INTERNAL False\n")
            f.write("EXECUTABLE external_wait_job.sh\n")
            f.write("MIN_ARG 2\n")
            f.write("MAX_ARG 2\n")
            f.write("ARG_TYPE 0 INT\n")
            f.write("ARG_TYPE 1 INT\n")


        with open("wait_workflow", "w") as f:
            f.write("WAIT 0 1\n")
            f.write("WAIT 1 10\n")
            f.write("WAIT 2 1\n")

        with open("fast_wait_workflow", "w") as f:
            f.write("WAIT 0 1\n")
            f.write("EXTERNAL_WAIT 1 1\n")
