from ert.test import ExtendedTestCase
from ert_gui.shell.libshell import ShellCollection


class MockParent(object):
    def __init__(self, name="DefaultMockParent"):
        self.__name = name
        self.__failed = False

    def shellContext(self):
        return self

    def shell(self):
        return self

    def columnize(self, items, size):
        print(items)

    def lastCommandFailed(self, message):
        print("[%s] Error: %s" % (self.__name, message))
        self.__failed = True

    def didLastCommandFail(self):
        failed = self.__failed
        self.__failed = False
        return failed


def completeMock(line):
    last_space_index = line.rfind(" ")
    begin_index = 0 if last_space_index == -1 else last_space_index + 1
    end_index = len(line)
    text = line[begin_index:end_index]

    return text, line, begin_index, end_index


class ShellCollectionTest(ExtendedTestCase):

    def test_creation(self):
        name = "test"
        shell_collection = ShellCollection(name)

        parent = MockParent()
        shell_collection.setParent(parent)

        self.assertEqual(shell_collection.name, name)

        self.assertTrue(hasattr(parent, "do_%s" % name))
        self.assertTrue(hasattr(parent, "complete_%s" % name))
        self.assertTrue(hasattr(parent, "help_%s" % name))


    def test_sub_collection(self):
        l1 = "level_1"
        l2 = "level_2"
        l3 = "level_3"

        root = MockParent()
        level_1 = ShellCollection(l1)
        level_1.setParent(root)
        level_2 = ShellCollection(l2)
        level_1.addCollection(level_2)
        level_3 = ShellCollection(l3)
        level_2.addCollection(level_3)

        tests = [(root, l1), (level_1, l2), (level_2, l3)]

        for parent, name in tests:
            self.assertTrue(hasattr(parent, "do_%s" % name))
            self.assertTrue(hasattr(parent, "complete_%s" % name))
            self.assertTrue(hasattr(parent, "help_%s" % name))

        root.do_level_1("level_2 ")
        self.assertFalse(root.didLastCommandFail())

        root.do_level_1("r ")
        self.assertTrue(root.didLastCommandFail())

        root.do_level_1(" level_2 level_3")
        self.assertFalse(root.didLastCommandFail())

        root.do_level_1("level_2 level_3 p")
        self.assertTrue(root.didLastCommandFail())

        result = root.complete_level_1(*completeMock("level_1 "))
        self.assertListEqual(result, [l2])

        result = root.complete_level_1(*completeMock("level_1 lev"))
        self.assertListEqual(result, [l2])

        result = root.complete_level_1(*completeMock("level_1 level_2 "))
        self.assertListEqual(result, [l3])

        root.help_level_1()
        level_1.help_level_2()
        level_2.help_level_3()
