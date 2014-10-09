from ert_gui.ide.keywords import ConfigurationLineParser
from ert_gui.ide.keywords.data import Argument
from ert.test import ExtendedTestCase


class ConfigurationLineParserTest(ExtendedTestCase):


    def test_comments(self):
        keyword_parser = ConfigurationLineParser()

        test_line = "-- comment"
        keyword_parser.parseLine(test_line)

        self.assertTrue(keyword_parser.hasComment())
        self.assertEqual(keyword_parser.commentIndex(), 0)
        self.assertFalse(keyword_parser.hasKeyword())
        self.assertIsNone(keyword_parser.keyword())
        self.assertEqual(keyword_parser.uncommentedText(), "")


        test_line = "     -- comment"
        keyword_parser.parseLine(test_line)

        self.assertTrue(keyword_parser.hasComment())
        self.assertEqual(keyword_parser.commentIndex(), 5)
        self.assertFalse(keyword_parser.hasKeyword())
        self.assertIsNone(keyword_parser.keyword())
        self.assertEqual(keyword_parser.uncommentedText(), "     ")

        test_line = "NUM_REALIZATIONS-- comment"
        keyword_parser.parseLine(test_line)

        self.assertTrue(keyword_parser.hasComment())
        self.assertEqual(keyword_parser.commentIndex(), 16)
        self.assertTrue(keyword_parser.hasKeyword())
        self.assertEqual(keyword_parser.keyword().value(), "NUM_REALIZATIONS")
        self.assertEqual(keyword_parser.uncommentedText(), "NUM_REALIZATIONS")

        test_line = "NUM_REALIZATIONS -- comment"
        keyword_parser.parseLine(test_line)

        self.assertTrue(keyword_parser.hasComment())
        self.assertEqual(keyword_parser.commentIndex(), 17)
        self.assertTrue(keyword_parser.hasKeyword())
        self.assertEqual(keyword_parser.keyword().value(), "NUM_REALIZATIONS")
        self.assertEqual(keyword_parser.uncommentedText(), "NUM_REALIZATIONS ")


    def test_argument_text(self):
        keyword_parser = ConfigurationLineParser()

        test_line = "NUM_REALIZATIONS 25"
        keyword_parser.parseLine(test_line)

        self.assertFalse(keyword_parser.hasComment())
        self.assertEqual(keyword_parser.commentIndex(), -1)
        self.assertTrue(keyword_parser.hasKeyword())
        self.assertEqual(keyword_parser.keyword().value(), "NUM_REALIZATIONS")
        self.assertEqual(keyword_parser.uncommentedText(), "NUM_REALIZATIONS 25")
        self.assertEqual(keyword_parser.argumentsText(), " 25")


        test_line = "NUM_REALIZATIONS 25--comment"
        keyword_parser.parseLine(test_line)

        self.assertTrue(keyword_parser.hasComment())
        self.assertEqual(keyword_parser.commentIndex(), 19)
        self.assertTrue(keyword_parser.hasKeyword())
        self.assertEqual(keyword_parser.keyword().value(), "NUM_REALIZATIONS")
        self.assertEqual(keyword_parser.uncommentedText(), "NUM_REALIZATIONS 25")
        self.assertEqual(keyword_parser.argumentsText(), " 25")

        test_line = "NUM_REALIZATIONS 25 something_else"
        keyword_parser.parseLine(test_line)

        self.assertTrue(keyword_parser.hasKeyword())
        self.assertEqual(keyword_parser.keyword().value(), "NUM_REALIZATIONS")
        self.assertEqual(keyword_parser.uncommentedText(), "NUM_REALIZATIONS 25 something_else")
        self.assertEqual(keyword_parser.argumentsText(), " 25 something_else")




    def test_argument_list(self):
        keyword_parser = ConfigurationLineParser()

        test_line = "KEYWORD arg1 arg2"
        keyword_parser.parseLine(test_line)

        keyword = keyword_parser.keyword()
        self.assertEqual(keyword.value(), "KEYWORD")

        arguments = keyword_parser.arguments()
        self.assertEqual(arguments[0].value(), "arg1")
        self.assertEqual(arguments[1].value(), "arg2")


