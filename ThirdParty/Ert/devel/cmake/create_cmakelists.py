#!/usr/bin/env python

from os import listdir
from os.path import isfile, join, isdir, islink
import sys



def findFilesAndDirectories(directory):
    all_files = listdir(directory)
    files = []
    directories = []
    for f in all_files:
        path = join(directory, f)
        if isfile(path) and not f == "CMakeLists.txt" and not islink(path):
            files.append(f)
        if isdir(path):
            directories.append(f)

    return sorted(files), sorted(directories)


def findRelativeModulePath(directory):
    """@type directory: str"""
    index = directory.rfind("python/")
    index += len("python/")
    return directory[index:len(directory)]

def createPythonSources(files):
    result = ""

    if len(files) > 0:
        result = "set(PYTHON_SOURCES\n"

    files = [f for f in files if f.endswith(".py")]

    for f in files:
        result += "    " + str(f) + "\n"

    if len(files) > 0:
        result += ")"

    return result

def addSubDirectories(directories):
    result = ""

    for d in directories:
        result += "add_subdirectory(" + str(d) + ")\n"

    return result

def addPythonPackage(relative_module_path):
    module_name = ".".join(relative_module_path.split("/"))
    template = "add_python_package(\"python.%s\"  ${PYTHON_INSTALL_PREFIX}/%s \"${PYTHON_SOURCES}\" True)"

    return template % (module_name, relative_module_path)

def addInclude(filename):
    with open(filename, "r") as include_file:
        content = include_file.read()
    return content

files, directories = findFilesAndDirectories(sys.argv[1])
module_path = findRelativeModulePath(sys.argv[1])

output_file = join(sys.argv[1], "CMakeLists.txt")
with open(output_file, "w+") as text_file:
    text_file.write(createPythonSources(files))
    text_file.write("\n\n")
    text_file.write(addPythonPackage(module_path))
    text_file.write("\n\n")
    text_file.write(addSubDirectories(directories))

    if "local.cmake" in files:
        text_file.write("\n\n")
        text_file.write(addInclude(join(sys.argv[1], "local.cmake")))




