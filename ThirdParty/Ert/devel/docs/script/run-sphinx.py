#!/usr/bin/env python
import sys
import os
import subprocess
import shutil

config_file = sys.argv[1]
PYTHONPATH = sys.argv[2]
work_path = sys.argv[3]

os.environ["PYTHONPATH"] = PYTHONPATH
shutil.copy(config_file , work_path)

os.chdir( work_path )
if not os.path.isdir("_static"):
    os.mkdir("_static")

subprocess.call(["sphinx-apidoc" , "-e" , "-o" , "API/python" , PYTHONPATH ])
subprocess.call(["sphinx-build" , "-b" , "html" , "-d" , "_build/doctrees" , "." , "_build"])
