#!/usr/bin/env python
import os
import sys
import traceback

class ImportTester(object):
    @staticmethod
    def testImport(module):
        try:
            if '__pycache__' in str(module):
                # python 3 hack
                return True
            else:
                print('Importing module %s.' % str(module))
            __import__(module)
            return True
        except ImportError:
            tb = traceback.format_exc()
            sys.stderr.write("Error importing module %s!\n\n" % module)
            sys.stderr.write(str(tb))
            sys.stderr.write("\n")
        except Exception:
            tb = traceback.format_exc()
            sys.stderr.write("Import of module %s caused errors!\n\n" % module)
            sys.stderr.write(str(tb))
            sys.stderr.write("\n")

        return False


    @staticmethod
    def importRecursively(path, package_name):
        entries = os.listdir(path)

        result = True

        for entry in sorted(entries):
            import_success = True

            entry_path = os.path.join(path, entry)
            if os.path.isdir(entry_path):
                package = "%s.%s" % (package_name, entry)
                import_success = ImportTester.testImport(package)
                new_path = os.path.join(path, entry)
                import_success = import_success and ImportTester.importRecursively(new_path, package)
            elif os.path.isfile(entry_path):
                if not entry.startswith("__init__") and entry.endswith(".py"):
                    module = entry[0:len(entry) - 3]
                    import_success = ImportTester.testImport("%s.%s" % (package_name, module))
            else:
                # skip other files
                pass
                # print("Skipped entry: %s" % entry)

            if not import_success:
                result = False

        return result


if __name__ == '__main__':
    PYTHONPATH = sys.argv[1]
    package_name = sys.argv[2]

    sys.path.insert(0, PYTHONPATH)

    package_path = os.path.join(PYTHONPATH, package_name)

    if ImportTester.importRecursively(package_path, package_name):
        sys.exit(0)
    else:
        sys.exit(1)
