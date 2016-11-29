from ert.test import ExtendedTestCase
from ert_gui.shell.libshell import ShellFunction


class TestTarget(object):
    def __init__(self, name="DefaultTestTarget"):
        self.__name = name
        self.__failed = False

    def lastCommandFailed(self, message):
        print("[%s] Error: %s" % (self.__name, message))
        self.__failed = True

    def didLastCommandFail(self):
        return self.__failed

    def resetFailedFlag(self):
        self.__failed = False

    def getModelForFunction(self, function_name):
        if function_name == "test_function":
            return self
        else:
            return {"key": "value"}


def dummyFunction(model, line):
    print("[Dummy] Received line: %s" % line)


def completeMock(line):
    last_space_index = line.rfind(" ")
    begin_index = 0 if last_space_index == -1 else last_space_index + 1
    end_index = len(line)
    text = line[begin_index:end_index]

    return text, line, begin_index, end_index


class ShellFunctionTest(ExtendedTestCase):

    def test_creation(self):
        target = TestTarget()
        name = "test_function"
        shell_function = ShellFunction(name, dummyFunction, None)
        shell_function.setParent(target)

        self.assertTrue(hasattr(target, "do_%s" % name))
        self.assertTrue(hasattr(target, "complete_%s" % name))
        self.assertTrue(hasattr(target, "help_tuple_%s" % name))

    def test_no_parent(self):
        target = TestTarget()
        name = "test_function"
        shell_function = ShellFunction(name, dummyFunction, None)

        self.assertFalse(hasattr(target, "do_%s" % name))
        self.assertFalse(hasattr(target, "complete_%s" % name))
        self.assertFalse(hasattr(target, "help_tuple_%s" % name))

        with self.assertRaises(AttributeError):
            shell_function.doFunction("nonsense")


    def test_failed_creation(self):
        shell_function = ShellFunction("test_function", dummyFunction)
        with self.assertRaises(ValueError):
            shell_function.setParent(None)

        with self.assertRaises(ValueError):
            ShellFunction("test_function", None)


    def test_duplicate_function(self):
        target = TestTarget()
        shell_function = ShellFunction("duplicate", dummyFunction)
        shell_function.setParent(target)

        with self.assertRaises(ValueError):
            duplicate = ShellFunction("duplicate", dummyFunction)
            duplicate.setParent(target)

    def test_addition_of_help(self):
        target = TestTarget()

        name = "test_function"
        help_arguments = "%s args" % name
        help_message = "%s msg" % name
        shell_function = ShellFunction(name, str, None, help_arguments=help_arguments, help_message=help_message)
        shell_function.setParent(target)

        help_function = getattr(target, "help_tuple_%s" % name)

        cmd_name, args, msg = help_function()
        self.assertEqual(cmd_name, name)
        self.assertEqual(args, help_arguments)
        self.assertEqual(msg, help_message)
        self.assertFalse(target.didLastCommandFail())

    def test_function_with_target_as_model(self):
        target = TestTarget()

        def checkModel(model, line):
            self.assertIsInstance(model, TestTarget)
            self.assertEqual(line, "some text to parse")

        shell_function = ShellFunction("test_function", checkModel)
        shell_function.setParent(target)
        target.do_test_function("some text to parse")

    def test_function_with_custom_model(self):
        target = TestTarget()

        def checkModel(model, line):
            self.assertIsInstance(model, dict)
            self.assertTrue("key" in model)
            self.assertEqual(line, "some other text to parse")

        shell_function = ShellFunction("other_test_function", checkModel)
        shell_function.setParent(target)
        target.do_other_test_function("some other text to parse")

    def test_completion(self):
        target = TestTarget()

        def completer(model, text, line, begidx, endidx):
            text = text.strip().lower()

            if text == "s":
                return ["Large"]
            elif text == "l":
                return ["Small"]
            elif text == "m":
                return ["Medium"]
            else:
                return []

        shell_function = ShellFunction("test_function", dummyFunction, completer)
        shell_function.setParent(target)

        result = target.complete_test_function(*completeMock("s"))
        self.assertEqual(result, ["Large"])

        result = target.complete_test_function(*completeMock("l"))
        self.assertEqual(result, ["Small"])

        result = target.complete_test_function(*completeMock(""))
        self.assertEqual(result, [])
