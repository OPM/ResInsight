import re
from PyQt4.QtCore import Qt, QEvent, QDir, QRegExp, QChar
from PyQt4.QtGui import QPlainTextEdit, QTextOption, QCompleter, QStringListModel, QFont, QColor, QShortcut, QKeySequence, QTextCursor, QFileSystemModel, QTextDocument
from ert_gui.tools import HelpCenter


class IdePanel(QPlainTextEdit):
    def __init__(self):
        QPlainTextEdit.__init__(self)
        self.setWordWrapMode(QTextOption.NoWrap)
        self.setFont(QFont("monospace", 10))
        self.setCursorWidth(2)
        self.installEventFilter(self)

        self.cursorPositionChanged.connect(self.showHelp)

        self.completer = QCompleter(self)
        self.completer.setWidget(self)
        self.completer.setCaseSensitivity(Qt.CaseInsensitive)
        self.completer.activated.connect(self.insertCompletion)


        auto_complete = QShortcut(QKeySequence("Ctrl+Space"), self)
        auto_complete.activated.connect(self.activateCompleter)

        copy_line = QShortcut(QKeySequence("Ctrl+D"), self)
        copy_line.activated.connect(self.duplicateLine)

        select_fragment = QShortcut(QKeySequence("Ctrl+J"), self)
        select_fragment.activated.connect(self.selectFragment)

    def showHelp(self):
        text_cursor = self.textCursor()
        user_data = text_cursor.block().userData()

        if user_data is not None:
            configuration_line = user_data.configuration_line

            if configuration_line.keyword().hasKeywordDefinition():
                HelpCenter.getHelpCenter("ERT").setHelpMessageLink("config/" + configuration_line.documentationLink())


    def getText(self):
        return self.document().toPlainText()


    def eventFilter(self, qobject, qevent):
        if qobject == self and qevent.type() == QEvent.ToolTip:
            text_cursor = self.cursorForPosition(qevent.pos())
            pos = text_cursor.positionInBlock()

            user_data = text_cursor.block().userData()
            if user_data is not None:
                #: :type: ConfigurationLine
                configuration_line = user_data.configuration_line
                # if configuration_line.keyword().hasKeywordDefinition():
                #     print(configuration_line.keyword().keywordDefinition().documentation)

                if pos in configuration_line.keyword():
                    self.setToolTip(configuration_line.validationStatusForToken(configuration_line.keyword()).message())
                else:
                    for argument in configuration_line.arguments():
                        if pos in argument:
                            self.setToolTip(configuration_line.validationStatusForToken(argument).message())

            else:
                self.setToolTip("")


        return QPlainTextEdit.eventFilter(self, qobject, qevent)


    def activateCompleter(self):
        text_cursor = self.textCursor()
        block = self.document().findBlock(text_cursor.position())
        position_in_block = text_cursor.positionInBlock()



        self.selectWordUnderCursor(text_cursor)
        word = unicode(text_cursor.selectedText())

        user_data = block.userData()

        self.completer.setCompletionPrefix(word)

        show_completer = False
        if user_data is None:
            self.completer.setModel(QStringListModel(self.handler_names))
            show_completer = True

        else:
            keyword = user_data.keyword
            options = keyword.handler.parameterOptions(keyword, word, position_in_block)

            if len(options) == 1:
                self.insertCompletion(options[0])
            elif len(options) > 1:
                self.completer.setModel(QStringListModel(options))
                if self.completer.completionCount() == 1:
                    self.insertCompletion(self.completer.currentCompletion())
                else:
                    show_completer = True


        if show_completer:
            rect = self.cursorRect(text_cursor)
            rect.setWidth(self.completer.popup().sizeHintForColumn(0) + self.completer.popup().verticalScrollBar().sizeHint().width())
            self.completer.complete(rect)

    def keyPressEvent(self, qkeyevent):
        if self.completer.popup().isVisible():
            dead_keys = [Qt.Key_Enter, Qt.Key_Return, Qt.Key_Escape, Qt.Key_Tab, Qt.Key_Backtab]
            if qkeyevent.key() in dead_keys:
                qkeyevent.ignore()
                return

        if qkeyevent.modifiers() == Qt.ShiftModifier:
            if qkeyevent.key() & Qt.Key_Delete == Qt.Key_Delete:
                self.deleteLine()

        QPlainTextEdit.keyPressEvent(self, qkeyevent)


    def insertCompletion(self, string):
        text_cursor = self.textCursor()
        self.selectWordUnderCursor(text_cursor)
        text_cursor.insertText(string)

    def isCursorInSpace(self):
        text_cursor = self.textCursor()
        if text_cursor.positionInBlock() > 0:
            text_cursor.movePosition(QTextCursor.Left, QTextCursor.MoveAnchor)
            text_cursor.movePosition(QTextCursor.Right, QTextCursor.KeepAnchor)

        if text_cursor.positionInBlock() < text_cursor.block().length() - 1:
            text_cursor.movePosition(QTextCursor.Right, QTextCursor.KeepAnchor)

        if unicode(text_cursor.selectedText()).strip() == "":
            return True

        return False

    def selectWordUnderCursor(self, text_cursor):
        if not self.isCursorInSpace():
            # text_cursor.select(QTextCursor.WordUnderCursor)

            # pattern = "[\s|\v|\f|\n|\r|\t|\xe2\x80\xa8|\xe2\x80\xa9]"
            # pattern = "[\\s|\\xe2\\x80\\xa9|\\xe2\\x80\\xa8]"

            block_start = 0
            block_end = text_cursor.block().length()

            cursor_pos = text_cursor.positionInBlock()
            pos = cursor_pos
            pattern = u"[\s\u2029\u2028]"
            while pos >= block_start:
                text_cursor.movePosition(QTextCursor.Left, QTextCursor.KeepAnchor)
                text = text_cursor.selectedText()
                if re.search(pattern, text):
                    break
                pos -= 1

            text_cursor.movePosition(QTextCursor.Right, QTextCursor.MoveAnchor)

            while pos < block_end:
                text_cursor.movePosition(QTextCursor.Right, QTextCursor.KeepAnchor)
                text = text_cursor.selectedText()
                if re.search(pattern, text):
                    break
                pos += 1

            text_cursor.movePosition(QTextCursor.Left, QTextCursor.KeepAnchor)

            # pattern = "[\\s]"
            # start = self.document().find(QRegExp(pattern), text_cursor, QTextDocument.FindBackward | QTextDocument.FindCaseSensitively)
            # end = self.document().find(QRegExp(pattern), text_cursor, QTextDocument.FindCaseSensitively)
            # block_end_pos = text_cursor.block().position() + text_cursor.block().length()
            #
            # text_cursor.setPosition(start.position(), QTextCursor.MoveAnchor)
            # # text_cursor.setPosition(min(block_end_pos, end.position() - 1), QTextCursor.KeepAnchor)
            # text_cursor.setPosition(end.position() - 1, QTextCursor.KeepAnchor)


    def deleteLine(self):
        text_cursor = self.textCursor()
        text_cursor.beginEditBlock()
        text_cursor.select(QTextCursor.LineUnderCursor)
        text_cursor.removeSelectedText()
        text_cursor.deletePreviousChar()

        text_cursor.movePosition(QTextCursor.NextBlock)
        text_cursor.movePosition(QTextCursor.StartOfLine)
        self.setTextCursor(text_cursor)
        text_cursor.endEditBlock()

    def duplicateLine(self):
        text_cursor = self.textCursor()
        text_cursor.beginEditBlock()
        text_cursor.select(QTextCursor.LineUnderCursor)
        text = text_cursor.selectedText()
        text_cursor.movePosition(QTextCursor.EndOfLine)
        text_cursor.insertBlock()
        text_cursor.insertText(text)
        text_cursor.endEditBlock()


    def selectFragment(self):
        text_cursor = self.textCursor()
        self.selectWordUnderCursor(text_cursor)
        self.setTextCursor(text_cursor)
