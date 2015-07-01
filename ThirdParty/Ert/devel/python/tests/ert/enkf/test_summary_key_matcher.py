from ert.enkf import SummaryKeyMatcher
from ert.test import ExtendedTestCase

class SummaryKeyMatcherTest(ExtendedTestCase):

    def test_creation(self):
        matcher = SummaryKeyMatcher()

        self.assertTrue(len(matcher) == 0)

        matcher.addSummaryKey("F*")
        self.assertTrue(len(matcher) == 1)

        matcher.addSummaryKey("F*")
        self.assertTrue(len(matcher) == 1)

        matcher.addSummaryKey("FOPT")
        self.assertTrue(len(matcher) == 2)

        self.assertItemsEqual(["F*", "FOPT"], matcher.keys())

        self.assertTrue("FGIR" in matcher)
        self.assertTrue("FOPT" in matcher)
        self.assertFalse("TCPU" in matcher)

        self.assertTrue(matcher.isRequired("FOPT"))
        self.assertFalse(matcher.isRequired("FGIR"))
        self.assertFalse(matcher.isRequired("TCPU"))