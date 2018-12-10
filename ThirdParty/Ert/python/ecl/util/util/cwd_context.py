import os

class CWDContext(object):
    def __init__(self , path):
        self.cwd = os.getcwd()
        if os.path.isdir( path ):
            os.chdir( path )
        else:
            raise IOError("Path:%s does not exist" % path)

    def __exit__(self , exc_type , exc_val , exc_tb):
        os.chdir( self.cwd )
        return False

    def __enter__(self):
        return self
