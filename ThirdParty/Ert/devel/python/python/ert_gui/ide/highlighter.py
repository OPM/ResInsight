import re
from PyQt4.QtGui import QSyntaxHighlighter, QTextCharFormat, QColor, QTextBlockUserData

from ert_gui.ide.keywords import ErtKeywords
from ert_gui.ide.keywords.configuration_line_builder import ConfigurationLineBuilder
from ert_gui.ide.keywords.data import Keyword


class ConfigurationLineUserData(QTextBlockUserData):
    def __init__(self, configuration_line):
        QTextBlockUserData.__init__(self)
        self.configuration_line = configuration_line

class KeywordHighlighter(QSyntaxHighlighter):
    def __init__(self, document):
        QSyntaxHighlighter.__init__(self, document)

        self.clb = ConfigurationLineBuilder(ErtKeywords())


        self.comment_format = QTextCharFormat()
        self.comment_format.setForeground(QColor(0, 128, 0))
        self.comment_format.setFontItalic(True)

        self.keyword_format = QTextCharFormat()
        self.keyword_format.setForeground(QColor(200, 100, 0))
        # self.keyword_format.setFontWeight(QFont.Bold)

        self.error_format = QTextCharFormat()
        # self.error_format.setForeground(QColor(255, 0, 0))
        self.error_format.setUnderlineStyle(QTextCharFormat.WaveUnderline)
        self.error_format.setUnderlineColor(QColor(255, 0, 0))

        self.search_format = QTextCharFormat()
        self.search_format.setBackground(QColor(220, 220, 220))

        self.builtin_format = QTextCharFormat()
        self.builtin_format.setForeground(QColor(0, 170, 227))

        self.search_string = ""


    def formatKeyword(self, keyword, validation_status):
        assert isinstance(keyword, Keyword)
        if keyword.hasKeywordDefinition():
            keyword_format = QTextCharFormat(self.keyword_format)

            if not validation_status:
                keyword_format.merge(self.error_format)

            self.formatToken(keyword, keyword_format)
        else:
            self.formatToken(keyword, self.error_format)


    def highlightBlock(self, complete_block):
        block = unicode(complete_block)

        self.clb.processLine(block)


        if self.clb.hasComment():
            self.setFormat(self.clb.commentIndex(), len(block) - self.clb.commentIndex(), self.comment_format)

        if not self.clb.hasConfigurationLine():
            count = len(block)

            if self.clb.hasComment():
                count = self.clb.commentIndex()

            self.setFormat(0, count, self.error_format)


        if self.clb.hasConfigurationLine():
            cl = self.clb.configurationLine()
            self.setCurrentBlockUserData(ConfigurationLineUserData(cl))

            self.formatKeyword(cl.keyword(), cl.validationStatusForToken(cl.keyword()))

            arguments = cl.arguments()

            for argument in arguments:
                if not argument.hasArgumentDefinition():
                    pass

                elif argument.argumentDefinition().isBuiltIn():
                    self.formatToken(argument, self.builtin_format)

                if not cl.validationStatusForToken(argument):
                    self.formatToken(argument, self.error_format)


        if self.search_string != "":
            for match in re.finditer("(%s)" % self.search_string, complete_block):
                self.setFormat(match.start(1), match.end(1) - match.start(1), self.search_format)


    def setSearchString(self, string):
        if self.search_string != unicode(string):
            self.search_string = unicode(string)
            self.rehighlight()


    def formatToken(self, token, highlight_format):
        self.setFormat(token.fromIndex(), token.count(), highlight_format)