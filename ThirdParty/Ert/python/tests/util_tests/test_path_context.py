import os
from ecl.util.test import PathContext,TestAreaContext
from tests import EclTest


class PathContextTest(EclTest):
    
    def test_error(self):
        with TestAreaContext("pathcontext"):
            # Test failure on creating PathContext with an existing path 
            os.makedirs("path/1")
            with self.assertRaises(OSError):
                with PathContext("path/1"):
                    pass
                
            # Test failure on non-writable path 
            os.chmod("path/1", 0o0444)
            with self.assertRaises(OSError):
                with PathContext("path/1/subfolder"):
                    pass
            os.chmod("path/1", 0o0744)
            
            # Test failure on creating PathContext with an existing file
            with open("path/1/file", "w") as f:
                f.write("xx")
            with self.assertRaises(OSError):
                with PathContext("path/1/file"):
                    pass
    
            
    def test_chdir(self):
        with PathContext("/tmp/pc"):
            self.assertEqual(
                    os.path.realpath(os.getcwd()),
                    os.path.realpath("/tmp/pc")
                    )

    def test_cleanup(self):
        with TestAreaContext("pathcontext"):
            os.makedirs("path/1")
                
            with PathContext("path/1/next/2/level"):
                with open("../../file" , "w") as f:
                    f.write("Crap")
            
            self.assertTrue(os.path.isdir("path/1"))
            self.assertTrue(os.path.isdir("path/1/next"))
            self.assertFalse(os.path.isdir("path/1/next/2"))
            
            
