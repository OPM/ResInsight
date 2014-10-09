var STYLES = {
    default: {
        stroke: "rgba(0, 0, 0, 1.0)",
        fill: "rgba(200, 200, 200, 1.0)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },
    observation: {
        stroke: "rgba(0, 0, 0, 1.0)",
        fill: "rgba(0, 0, 0, 0.0)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },
    observation_error_bar: {
        stroke: "rgba(0, 0, 0, 1.0)",
        fill: "rgba(0, 0, 0, 0.0)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },
    observation_area: {
        stroke: "rgba(0, 0, 0, 0.15)",
        fill: "rgba(0, 0, 0, 0.2)",
        stroke_width: 2,
        dash_array: [],
        line_cap: "butt"
    },
    refcase: {
        stroke: "rgba(0, 0, 0, 0.7)",
        fill: "rgba(0, 0, 0, 0.0)",
        stroke_width: 1.5,
        dash_array: [],
        line_cap: "butt"
    },
    ensemble_1: {
        stroke: "rgba(56, 108, 176, 0.8)",
        fill: "rgba(56, 108, 176, 0.5)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },
    ensemble_2: {
        stroke: "rgba(127, 201, 127, 0.8)",
        fill: "rgba(127, 201, 127, 0.5)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },
    ensemble_3: {
        stroke: "rgba(253, 192, 134, 0.8)",
        fill: "rgba(253, 192, 134, 0.5)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },
    ensemble_4: {
        stroke: "rgba(240, 2, 127, 0.8)",
        fill: "rgba(240, 2, 127, 0.5)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },
    ensemble_5: {
        stroke: "rgba(191, 91, 23, 0.8)",
        fill: "rgba(191, 91, 23, 0.5)",
        stroke_width: 1,
        dash_array: [],
        line_cap: "butt"
    },

    ensemble_colors: ["ensemble_1", "ensemble_2", "ensemble_3", "ensemble_4", "ensemble_5"]

};


STYLES.parseColor = function(input) {
    var result = [255, 255, 255, 1];

    var match = input.match(/^rgba\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+\.?\d*)\s*\)$/i);
    if(match) {
        result[0] = match[1];
        result[1] = match[2];
        result[2] = match[3];
        result[3] = parseFloat(match[4]);
        return result;
    }

    return result;
};


STYLES.asRgb = function(r, g, b) {
    return "rgb(" + r + "," + g + "," + b + ")";
};

STYLES.asRgba = function(r, g, b, a) {
    return "rgba(" + r + "," + g + "," + b + "," + a + ")";
};


STYLES.componentToHex = function(c) {
    var hex = c.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
};

STYLES.rgbToHex = function(r, g, b) {
    return "#" + CanvasPlotLegend.componentToHex(r) + CanvasPlotLegend.componentToHex(g) + CanvasPlotLegend.componentToHex(b);
};

STYLES.darker = function(color) {
    var rgba = STYLES.parseColor(color);

    var f = 0.80;
    var a = rgba[3];
    var r = parseInt(rgba[0] * f);
    var g = parseInt(rgba[1] * f);
    var b = parseInt(rgba[2] * f);

    return STYLES.asRgba(r, g, b, a);
};

STYLES.blendWithWhite = function(color, result_alpha) {
    var rgba = STYLES.parseColor(color);

    var a = rgba[3];
    var ab = (1 - rgba[3]) * 255;
    var r = parseInt(rgba[0] * a + ab);
    var g = parseInt(rgba[1] * a + ab);
    var b = parseInt(rgba[2] * a + ab);

    return STYLES.asRgba(r, g, b, result_alpha);
};

STYLES.createFillColor = function(color, fill_alpha, blend_with_white_alpha) {
    var rgba = STYLES.parseColor(color);
    rgba[3] = fill_alpha;
    return STYLES.blendWithWhite(STYLES.asRgba(rgba[0], rgba[1], rgba[2], rgba[3]), blend_with_white_alpha);
};


STYLES.updateColors = function(settings) {
    var alpha = 0.7;
    var fill_alpha = 0.5;
    if("observation" in settings) {
        STYLES["observation"]["stroke"] = settings["observation"];
    }

    if("observation_error_bar" in settings) {
        STYLES["observation_error_bar"]["stroke"] = settings["observation_error_bar"];
    }

    if("observation_area" in settings) {
        STYLES["observation_area"]["stroke"] = settings["observation_area"];
        STYLES["observation_area"]["fill"] = settings["observation_area"];
    }

    if("refcase" in settings) {
        STYLES["refcase"]["stroke"] = settings["refcase"];
    }

    if("ensemble_1" in settings) {
        STYLES["ensemble_1"]["stroke"] = settings["ensemble_1"];
//        STYLES["ensemble_1"]["fill"] = settings["ensemble_1"];
        STYLES["ensemble_1"]["fill"] = STYLES.createFillColor(settings["ensemble_1"], fill_alpha, alpha);
    }

    if("ensemble_2" in settings) {
        STYLES["ensemble_2"]["stroke"] = settings["ensemble_2"];
        STYLES["ensemble_2"]["fill"] = STYLES.createFillColor(settings["ensemble_2"], fill_alpha, alpha);
//        STYLES["ensemble_2"]["fill"] = settings["ensemble_2"];
    }

    if("ensemble_3" in settings) {
        STYLES["ensemble_3"]["stroke"] = settings["ensemble_3"];
        STYLES["ensemble_3"]["fill"] = STYLES.createFillColor(settings["ensemble_3"], fill_alpha, alpha);
//        STYLES["ensemble_3"]["fill"] = settings["ensemble_3"];
    }

    if("ensemble_4" in settings) {
        STYLES["ensemble_4"]["stroke"] = settings["ensemble_4"];
        STYLES["ensemble_4"]["fill"] = STYLES.createFillColor(settings["ensemble_4"], fill_alpha, alpha);
//        STYLES["ensemble_4"]["fill"] = settings["ensemble_4"];
    }

    if("ensemble_5" in settings) {
        STYLES["ensemble_5"]["stroke"] = settings["ensemble_5"];
        STYLES["ensemble_5"]["fill"] = STYLES.createFillColor(settings["ensemble_5"], fill_alpha, alpha);
//        STYLES["ensemble_5"]["fill"] = settings["ensemble_5"];
    }

};


var alpha = 0.7;
STYLES["ensemble_1"]["fill"] = STYLES.blendWithWhite(STYLES["ensemble_1"]["fill"], alpha);
STYLES["ensemble_2"]["fill"] = STYLES.blendWithWhite(STYLES["ensemble_2"]["fill"], alpha);
STYLES["ensemble_3"]["fill"] = STYLES.blendWithWhite(STYLES["ensemble_3"]["fill"], alpha);
STYLES["ensemble_4"]["fill"] = STYLES.blendWithWhite(STYLES["ensemble_4"]["fill"], alpha);
STYLES["ensemble_5"]["fill"] = STYLES.blendWithWhite(STYLES["ensemble_5"]["fill"], alpha);