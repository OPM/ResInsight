from ert_gui.ide.keywords.data import Keyword, Argument, Token
from ert_gui.ide.keywords.definitions import KeywordDefinition, StringArgument
from ert.test import ExtendedTestCase


class TokenTest(ExtendedTestCase):
    def test_token(self):
        line = "some words in a line"
        token = Token(5, 10, line)

        self.assertEqual(token.value(), "words")
        self.assertEqual(token.fromIndex(), 5)
        self.assertEqual(token.toIndex(), 10)
        self.assertEqual(token.count(), 5)
        self.assertTrue(5 in token)
        self.assertTrue(9 in token)
        self.assertTrue(not 10 in token)


    def test_empty_token(self):
        token = Token(4, 4, "text")

        self.assertEqual(token.value(), "")

    def test_keyword(self):
        num_realizations = "NUM_REALIZATIONS"

        keyword_def = KeywordDefinition(num_realizations)

        keyword = Keyword(0, 16, num_realizations)
        keyword.setKeywordDefinition(keyword_def)

        self.assertEqual(keyword.value(), num_realizations)
        self.assertEqual(keyword_def, keyword.keywordDefinition())
        self.assertTrue(keyword.hasKeywordDefinition())


    def test_argument(self):
        text = "KEYWORD arg1 arg2"

        arg1 = Argument(8, 12, text)
        arg1.setArgumentDefinition(StringArgument())

        arg2 = Argument(13, 17, text)

        self.assertEqual(arg1.value(), "arg1")
        self.assertEqual(arg2.value(), "arg2")

        self.assertEqual(arg2.line(), text)

        self.assertTrue(arg1.hasArgumentDefinition())
        self.assertIsInstance(arg1.argumentDefinition(), StringArgument)

        self.assertFalse(arg2.hasArgumentDefinition())
        self.assertIsNone(arg2.argumentDefinition())





