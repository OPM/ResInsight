#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'clean.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 


import os, sys
def pyc_clean(dir):
    findcmd = 'find %s -name "*.pyc" -print' % dir
    count = 0
    for f in os.popen(findcmd).readlines():
        count += 1

        # try / except here in case user does not have permission to remove old .pyc files
        try:                 
            os.remove(str(f[:-1]))
        except:
            pass

    print "Removed %d .pyc files" % count

if __name__ == "__main__":
    script_path = os.path.abspath(__file__)
    prefix = os.path.dirname(script_path)
    pyc_clean("%s/../code" % (prefix))
