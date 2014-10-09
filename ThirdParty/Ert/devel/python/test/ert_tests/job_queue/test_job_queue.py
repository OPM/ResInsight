from ert.job_queue import JobStatusType
from ert.test import ExtendedTestCase


class JobQueueTest(ExtendedTestCase):

    def testStatusEnum(self):
        source_path = "libjob_queue/include/ert/job_queue/queue_driver.h"
        self.assertEnumIsFullyDefined(JobStatusType, "job_status_type", source_path)


