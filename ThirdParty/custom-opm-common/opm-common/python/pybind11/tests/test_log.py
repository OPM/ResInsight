import unittest
from opm.io import OpmLog

class TestLog(unittest.TestCase):

   def test_log(self):
       OpmLog.info("Info message")
       OpmLog.debug("Debug message")
