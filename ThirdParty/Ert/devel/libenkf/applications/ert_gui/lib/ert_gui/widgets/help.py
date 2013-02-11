#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'help.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import os
import sys

# The variable @help_prefix should be set to point to a directory
# containing (directories) with html help files. In the current
# implementation this variable is set from the gert_main.py script.
help_prefix = None


def getTemplate():
    path = help_prefix + "template.html"
    if os.path.exists(path) and os.path.isfile(path):
        f = open(path, 'r')
        template = f.read()
        f.close()
        return template
    else:
        return "<html>%s</html>"


def resolveHelpLabel(label):
    """
    Reads a HTML file from the help directory.
    The HTML must follow the specification allowed by QT here: http://doc.trolltech.com/4.6/richtext-html-subset.html
    """

    # This code can be used to find widgets with empty help labels
#    if label.strip() == "":
#        raise AssertionError("NOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!!")

    path = help_prefix + label + ".html"
    if os.path.exists(path) and os.path.isfile(path):
        f = open(path, 'r')
        help = f.read()
        f.close()
        return getTemplate() % help
    else:
        # This code automatically creates empty help files
#        sys.stderr.write("Missing help file: '%s'\n" % label)
#        if not label == "" and not label.find("/") == -1:
#            sys.stderr.write("Creating help file: '%s'\n" % label)
#            directory, filename = os.path.split(path)
#
#            if not os.path.exists(directory):
#                os.makedirs(directory)
#
#            file_object = open(path, "w")
#            file_object.write(label)
#            file_object.close()
        return ""
