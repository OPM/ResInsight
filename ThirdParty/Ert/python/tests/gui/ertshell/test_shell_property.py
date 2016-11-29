from ert.test.extended_testcase import ExtendedTestCase
from ert_gui.shell.libshell import ShellProperty, boolValidator, createListValidator, createFloatValidator


class TestModel(object):
    STATUSES = ["OK", "Error", "Unknown"]

    def __init__(self, name="TestModel"):
        self.__enabled = True
        self.__status = "OK"
        self.__name = name
        self.__description = None
        self.__threshold = 0.5
        self.__priority = 1

    def setEnabled(self, enabled):
        self.__enabled = enabled

    def isEnabled(self):
        return self.__enabled

    def setStatus(self, status):
        self.__status = status

    def status(self):
        return self.__status

    def statusCompleter(self, text, line, begidx, endidx):
        def test(status, text):
            status = status.lower()
            text = text.lower()
            return status != self.status().lower() and status.startswith(text)
        return [status for status in TestModel.STATUSES if test(status, text)]

    def statusValidator(self, value):
        if value.lower() == self.status().lower():
            raise ValueError("Value can not be set to current value!")

        for status in TestModel.STATUSES:
            if value.lower() == str(status).lower():
                return status

        raise ValueError("Unknown value")


    def name(self):
        return self.__name

    def setDescription(self, description):
        self.__description = description

    def description(self):
        return self.__description

    @property
    def threshold(self):
        return self.__threshold

    @threshold.setter
    def threshold(self, threshold):
        self.__threshold = max(0.0, min(threshold, 1.0))

    @property
    def priority(self):
        return self.__priority

    @priority.setter
    def priority(self, priority):
        try:
            priority = int(priority)
            self.__priority = priority
        except ValueError as error:
            self.__priority = 0


class TestTarget(object):
    def __init__(self, model, name="DefaultTestTarget"):
        self.__model = model
        self.__name = name
        self.__failed = False

    def lastCommandFailed(self, message):
        print("[%s] Error: %s" % (self.__name, message))
        self.__failed = True

    def didLastCommandFail(self):
        return self.__failed

    def resetFailedFlag(self):
        self.__failed = False

    def getModelForProperty(self, property_name):
        return self.__model


def completeMock(line):
    last_space_index = line.rfind(" ")
    begin_index = 0 if last_space_index == -1 else last_space_index + 1
    end_index = len(line)
    text = line[begin_index:end_index]

    return text, line, begin_index, end_index


