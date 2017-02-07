import ert

from ert.job_queue import WorkflowJob
from ert.test import TestAreaContext, ExtendedTestCase
from .workflow_common import WorkflowCommon

from cwrap import Prototype

class _TestWorkflowJobPrototype(Prototype):
    lib = ert.load('libjob_queue')

    def __init__(self, prototype, bind=True):
        super(_TestWorkflowJobPrototype, self).__init__(_TestWorkflowJobPrototype.lib, prototype, bind=bind)

class WorkflowJobTest(ExtendedTestCase):
    _alloc_config    = _TestWorkflowJobPrototype("void* workflow_job_alloc_config()", bind = False)
    _alloc_from_file = _TestWorkflowJobPrototype("workflow_job_obj workflow_job_config_alloc(char*, void*, char*)", bind = False)


    def test_workflow_job_creation(self):
        workflow_job = WorkflowJob("Test")

        self.assertTrue(workflow_job.isInternal())
        self.assertEqual(workflow_job.name(), "Test")


    def test_read_internal_function(self):
        with TestAreaContext("python/job_queue/workflow_job") as work_area:
            WorkflowCommon.createInternalFunctionJob()
            WorkflowCommon.createErtScriptsJob()

            config = self._alloc_config()
            workflow_job = self._alloc_from_file("SELECT_CASE", config, "select_case_job")

            self.assertEqual(workflow_job.name(), "SELECT_CASE")
            self.assertTrue(workflow_job.isInternal())
            self.assertEqual(workflow_job.functionName(), "enkf_main_select_case_JOB")

            self.assertFalse(workflow_job.isInternalScript())
            self.assertIsNone(workflow_job.getInternalScriptPath())


            workflow_job = self._alloc_from_file("SUBTRACT", config, "subtract_script_job")
            self.assertEqual(workflow_job.name(), "SUBTRACT")
            self.assertTrue(workflow_job.isInternal())
            self.assertIsNone(workflow_job.functionName())

            self.assertTrue(workflow_job.isInternalScript())
            self.assertTrue(workflow_job.getInternalScriptPath().endswith("subtract_script.py"))



    def test_arguments(self):
        with TestAreaContext("python/job_queue/workflow_job") as work_area:
            WorkflowCommon.createInternalFunctionJob()

            config = self._alloc_config()
            job = self._alloc_from_file("PRINTF", config, "printf_job")

            self.assertEqual(job.minimumArgumentCount(), 4)
            self.assertEqual(job.maximumArgumentCount(), 5)
            self.assertEqual(job.argumentTypes(), [str, int, float, bool, str])

            self.assertTrue(job.run(None, ["x %d %f %d", 1, 2.5, True]))
            self.assertTrue(job.run(None, ["x %d %f %d %s", 1, 2.5, True, "y"]))

            with self.assertRaises(UserWarning): # Too few arguments
                job.run(None, ["x %d %f", 1, 2.5])

            with self.assertRaises(UserWarning): # Too many arguments
                job.run(None, ["x %d %f %d %s", 1, 2.5, True, "y", "nada"])


    def test_run_external_job(self):

        with TestAreaContext("python/job_queue/workflow_job") as work_area:
            WorkflowCommon.createExternalDumpJob()

            config = self._alloc_config()
            job = self._alloc_from_file("DUMP", config, "dump_job")

            self.assertFalse(job.isInternal())
            argTypes = job.argumentTypes()
            self.assertEqual( argTypes , [str , str] )
            self.assertIsNone(job.run(None, ["test", "text"]))

            with open("test", "r") as f:
                self.assertEqual(f.read(), "text")


    def test_run_internal_script(self):
        with TestAreaContext("python/job_queue/workflow_job") as work_area:
            WorkflowCommon.createErtScriptsJob()

            config = self._alloc_config()
            job = self._alloc_from_file("SUBTRACT", config, "subtract_script_job")

            result = job.run(None, ["1", "2"])

            self.assertEqual(result, -1)

