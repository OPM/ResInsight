import os
import shutil

class PathContext(object):
    def __init__(self , path , store = False):
        self.path = path
        self.cwd = os.getcwd()
        self.store = store
        self.path_list = [ ]

        if not os.path.exists(path):
            work_path = path

            while True:
                work_path , base = os.path.split(work_path)
                if work_path:
                    if os.path.isdir(work_path):
                        break
                    else:
                        self.path_list.append( work_path )
                else:
                    break

            os.makedirs( path )
        else:
            if not self.store:
                raise OSError("Entry %s already exists" % path)
        os.chdir( path )



    def __exit__(self , exc_type , exc_val , exc_tb):
        os.chdir( self.cwd )
        if self.store == False:
            shutil.rmtree( self.path )
            for path in self.path_list:
                try:
                    os.rmdir( path )
                except OSError:
                    break

        return False


    def __enter__(self):
        return self
