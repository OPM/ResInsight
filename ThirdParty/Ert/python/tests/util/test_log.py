from ecl.util.enums import MessageLevelEnum

from ecl.test import ExtendedTestCase

class LogTest(ExtendedTestCase):

    def test_enums(self):
        self.assertEnumIsFullyDefined(MessageLevelEnum, "message_level_type", "lib/include/ert/util/log.h")
