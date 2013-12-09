import os
import sys
from ert_tests.import_tester import ImportTester

if __name__ == '__main__':
    PYTHONPATH = sys.argv[1]
    package_name = sys.argv[2]

    sys.path.insert(0, PYTHONPATH)

    package_path = os.path.join(PYTHONPATH, package_name)

    if ImportTester.importRecursively(package_path, package_name):
        sys.exit(0)
    else:
        sys.exit(1)
