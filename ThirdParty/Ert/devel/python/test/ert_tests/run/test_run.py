from ert.test_run import TestRun , path_exists
from ert_tests import ExtendedTestCase

    


class RunTest(ExtendedTestCase):

    def test_init(self):
        with self.assertRaises(IOError):
            TestRun("Does/notExist")
            
        tr = TestRun("test-data/local/run/config.txt")
        self.assertEqual( tr.config_file , "config.txt")


    def test_cmd(self):
        tr = TestRun("test-data/local/run/config.txt")
        self.assertEqual( tr.ert_cmd , TestRun.default_ert_cmd )

        tr.ert_cmd = "/tmp/test"
        self.assertEqual( "/tmp/test" , tr.ert_cmd )


    def test_args(self):
        tr = TestRun("test-data/local/run/config.txt")
        self.assertEqual( tr.get_args() , [])

        tr.add_arg("-v")
        self.assertEqual( tr.get_args() , ["-v"])
        tr.add_arg("latest")
        self.assertEqual( tr.get_args() , ["-v" , "latest"])


    def test_workflows(self):
        tr = TestRun("test-data/local/run/config.txt")
        self.assertEqual( tr.get_workflows() , [])
        
        tr.add_workflow( "wf1" )
        tr.add_workflow( "wf2" )
        self.assertEqual( tr.get_workflows() , ["wf1" , "wf2"])

        
    def test_run_no_workflow(self):
        tr = TestRun("test-data/local/run/config.txt")
        with self.assertRaises(Exception):
            tr.run()

                    
    def test_name(self):
        tr = TestRun("test-data/local/run/config.txt" , "Name")
        self.assertEqual( "Name" , tr.name[:4] )

        tr = TestRun("test-data/local/run/config.txt")
        self.assertEqual( "test-data.local.run.config.txt" , tr.name[:len("test-data/local/run/config.txt")] )
        

    def test_runpath(self):
        tr = TestRun("test-data/local/run/config.txt" , "Name")
        self.assertEqual( TestRun.default_path_prefix , tr.path_prefix )
        

    def test_check(self):
        tr = TestRun("test-data/local/run/config.txt" , "Name")
        tr.add_check( path_exists , "some/file" )

        with self.assertRaises(Exception):
            tr.add_check( 25 , "arg")

        with self.assertRaises(Exception):
            tr.add_check( func_does_not_exist , "arg")
