from ert.job_queue import ErtScript
from ert.test import TestAreaContext, ExtendedTestCase
from ert_tests.job_queue.workflow_common import WorkflowCommon


class ReturnErtScript(ErtScript):
    def run(self):
        return self.ert()

class AddScript(ErtScript):
    def run(self, arg1, arg2):
        return arg1 + arg2

class FailScript(ErtScript):
    def rum(self):
        pass

class NoneScript(ErtScript):
    def run(self, arg):
        assert arg is None


class ErtScriptTest(ExtendedTestCase):

    @staticmethod
    def createScripts():
        WorkflowCommon.createErtScriptsJob()

        with open("syntax_error_script.py", "w") as f:
            f.write("from ert.enkf not_legal_syntax ErtScript\n")

        with open("import_error_script.py", "w") as f:
            f.write("from ert.enkf import DoesNotExist\n")

        with open("empty_script.py", "w") as f:
            f.write("from ert.enkf import ErtScript\n")


    def test_ert_script_return_ert(self):
        script = ReturnErtScript("ert")
        result = script.initializeAndRun([], [])
        self.assertEqual(result, "ert")


    def test_ert_script_add(self):
        script = AddScript("ert")

        result = script.initializeAndRun([int, int], ["5", "4"])

        self.assertEqual(result, 9)

        with self.assertRaises(ValueError):
            result = script.initializeAndRun([int, int], ["5", "4.6"])


    def test_ert_script_failed_implementation(self):
        with self.assertRaises(UserWarning):
            script = FailScript("ert")


    def test_ert_script_from_file(self):
        with TestAreaContext("python/job_queue/ert_script") as work_area:
            ErtScriptTest.createScripts()

            script_object = ErtScript.loadScriptFromFile("subtract_script.py")

            script = script_object("ert")
            result = script.initializeAndRun([int, int], ["1", "2"])
            self.assertEqual(result, -1)


            # with self.assertRaises(ErtScriptError):
            self.assertIsNone(ErtScript.loadScriptFromFile("syntax_error_script.py"))
            self.assertIsNone(ErtScript.loadScriptFromFile("import_error_script.py"))
            self.assertIsNone(ErtScript.loadScriptFromFile("empty_script.py"))


    def test_none_ert_script(self):
        #Check if None is not converted to string "None"
        script = NoneScript("ert")

        script.initializeAndRun([str], [None])
