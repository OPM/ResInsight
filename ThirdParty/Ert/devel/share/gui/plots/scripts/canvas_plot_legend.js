// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'canvas_plot_legend.js' is part of ERT - Ensemble based Reservoir Tool.
//
// ERT is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ERT is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.
//
// See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
// for more details.


function CanvasPlotLegend() {

    var size = 12;

    function legend(selection) {
        var groups = selection.enter()
            .append("div")
            .attr("class", "plot-legend");

        groups.append("canvas")
            .attr("width", size)
            .attr("height", size)
            .attr("class", "legend-marker")
            .call(markerRenderer);

        selection.selectAll(".legend-marker")
            .data(function(d) { return [d];})
            .transition()
            .call(markerRenderer);

        groups.append("div")
            .attr("class", "plot-legend-label")
            .text(function(d) {
                return d["name"];
            });

        selection.selectAll(".plot-legend-label")
            .data(function(d) {
                return [d];
            })
            .transition()
            .text(function(d) {
                return d["name"];
            });

        selection.exit()
            .remove();
    }

    var markerRenderer = function(selection) {
        selection.each(function(datum) {
            var ctx = d3.select(this).node().getContext("2d");
            ctx.clearRect(0, 0, size, size);
            var style = datum["style"];

            ctx.save();
            if("render_function" in datum) {
                ctx.transform(1, 0, 0, 1, 1, 1);
                datum["render_function"](ctx, style, size - 2, size - 2);
            } else {
                ctx.fillStyle = style["fill"];
                ctx.fillRect(1, 1, size - 2, size - 2);

                ctx.strokeStyle = style["stroke"];
                ctx.lineWidth = style["stroke_width"];
                ctx.strokeRect(1, 1, size - 2, size - 2);
            }
            ctx.restore();
        });
    };

    return legend;

}

CanvasPlotLegend.parseColor = function(input) {
    var result = [255, 255, 255, 1];

//        match = input.match(/^#([0-9a-f]{3})$/i);
//        if(match) {
//            match = match[1];
//            // in three-character format, each value is multiplied by 0x11 to give an
//            // even scale from 0x00 to 0xff
//            result[0] = parseInt(match.charAt(0), 16) * 0x11;
//            result[1] = parseInt(match.charAt(1), 16) * 0x11;
//            result[2] = parseInt(match.charAt(2), 16) * 0x11;
//            return result;
//        }
//
//        console.log("2");
//        match = input.match(/^#([0-9a-f]{6})$/i);
//        if(match) {
//            match = match[1];
//            result[0] = parseInt(match.substr(0,2), 16);
//            result[1] = parseInt(match.substr(2,4), 16);
//            result[2] = parseInt(match.substr(4,6), 16);
//            return result;
//        }
//
//        match = input.match(/^rgb\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)$/i);
//        if(match) {
//            result[0] = match[1];
//            result[1] = match[2];
//            result[2] = match[3];
//            return result;
//        }
//        if(input === undefined) {
//            console.log("UNDEFINED!");
//            return result;
//        }
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

CanvasPlotLegend.asRgb = function(r, g, b) {
    return "rgb(" + r + "," + g + "," + b + ")";
};

CanvasPlotLegend.asRgba = function(r, g, b, a) {
    return "rgba(" + r + "," + g + "," + b + "," + a + ")";
};


CanvasPlotLegend.componentToHex = function(c) {
    var hex = c.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
};

CanvasPlotLegend.rgbToHex = function(r, g, b) {
    return "#" + CanvasPlotLegend.componentToHex(r) + CanvasPlotLegend.componentToHex(g) + CanvasPlotLegend.componentToHex(b);
};

CanvasPlotLegend.circledLine = function(context, style, width, height) {
    context.strokeStyle = style["stroke"];
    context.lineWidth = style["stroke_width"];
    context.fillStyle = style["fill"];
    context.beginPath();
    context.moveTo(0, height / 2);
    context.lineTo(width, height / 2);
    context.stroke();

    context.beginPath();
    context.arc(width / 2, height / 2, 2.5, 0, 2 * Math.PI);
    context.fill();
    context.stroke();
};

CanvasPlotLegend.filledCircle = function(context, style, width, height) {
    context.strokeStyle = style["stroke"];
    context.lineWidth = style["stroke_width"];
    context.fillStyle = style["fill"];

    context.beginPath();
    context.arc(width / 2, height / 2, width / 2, 0, 2 * Math.PI);
    context.fill();
    context.stroke();
};

CanvasPlotLegend.cross = function(context, style, width, height) {
    context.strokeStyle = style["stroke"];
    context.lineWidth = style["stroke_width"];
    context.fillStyle = style["fill"];

    var cx = width / 2;
    var cy = height / 2;
    var radius = width / 2;

    context.beginPath();
    context.moveTo(cx - radius, cy - radius);
    context.lineTo(cx + radius, cy + radius);

    context.moveTo(cx + radius, cy - radius);
    context.lineTo(cx - radius, cy + radius);
    context.stroke();
};

CanvasPlotLegend.errorBar = function(context, style, width, height) {
    context.strokeStyle = style["stroke"];
    context.lineWidth = 1;
    context.fillStyle = style["fill"];
    var cx = width / 2;
    var cy = height / 2;
    var radius = 2;
    context.beginPath();
    context.moveTo(cx, cy - radius);
    context.lineTo(cx, 0);
    context.moveTo(cx - radius, 0);
    context.lineTo(cx + radius, 0);

    context.moveTo(cx, cy + radius);
    context.lineTo(cx, height);
    context.moveTo(cx - radius, height);
    context.lineTo(cx + radius, height);
    context.stroke();

    context.beginPath();
    context.arc(cx, cy, radius, 0, 2 * Math.PI);
    context.fill();
    context.stroke();
};

CanvasPlotLegend.simpleLine = function(context, style, width, height) {
    var color = style["stroke"];
    var rgba = CanvasPlotLegend.parseColor(color);
    context.strokeStyle = CanvasPlotLegend.asRgba(rgba[0], rgba[1], rgba[2], 1.0);
    context.lineWidth = style["stroke_width"] * 2;

    context.beginPath();
    context.moveTo(0, height / 2);
    context.lineTo(width, height / 2);
    context.stroke();
};

CanvasPlotLegend.stippledLine = function(context, style, width, height) {
    var line_renderer = CanvasPlotStippledLine();
    line_renderer.style(style);
    line_renderer(context, [0, width], [height / 2, height / 2]);
};

CanvasPlotLegend.filledCircleWithLine = function(context, style, width, height) {
    context.strokeStyle = style[0]["stroke"];
    context.lineWidth = style[0]["stroke_width"];
    context.fillStyle = style[0]["fill"];

    context.beginPath();
    context.arc(width / 2, height / 2, (width - 1) / 2, 0, 2 * Math.PI);
    context.fill();
    context.stroke();


    context.strokeStyle = style[1]["stroke"];
    context.lineWidth = style[1]["stroke_width"];
    context.fillStyle = style[1]["fill"];

    context.beginPath();
    context.moveTo(0, height / 2);
    context.lineTo(width, height / 2);
    context.stroke();
};