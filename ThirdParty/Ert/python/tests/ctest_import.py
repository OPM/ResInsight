#!/usr/bin/env python
import os
import sys

PYTHONPATH = sys.argv[1]
sys.path.insert(0, PYTHONPATH)

from import_tester import ImportTester

package_name = sys.argv[2]
package_path = os.path.join(PYTHONPATH, package_name)

if ImportTester.importRecursively(package_path, package_name):
    sys.exit(0)
else:
    sys.exit(1)
