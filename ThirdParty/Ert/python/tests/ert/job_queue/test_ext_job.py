import os.path

from ert.test import TestAreaContext, ExtendedTestCase
from ert.job_queue.ext_job import ExtJob


def create_valid_config( config_file ):
    with open(config_file , "w") as f:
        f.write("EXECUTABLE script.sh\n")

    with open("script.sh" , "w") as f:
        f.write("This is a script")


def create_config_missing_executable( config_file ):
    with open(config_file , "w") as f:
        f.write("EXECUTABLE missing_script.sh\n")


def create_config_missing_EXECUTABLE( config_file ):
    with open(config_file , "w") as f:
        f.write("EXECU missing_script.sh\n")


def create_config_executable_directory( config_file ):
    with open(config_file , "w") as f:
        f.write("EXECUTABLE /tmp\n")


def create_config_foreign_file( config_file ):
    with open(config_file , "w") as f:
        f.write("EXECUTABLE /etc/passwd\n")

        


class ExtJobTest(ExtendedTestCase):
    def test_load_forward_model(self):
        with self.assertRaises(IOError):
            job = ExtJob("CONFIG_FILE" , True)

        with TestAreaContext("python/job_queue/forward_model1"):
            create_valid_config("CONFIG")
            job = ExtJob("CONFIG" , True)
            self.assertEqual( job.name() , "CONFIG")
            
            self.assertEqual( job.get_executable() , os.path.join( os.getcwd() , "script.sh"))
            self.assertTrue( os.access( job.get_executable() , os.X_OK ))
        

            job = ExtJob("CONFIG" , True , name = "Job")
            self.assertEqual( job.name() , "Job")

            
        with TestAreaContext("python/job_queue/forward_model2"):
            create_config_missing_executable( "CONFIG" )
            with self.assertRaises(ValueError):
                job = ExtJob("CONFIG" , True)


        with TestAreaContext("python/job_queue/forward_model3"):
            create_config_missing_EXECUTABLE( "CONFIG" )
            with self.assertRaises(ValueError):
                job = ExtJob("CONFIG" , True)

                
        with TestAreaContext("python/job_queue/forward_model4"):
            create_config_executable_directory( "CONFIG" )
            with self.assertRaises(ValueError):
                job = ExtJob("CONFIG" , True)


        with TestAreaContext("python/job_queue/forward_model5"):
            create_config_foreign_file( "CONFIG" )
            with self.assertRaises(ValueError):
                job = ExtJob("CONFIG" , True)

