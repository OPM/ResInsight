from unittest import TestCase
import os.path


class OpmTest(TestCase):

    def createPath(self, path):
        return os.path.join( os.path.abspath( os.path.dirname(__file__ )) , "../../../bin/testdata" , path)
