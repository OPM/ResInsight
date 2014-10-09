// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'canvas_plot_area.js' is part of ERT - Ensemble based Reservoir Tool.
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


function CanvasPlotArea() {
    var x = function(d) { return d;};
    var y = function(d) { return d;};
    var style = STYLES["default"];

    function render(context, x_samples, y_samples) {
        context.lineWidth = style["stroke_width"];
        context.strokeStyle = style["stroke"];
        context.fillStyle = style["fill"];

        context.beginPath();
        for(var index in x_samples) {
            var x_sample = x_samples[index];
            var y_sample = y_samples[index];
            if(index == 0) {
                context.moveTo(x(x_sample), y(y_sample));
            } else {
                context.lineTo(x(x_sample), y(y_sample));
            }
        }
        context.closePath();
        context.stroke();
        context.fill();
    }

    render.x = function(value) {
        if (!arguments.length) return x;
        x = value;
        return render;
    };

    render.y = function(value) {
        if (!arguments.length) return y;
        y = value;
        return render;
    };

    render.style = function(value) {
        if (!arguments.length) return style;
        style = value;
        return render;
    };

    return render;
}

//append list_2 reversed to list_1 as a new list
CanvasPlotArea.mergePoints = function(list_1, list_2) {
    var result = [];

    for (var j = 0; j < list_1.length; j++) {
        result.push(list_1[j]);
    }

    for (var k = list_2.length - 1; k >= 0; k--) {
        result.push(list_2[k]);
    }

    return result;
};