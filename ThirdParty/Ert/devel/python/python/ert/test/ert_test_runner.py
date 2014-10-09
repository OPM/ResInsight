import os

try:
    from unittest2 import TestLoader, TextTestRunner
except ImportError:
    from unittest import TestLoader, TextTestRunner


class ErtTestRunner(object):

    @staticmethod
    def findTestsInDirectory(path, recursive=True):
        loader = TestLoader()
        tests = loader.discover(path)

        for (root, dirnames, filenames) in os.walk( path ):
            for directory in dirnames:
                tests.addTests(ErtTestRunner.findTestsInDirectory(os.path.join(root, directory), recursive))

        return tests


    @staticmethod
    def runTestsInDirectory(path=".", recursive=True, test_verbosity=3):
        tests = ErtTestRunner.findTestsInDirectory(path, recursive)
        if tests.countTestCases() > 0:
            print("Running %d tests in %s" % (tests.countTestCases(), path))

        test_runner = TextTestRunner(verbosity=test_verbosity)
        result = test_runner.run(tests)

        return result.wasSuccessful()


    @staticmethod
    def runTestsInClass(classpath, test_verbosity=3):
        klass = ErtTestRunner.importClass(classpath)
        loader = TestLoader()
        tests = loader.loadTestsFromTestCase(klass)
        testRunner = TextTestRunner(verbosity=test_verbosity)
        testRunner.run(tests)


    @staticmethod
    def importClass(classpath):
        dot = classpath.rfind(".")
        class_name = classpath[dot + 1:len(classpath)]
        m = __import__(classpath[0:dot], globals(), locals(), [class_name])
        return getattr(m, class_name)


    @staticmethod
    def getTestsFromTestClass(test_class_path, argv=None):
        klass = ErtTestRunner.importClass(test_class_path)
        klass.argv = argv
        loader = TestLoader()
        return loader.loadTestsFromTestCase(klass)
