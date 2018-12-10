import os

try:
    from unittest2 import TestLoader, TextTestRunner
except ImportError:
    from unittest import TestLoader, TextTestRunner


class ErtTestRunner(object):

    @staticmethod
    def runTestSuite(tests , test_verbosity = 3):
        test_runner = TextTestRunner(verbosity=test_verbosity)
        result = test_runner.run(tests)

        return result.wasSuccessful()


    @staticmethod
    def findTestsInDirectory(path, recursive=True , pattern = "test*.py"):
        loader = TestLoader()
        test_suite = loader.discover(path , pattern = pattern)
        
        for (root, dirnames, filenames) in os.walk( path ):
            for directory in dirnames:
                test_suite.addTests(ErtTestRunner.findTestsInDirectory(os.path.join(root, directory), recursive , pattern))

        return test_suite


    @staticmethod
    def runTestsInDirectory(path=".", recursive=True, test_verbosity=3):
        test_suite = ErtTestRunner.findTestsInDirectory(path, recursive)
        return ErtTestRunner.runTestSuite(test_suite)
        

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
        class_name = classpath[dot + 1:]
        try:
            m = __import__(classpath[0:dot], globals(), locals(), [class_name])
            return getattr(m, class_name)
        except ImportError:
            print("Failed to import: %s" % classpath)
            raise


    @staticmethod
    def getTestsFromTestClass(test_class_path, argv=None):
        klass = ErtTestRunner.importClass(test_class_path)
        klass.argv = argv
        loader = TestLoader()
        return loader.loadTestsFromTestCase(klass)
