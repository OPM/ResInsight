from ert_gui.ide.keywords.definitions import ArgumentDefinition, KeywordDefinition, ConfigurationLineDefinition, IntegerArgument
from ert.test import ExtendedTestCase





class ConfigurationLineDefinitionTest(ExtendedTestCase):

    def test_keyword_definition(self):
        keyword = KeywordDefinition("KEYWORD")
        self.assertEqual(keyword.name(), "KEYWORD")


    def test_argument_definition(self):
        arg_def = ArgumentDefinition(optional=True, built_in=True, rest_of_line=True)

        self.assertTrue(arg_def.isBuiltIn())
        self.assertTrue(arg_def.isOptional())
        self.assertTrue(arg_def.consumeRestOfLine())


    def test_configuration_line_definition(self):

        cld = ConfigurationLineDefinition(keyword=KeywordDefinition("KEYWORD"),
                                          arguments=[IntegerArgument(from_value=1)],
                                          documentation_link="help/path",
                                          required=True,
                                          group="Group")

        self.assertTrue(cld.isRequired())
        self.assertEqual(cld.documentationLink(), "help/path")

        keyword_definition = cld.keywordDefinition()
        self.assertIsInstance(keyword_definition, KeywordDefinition)
        self.assertEqual(keyword_definition.name(), "KEYWORD")

        argument_definitions = cld.argumentDefinitions()
        self.assertIsInstance(argument_definitions, list)
        self.assertEqual(len(argument_definitions), 1)
        self.assertIsInstance(argument_definitions[0], IntegerArgument)


        self.assertEqual(cld.group(), "Group")

