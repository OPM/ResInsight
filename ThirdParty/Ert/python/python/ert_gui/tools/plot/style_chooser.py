from PyQt4.QtGui import QWidget, QHBoxLayout, QComboBox, QDoubleSpinBox, QLabel, QHBoxLayout

from ert_gui.plottery import PlotStyle


STYLE_OFF = ("Off", None)
STYLE_AREA = ("Area", "#")
STYLE_SOLID = ("Solid", "-")
STYLE_DASHED = ("Dashed", "--")
STYLE_DOTTED = ("Dotted", ":")
STYLE_DASH_DOTTED = ("Dash Dotted", "-.")

STYLESET_DEFAULT = 'default'
STYLESET_AREA = 'area'
STYLESET_TOGGLE = 'toggle_only'

STYLES = {
    STYLESET_DEFAULT:  [STYLE_OFF, STYLE_SOLID, STYLE_DASHED, STYLE_DOTTED, STYLE_DASH_DOTTED],
    STYLESET_AREA:     [STYLE_OFF, STYLE_AREA, STYLE_SOLID, STYLE_DASHED, STYLE_DOTTED, STYLE_DASH_DOTTED],
    STYLESET_TOGGLE:   [STYLE_OFF, STYLE_SOLID]
}

MARKER_OFF = ("Off", None)
MARKER_X = ("X", "x")
MARKER_CIRCLE = ("Circle", "o")
MARKER_POINT = ("Point", ".")
MARKER_PIXEL = ("Pixel", ",")
MARKER_PLUS = ("Plus", "+")
MARKER_STAR = ("Star", "*")
MARKER_DIAMOND = ("Diamond", "D")
MARKER_PENTAGON = ("Pentagon", "p")
MARKER_SQUARE = ("Square", "s")
MARKER_HLINE = ("H Line", "_")
MARKER_VLINE = ("V Line", "|")
MARKER_OCTAGON = ("Octagon", "8")
MARKER_HEXAGON1 = ("Hexagon 1", "h")
MARKER_HEXAGON2 = ("Hexagon 2", "H")

MARKERS = [MARKER_OFF, MARKER_X, MARKER_CIRCLE, MARKER_POINT, MARKER_STAR, MARKER_DIAMOND, MARKER_PLUS, MARKER_PENTAGON, MARKER_SQUARE, MARKER_OCTAGON, MARKER_HEXAGON1, MARKER_HEXAGON2]


class StyleChooser(QWidget):

    def __init__(self, line_style_set=STYLESET_DEFAULT):
        QWidget.__init__(self)
        self._style = PlotStyle("StyleChooser Internal Style")

        self._styles = STYLES['default'] if not line_style_set in STYLES else STYLES[line_style_set]

        self.setMinimumWidth(140)
        self.setMaximumHeight(25)

        layout = QHBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(2)

        self.line_chooser = QComboBox()
        self.line_chooser.setToolTip("Select line style.")
        for style in self._styles:
            self.line_chooser.addItem(*style)

        self.marker_chooser = QComboBox()
        self.marker_chooser.setToolTip("Select marker style.")
        for marker in MARKERS:
            self.marker_chooser.addItem(*marker)

        self.thickness_spinner = QDoubleSpinBox()
        self.thickness_spinner.setToolTip("Line thickness")
        self.thickness_spinner.setMinimum(0.1)
        self.thickness_spinner.setDecimals(1)
        self.thickness_spinner.setSingleStep(0.1)

        self.size_spinner = QDoubleSpinBox()
        self.size_spinner.setToolTip("Marker Size")
        self.size_spinner.setMinimum(0.1)
        self.size_spinner.setDecimals(1)
        self.size_spinner.setSingleStep(0.1)

        # the text content of the spinner varies, but shouldn't push the control out of boundaries
        self.line_chooser.setMinimumWidth(110)
        layout.addWidget(self.line_chooser)
        layout.addWidget(self.thickness_spinner)
        layout.addWidget(self.marker_chooser)
        layout.addWidget(self.size_spinner)

        self.setLayout(layout)

        self.line_chooser.currentIndexChanged.connect(self._updateStyle)
        self.marker_chooser.currentIndexChanged.connect(self._updateStyle)
        self.thickness_spinner.valueChanged.connect(self._updateStyle)
        self.size_spinner.valueChanged.connect(self._updateStyle)

        self._updateLineStyleAndMarker(self._style.line_style, self._style.marker, self._style.width, self._style.size)
        self._layout = layout

    def getItemSizes(self):
        line_style_combo_width = self._layout.itemAt(0).sizeHint().width()
        thickness_spinner_width = self._layout.itemAt(1).sizeHint().width()
        marker_combo_width = self._layout.itemAt(2).sizeHint().width()
        size_spinner_width = self._layout.itemAt(3).sizeHint().width()
        return line_style_combo_width, thickness_spinner_width, marker_combo_width, size_spinner_width

    def _findLineStyleIndex(self, line_style):
        for index, style in enumerate(self._styles):
            if style[1] == line_style:
                return index
            elif style[1] is None and line_style == "":
                return index
        return -1

    def _findMarkerStyleIndex(self, marker):
        for index, style in enumerate(MARKERS):
            if style[1] == marker:
                return index
            elif style[1] is None and marker == "":
                return index
        return -1

    def _updateLineStyleAndMarker(self, line_style, marker, thickness, size):
        self.line_chooser.setCurrentIndex(self._findLineStyleIndex(line_style))
        self.marker_chooser.setCurrentIndex(self._findMarkerStyleIndex(marker))
        self.thickness_spinner.setValue(thickness)
        self.size_spinner.setValue(size)

    def _updateStyle(self):
        self.marker_chooser.setEnabled(self.line_chooser.currentText() != "Area")

        line_style = self.line_chooser.itemData(self.line_chooser.currentIndex())
        marker_style = self.marker_chooser.itemData(self.marker_chooser.currentIndex())
        thickness = float(self.thickness_spinner.value())
        size = float(self.size_spinner.value())

        self._style.line_style = str(line_style.toString())
        self._style.marker = str(marker_style.toString())
        self._style.width = thickness
        self._style.size = size

    def setStyle(self, style):
        """ @type style: PlotStyle """
        self._style.copyStyleFrom(style)
        self._updateLineStyleAndMarker(style.line_style, style.marker, style.width, style.size)

    def getStyle(self):
        """ @rtype: PlotStyle """
        style = PlotStyle("Generated Style from StyleChooser")
        style.copyStyleFrom(self._style)
        return style

    def createLabelLayout(self, layout=None):
        if layout is None:
            layout = QHBoxLayout()

        titles = ["Line Style", "Width", "Marker Style", "Size"]
        sizes = self.getItemSizes()
        for title, size in zip(titles, sizes):
            label = QLabel(title)
            label.setFixedWidth(size)
            layout.addWidget(label)

        return layout

