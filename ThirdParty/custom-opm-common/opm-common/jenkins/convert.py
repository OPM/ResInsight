#!/usr/bin/env python
# coding: utf-8
# originally from:
#  http://www.warp1337.com/content/how-use-ctest-jenkins-xunit-or-junit-plugin
# improved by:
#  Jorge Araya Navarro <elcorreo@deshackra.com>

#                         Veni, Sancte Spiritus.

from lxml import etree
import argparse
from os.path import expanduser
from os.path import join
import logging

# configure logging
logging.basicConfig(format="%(levelname)s: %(message)s",
                    level=logging.ERROR)

desc = ("Converts ctest XML file to xUnit/JUnit XML "
        "compatible file to use with Jenkins-CI. "
        "Did you found any bug? please report it on: "
        "https://bitbucket.org/shackra/ctest-jenkins/issues")

# configure argument parser.
parser = argparse.ArgumentParser(description=desc)
parser.add_argument("-x", "--xslt", help="the XSLT file to use", required=True)
parser.add_argument("-t", "--tag", help=("the directory where 'Testing/TAG'"
                                         "file is. Remember to call ctest with"
                                         " '-T test' option to generate it"),
                    required=True)

parsed = parser.parse_args()
# expanding user symbol "~"
parsed.xsl = expanduser(parsed.xslt)
parsed.tag = expanduser(parsed.tag)

# opening the TAG file
directory = None
try:
    with open(join(parsed.tag, "Testing", "TAG")) as tagfile:
        directory = tagfile.readline().strip()

except NotADirectoryError:
    logging.error(
        "'Testing/TAG' wasn't found on directory '{}'.".format(parsed.tag))
    exit(1)
except FileNotFoundError:
    logging.error(
        "File '{}' not found.".format(join(parsed.tag, "Testing", "TAG")))
    exit(1)

xmldoc = None
transform = None
try:
    with open(join(parsed.tag, "Testing", directory, "Test.xml"))\
            as testxmlfile:
        xmldoc = etree.parse(testxmlfile)

except FileNotFoundError:
    logging.error("File {} not found. Was it deleted or moved?".format(
        join(parsed.tag, "Testing", directory, "Test.xml")))
    exit(1)

try:
    with open(parsed.xslt) as xsltfile:
        xslt_root = etree.XML(xsltfile.read())
        transform = etree.XSLT(xslt_root)
except FileNotFoundError:
    logging.error("File {} not found.".format(parsed.xslt))
    exit(1)

result_tree = transform(xmldoc)
print(result_tree)
