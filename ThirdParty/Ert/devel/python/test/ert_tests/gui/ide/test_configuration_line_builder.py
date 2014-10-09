from ert_gui.ide.keywords import ErtKeywords
from ert_gui.ide.keywords.configuration_line_builder import ConfigurationLineBuilder
from ert_gui.ide.keywords.definitions import ArgumentDefinition
from ert_gui.ide.keywords.data.configuration_line import ConfigurationLine
from ert.test import ExtendedTestCase


class ConfigurationLineBuilderTest(ExtendedTestCase):



    def test_num_realizations(self):
        keywords = ErtKeywords()
        clb = ConfigurationLineBuilder(keywords)

        test_line = "NUM_REALIZATIONS 25"

        clb.processLine(test_line)

        self.assertTrue(clb.hasConfigurationLine())
        self.assertFalse(clb.hasComment())
        self.assertEqual(clb.commentIndex(), -1)

        config_line = clb.configurationLine()

        self.assertEqual(config_line.keyword().value(), "NUM_REALIZATIONS")
        self.assertEqual(config_line.keyword().keywordDefinition(), keywords["NUM_REALIZATIONS"].keywordDefinition())
        self.assertEqual(config_line.arguments()[0].value(), "25")

    def test_num_realizations_no_argument(self):
        keywords = ErtKeywords()
        clb = ConfigurationLineBuilder(keywords)

        test_line = "NUM_REALIZATIONS"

        clb.processLine(test_line)

        self.assertTrue(clb.hasConfigurationLine())

        config_line = clb.configurationLine()

        self.assertEqual(config_line.keyword().value(), "NUM_REALIZATIONS")
        self.assertEqual(config_line.keyword().keywordDefinition(), keywords["NUM_REALIZATIONS"].keywordDefinition())

        arguments = config_line.arguments()
        self.assertEqual(len(arguments), 1)

        self.assertFalse(config_line.validationStatusForToken(config_line.keyword()))
        self.assertFalse(config_line.validationStatusForToken(arguments[0]))


    def test_unknown_keyword_with_comment(self):
        keywords = ErtKeywords()
        clb = ConfigurationLineBuilder(keywords)

        test_line = "KEYWORD nothing --comment"

        clb.processLine(test_line)

        self.assertTrue(clb.hasConfigurationLine())

        config_line = clb.configurationLine()
        keyword = config_line.keyword()
        self.assertFalse(config_line.validationStatusForToken(keyword))
        message = ConfigurationLine.UNKNOWN_KEYWORD + '\n' + ConfigurationLine.ARGUMENT_ERROR + '\n' + ConfigurationLine.ARGUMENT_NOT_EXPECTED
        self.assertEqual(config_line.validationStatusForToken(keyword).message(), message)

        self.assertEqual(keyword.value(), "KEYWORD")
        arguments = config_line.arguments()

        self.assertEqual(arguments[0].value(), "nothing")
        self.assertIsNone(keyword.keywordDefinition())

        self.assertTrue(clb.hasComment())
        self.assertEqual(clb.commentIndex(), 16)

        self.assertFalse(config_line.validationStatusForToken(arguments[0]))
        self.assertEqual(config_line.validationStatusForToken(arguments[0]).message(), ConfigurationLine.ARGUMENT_NOT_EXPECTED)


    def test_queue_option_keyword(self):
        keywords = ErtKeywords()
        clb = ConfigurationLineBuilder(keywords)

        test_line = "QUEUE_OPTION LSF LSF_BJOBS_CMD STRING AND STRING"

        clb.processLine(test_line)
        config_line = clb.configurationLine()

        arguments = config_line.arguments()

        self.assertEqual(len(arguments), 3)
        self.assertEqual(arguments[0].value(), "LSF")
        self.assertEqual(arguments[1].value(), "LSF_BJOBS_CMD")
        self.assertEqual(arguments[2].value(), "STRING AND STRING")

        print(config_line.validationStatusForToken(config_line.keyword()))

        self.assertTrue(config_line.validationStatusForToken(arguments[0]))
        self.assertTrue(config_line.validationStatusForToken(arguments[1]))
        self.assertTrue(config_line.validationStatusForToken(arguments[2]))


    def test_queue_option_keyword_too_few_arguments(self):
        keywords = ErtKeywords()
        clb = ConfigurationLineBuilder(keywords)

        test_line = "QUEUE_OPTION LSF LSF_BJOBS_CMD"

        clb.processLine(test_line)
        config_line = clb.configurationLine()

        arguments = config_line.arguments()

        self.assertEqual(len(arguments), 3)

        self.assertEqual(arguments[0].value(), "LSF")
        self.assertEqual(arguments[1].value(), "LSF_BJOBS_CMD")
        self.assertEqual(arguments[2].value(), "")

        self.assertFalse(config_line.validationStatusForToken(config_line.keyword()))

        self.assertTrue(config_line.validationStatusForToken(arguments[0]))
        self.assertTrue(config_line.validationStatusForToken(arguments[1]))
        self.assertFalse(config_line.validationStatusForToken(arguments[2]))

        self.assertEqual(config_line.validationStatusForToken(arguments[2]).message(), ArgumentDefinition.MISSING_ARGUMENT)



