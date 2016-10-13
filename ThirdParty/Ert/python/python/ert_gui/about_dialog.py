#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

from PyQt4.QtCore import Qt, QSize
from PyQt4.QtGui import  QDialog, QVBoxLayout, QPushButton, QHBoxLayout, QLabel, QFont
from ert.util import Version
from ert_gui.ertwidgets import resourceImage


class AboutDialog(QDialog):

    def __init__(self, parent):
        QDialog.__init__(self, parent)

        self.setWindowTitle("About")
        self.setModal(True)
        self.setFixedSize(QSize(600, 480))
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowCloseButtonHint)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowCancelButtonHint)

        main_layout = QVBoxLayout()

        main_layout.addLayout(self.createTopLayout())
        main_layout.addLayout(self.createGplLayout())
        main_layout.addLayout(self.createButtonLayout())

        self.setLayout(main_layout)


    def createTopLayout(self):
        top_layout = QHBoxLayout()

        image_label = QLabel()
        image = resourceImage("splash.jpg")
        image_label.setPixmap(image.scaled(200, 240, Qt.KeepAspectRatio))

        top_layout.addWidget(image_label)

        top_layout.addLayout(self.createInfoLayout(), 1)

        return top_layout

    @staticmethod
    def createInfoLayout():
        info_layout = QVBoxLayout()

        ert = QLabel()
        ert.setAlignment(Qt.AlignHCenter)

        title_font = QFont()
        title_font.setPointSize(40)
        ert.setFont(title_font)
        ert.setText("ERT")

        info_layout.addWidget(ert)
        info_layout.addStretch(1)
        ert_title = QLabel()
        ert_title.setAlignment(Qt.AlignHCenter)
        ert_title.setText("Ensemble based Reservoir Tool")
        info_layout.addWidget(ert_title)

        version = QLabel()
        version.setAlignment(Qt.AlignHCenter)
        version.setText("Version: %s" % Version.getVersion())
        info_layout.addWidget(version)

        timestamp = QLabel()
        timestamp.setAlignment(Qt.AlignHCenter)
        timestamp.setText("Build time: %s" % Version.getBuildTime())
        info_layout.addWidget(timestamp)

        git_commit = QLabel()
        git_commit.setAlignment(Qt.AlignHCenter)
        git_commit.setText("Git commit hash: %s" % Version.getGitCommit(short=True))
        info_layout.addWidget(git_commit)

        info_layout.addStretch(5)

        return info_layout


    def createGplLayout(self):
        gpl = QLabel()
        gpl.setText("ERT is free software: you can redistribute it and/or modify \
          it under the terms of the GNU General Public License as published by \
          the Free Software Foundation, either version 3 of the License, or \
          (at your option) any later version. <br> <br>\
           \
          ERT is distributed in the hope that it will be useful, but WITHOUT ANY \
          WARRANTY; without even the implied warranty of MERCHANTABILITY or \
          FITNESS FOR A PARTICULAR PURPOSE.  <br> <br>\
          \
          See the GNU General Public License at <a href=\"http://www.gnu.org/licenses/gpl.html\">www.gnu.org</a> for more details. ")
        gpl.setWordWrap(True)
        gpl_layout = QVBoxLayout()
        gpl_layout.addWidget(gpl)
        return gpl_layout

    def createButtonLayout(self):
        button_layout = QHBoxLayout()

        close_button = QPushButton("Close")
        close_button.clicked.connect(self.accept)

        button_layout.addStretch()
        button_layout.addWidget(close_button)
        button_layout.addStretch()

        return button_layout
