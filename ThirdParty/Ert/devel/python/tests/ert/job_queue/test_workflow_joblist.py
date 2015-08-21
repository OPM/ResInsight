from ert.job_queue import WorkflowJoblist, WorkflowJob
from ert.test import ExtendedTestCase, TestAreaContext
from .workflow_common import WorkflowCommon


class WorkflowJoblistTest(ExtendedTestCase):

    def test_workflow_joblist_creation(self):
        joblist = WorkflowJoblist()

        job = WorkflowJob("JOB1")

        joblist.addJob(job)

        self.assertTrue(job in joblist)
        self.assertTrue("JOB1" in joblist)

        job_ref = joblist["JOB1"]

        self.assertEqual(job.name(), job_ref.name())



    def test_workflow_joblist_with_files(self):
        with TestAreaContext("python/job_queue/workflow_joblist") as work_area:
            WorkflowCommon.createErtScriptsJob()
            WorkflowCommon.createExternalDumpJob()
            WorkflowCommon.createInternalFunctionJob()

            joblist = WorkflowJoblist()

            joblist.addJobFromFile("DUMP_JOB", "dump_job")
            joblist.addJobFromFile("SELECT_CASE_JOB", "select_case_job")
            joblist.addJobFromFile("SUBTRACT_SCRIPT_JOB", "subtract_script_job")

            self.assertTrue("DUMP_JOB" in joblist)
            self.assertTrue("SELECT_CASE_JOB" in joblist)
            self.assertTrue("SUBTRACT_SCRIPT_JOB" in joblist)

            self.assertFalse((joblist["DUMP_JOB"]).isInternal())
            self.assertTrue((joblist["SELECT_CASE_JOB"]).isInternal())
            self.assertTrue((joblist["SUBTRACT_SCRIPT_JOB"]).isInternal())
