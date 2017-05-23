import os
from ecl.test import ExtendedTestCase, PathContext,TestAreaContext


class PathContextTest(ExtendedTestCase):
    def test_error(self):
        with self.assertRaises(OSError):
            with PathContext("/usr/lib/testing"):
                pass

        with open("/tmp/file" , "w") as f:
            f.write("xx")

        with self.assertRaises(OSError):
            with PathContext("/tmp/file"):
                pass
        
            
    def test_chdir(self):
        with PathContext("/tmp/pc"):
            self.assertEqual( os.getcwd() , "/tmp/pc")


    def test_cleanup(self):
        with TestAreaContext("pathcontext"):
            os.makedirs("path/1")

            with self.assertRaises(OSError):
                with PathContext("path/1"):
                    pass
                
            with PathContext("path/1/next/2/level"):
                with open("../../file" , "w") as f:
                    f.write("Crap")
            
            self.assertTrue(os.path.isdir("path/1"))
            self.assertTrue(os.path.isdir("path/1/next"))
            self.assertFalse(os.path.isdir("path/1/next/2"))
            
            
