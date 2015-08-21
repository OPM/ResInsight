from ert_gui.ide.keywords.data import ConfigurationLine, Keyword, Argument
from ert_gui.ide.keywords.definitions import StringArgument
from ert_gui.ide.keywords.definitions.keyword_definition import KeywordDefinition
from ert.test import ExtendedTestCase


class ConfigurationLineTest(ExtendedTestCase):

    def test_configuration_line_creation(self):
        line = "KEYWORD arg1"
        keyword = Keyword(0, 7, line)
        argument = Argument(8, 12, line)
        cl = ConfigurationLine(keyword=keyword,
                               arguments=[argument],
                               documentation_link="help/link",
                               group="group",
                               required=True)

        self.assertEqual(cl.keyword(), keyword)
        self.assertEqual(cl.arguments()[0], argument)

        self.assertEqual(cl.documentationLink(), "help/link")
        self.assertEqual(cl.group(), "group")
        self.assertTrue(cl.isRequired())

        self.assertFalse(cl.validationStatusForToken(keyword))
        self.assertFalse(cl.validationStatusForToken(argument))



    def test_configuration_line(self):

        keyword_def = KeywordDefinition("KEYWORD")
        arg1_def = StringArgument()

        line = "KEYWORD string 2"
        keyword = Keyword(0, 7, line)
        self.assertEqual(keyword.value(), "KEYWORD")

        keyword.setKeywordDefinition(keyword_def)

        arg1 = Argument(8, 14, line)
        self.assertEqual(arg1.value(), "string")
        arg1.setArgumentDefinition(arg1_def)

        arg2 = Argument(15, 16, line)
        self.assertEqual(arg2.value(), "2")


        cl = ConfigurationLine(keyword=keyword,
                               arguments=[arg1, arg2],
                               documentation_link="help",
                               group="test_group",
                               required=True)

        self.assertTrue(cl.keyword().hasKeywordDefinition())
        self.assertEqual(cl.keyword().keywordDefinition(), keyword_def)

        self.assertEqual(len(cl.arguments()), 2)

        self.assertEqual(cl.arguments()[0], arg1)
        self.assertTrue(cl.arguments()[0].hasArgumentDefinition())
        self.assertEqual(cl.arguments()[0].argumentDefinition(), arg1_def)

        self.assertEqual(cl.arguments()[1], arg2)
        self.assertFalse(cl.arguments()[1].hasArgumentDefinition())
        self.assertIsNone(cl.arguments()[1].argumentDefinition())

        self.assertFalse(cl.validationStatusForToken(keyword))
        self.assertTrue(cl.validationStatusForToken(arg1))
        self.assertFalse(cl.validationStatusForToken(arg2))