class ShellPropertyTest(ExtendedTestCase):
    def setUp(self):
        self.properties = {
            "enabled": ("enabled", TestModel.isEnabled, TestModel.setEnabled),
            "status": ("status", TestModel.status, TestModel.setStatus),
            "priority": ("priority", TestModel.priority, TestModel.priority),
            "name": ("name", TestModel.name, None),
            "description": ("description", TestModel.description, TestModel.setDescription),
            "threshold": ("threshold", TestModel.threshold, TestModel.threshold)
        }

        self.completers = {
            "enabled": ["true", "false"],
            "status": TestModel.statusCompleter,
            "threshold": None,
            "priority": [1, 2, 3, 4, 5],
            "name": None,
            "description": None
        }

        self.validators = {
            "enabled": boolValidator,
            "status": TestModel.statusValidator,
            "threshold": createFloatValidator(0.0, 1.0),
            "priority": createListValidator(self.completers["priority"]),
            "name": None,
            "description": None
        }

    def test_creation_and_check_existence_of_do_complete_help(self):
        target = TestTarget(TestModel())
        for key, (name, getter, setter) in self.properties.items():
            shell_property = ShellProperty(name, getter, setter)
            shell_property.setParent(target)

            self.assertTrue(hasattr(target, "do_%s" % name))
            self.assertTrue(hasattr(target, "complete_%s" % name))
            self.assertTrue(hasattr(target, "help_tuple_%s" % name))


    def test_addition_of_help(self):
        target = TestTarget(TestModel())

        for key, (name, getter, setter) in self.properties.items():
            help_arguments = "%s args" % name
            help_message = "%s msg" % name
            shell_property = ShellProperty(name, getter, setter, help_arguments=help_arguments, help_message=help_message)
            shell_property.setParent(target)

            help_function = getattr(target, "help_tuple_%s" % name)

            cmd_name, args, msg = help_function()
            self.assertEqual(cmd_name, name)
            self.assertEqual(args, help_arguments)
            self.assertEqual(msg, help_message)
            self.assertFalse(target.didLastCommandFail())


    def test_no_parent(self):
        target = TestTarget(TestModel())
        for key, (name, getter, setter) in self.properties.items():
            shell_property = ShellProperty(name, getter, setter)

            self.assertFalse(hasattr(target, "do_%s" % name))
            self.assertFalse(hasattr(target, "complete_%s" % name))
            self.assertFalse(hasattr(target, "help_tuple_%s" % name))

            with self.assertRaises(AttributeError):
                shell_property.doFunction("")

    def test_duplicate_properties(self):
        target = TestTarget(TestModel())
        shell_property = ShellProperty("test",  str, None)
        shell_property.setParent(target)

        with self.assertRaises(ValueError):
            shell_property = ShellProperty("test", str, None)
            shell_property.setParent(target)



    def test_read_only_property(self):
        property_name = "name"
        tests = [
            ("", "TestModel", False),
            ("new_name", "TestModel", True),
        ]

        completions = [
            ("", []),
        ]

        self.runPropertyTests(property_name, tests, completions)


    def test_bool_property(self):
        property_name = "enabled"
        tests = [
            ("", True, False),
            ("False", False, False),
            ("g", False, True),
            ("yes", True, False),
        ]

        completions = [
            ("", self.completers[property_name]),
            ("f", ["false"]),
            ("t", ["true"]),
            ("q", [])
        ]

        self.runPropertyTests(property_name, tests, completions)

    def test_string_property(self):
        property_name = "description"
        tests = [
            ("", None, False),
            ("a description", "a description", False),
            ("1234", "1234", False),
        ]

        completions = [
            ("", []),
            ("q", []),
        ]

        self.runPropertyTests(property_name, tests, completions)


    def test_choice_property(self):
        property_name = "status"
        tests = [
            ("", "OK", False),
            ("ERROR", "Error", False),
            ("DING", "Error", True),
        ]

        completions = [
            ("", ["OK", "Unknown"]),
            ("e", []),
            ("u", ["Unknown"]),
            ("q", [])
        ]

        self.runPropertyTests(property_name, tests, completions)


    def test_float_property(self):
        property_name = "threshold"
        tests = [
            ("", 0.5, False),
            ("0.7", 0.7, False),
            ("1.1", 1.0, False),
            ("0", 0.0, False),
            ("zero", 0.0, True)
        ]

        completions = [
            ("", []),
            ("1", []),
            ("e", [])
        ]

        self.runPropertyTests(property_name, tests, completions)


    def test_int_list_property(self):
        property_name = "priority"
        tests = [
            ("", 1, False),
            ("2", 2, False),
            ("6", 2, True),
        ]

        completions = [
            ("", self.completers[property_name]),
            ("5", [5]),
            ("e", [])
        ]

        self.runPropertyTests(property_name, tests, completions)


    def runPropertyTests(self, property_name, tests, completions):
        model = TestModel()
        target = TestTarget(model)

        name, getter, setter = self.properties[property_name]
        shell_property = ShellProperty(name, getter, setter, self.validators[property_name], self.completers[property_name])
        shell_property.setParent(target)

        doFunc = getattr(target, "do_%s" % property_name)
        for do_input, expected_value, fail in tests:
            doFunc(do_input)

            if isinstance(getter, property):
                model_value = getter.__get__(model)
            else:
                model_value = getter(model)

            self.assertEqual(expected_value, model_value)

            if fail:
                self.assertTrue(target.didLastCommandFail())
            else:
                self.assertFalse(target.didLastCommandFail())

            target.resetFailedFlag()

        completeFunc = getattr(target, "complete_%s" % property_name)
        for complete_input, expected_value in completions:
            choices = completeFunc(*completeMock(complete_input))
            self.assertListEqual(choices, expected_value)