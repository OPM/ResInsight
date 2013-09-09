from unittest2 import TestLoader, TextTestRunner


def runTestsInDirectory(path="."):
    loader = TestLoader()
    tests = loader.discover(path)
    testRunner = TextTestRunner()
    testRunner.run(tests)


def runTestsInClass(classpath):
    klass = importClass(classpath)
    loader = TestLoader()
    tests = loader.loadTestsFromTestCase(klass)
    testRunner = TextTestRunner()
    testRunner.run(tests)


def importClass(classpath):
    dot = classpath.rfind(".")
    classname = classpath[dot + 1:len(classpath)]
    m = __import__(classpath[0:dot], globals(), locals(), [classname])
    return getattr(m, classname)

def getTestsFromTestClass(test_class_path, argv=None):
    klass = importClass(test_class_path)
    klass.argv = argv
    loader = TestLoader()
    return loader.loadTestsFromTestCase(klass)


if __name__ == '__main__':
    # runTestsInDirectory()
    runTestsInClass("ert_tests.util.test_string_list.StringListTest")

    print(getTestsFromTestClass("ert_tests.util.test_string_list.StringListTest"))
