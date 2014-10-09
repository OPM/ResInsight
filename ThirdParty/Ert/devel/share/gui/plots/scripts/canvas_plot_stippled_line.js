// Copyright (C) 2014 Statoil ASA, Norway.
//
// The file 'canvas_plot_stippled_line.js' is part of ERT - Ensemble based Reservoir Tool.
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

function CanvasPlotStippledLine() {
    var X = function (d) { return d; };
    var Y = function (d) { return d; };
    var style = STYLES["default"];
    var stipple_length = 2;

    var dashing = true;
    var unfinished_pixels_from_previous = 0;


    function render(context, x_samples, y_samples) {
        context.lineWidth = style["stroke_width"];
        context.strokeStyle = style["stroke"];
        context.lineCap = style["line_cap"];
        dashing = true;
        unfinished_pixels_from_previous = 0;

        var length = Math.min(x_samples.length, y_samples.length);

        context.beginPath();
        var previous_x = 0;
        var previous_y = 0;
        for (var index = 0; index < length; index++) {
            var x_sample = x_samples[index];
            var y_sample = y_samples[index];
            var x = X(x_sample);
            var y = Y(y_sample);

            if (index > 0) {
                render.dashedLineFromTo(context, previous_x, previous_y, x, y);
            }
            previous_x = x;
            previous_y = y;
        }

        context.stroke();
    }

    render.dashedLineFromTo = function(context, x1, y1, x2, y2) {
        var x = x1;
        var y = y1;
        var dash_length = stipple_length;
        var dx = (x2 - x) + 0.00000001;
        var dy = (y2 - y);
        var slope = dy / dx;
        var distance_remaining = Math.sqrt(dx * dx + dy * dy);
        var unfinished_pixels = false;
        var current_dash_length;
        var x_step;

        context.moveTo(x, y);

        while(distance_remaining >= 0.1) {
            if(unfinished_pixels_from_previous === 0) {
                current_dash_length = dash_length;
            } else {
                current_dash_length = unfinished_pixels_from_previous;
                unfinished_pixels_from_previous = 0;
                dashing = !dashing;
            }

            if(dash_length > distance_remaining) {
                dash_length = distance_remaining;
                unfinished_pixels = true;
            }

            x_step = Math.sqrt(current_dash_length * current_dash_length / (1 + slope * slope));
            x += x_step;
            y += slope * x_step;

            if(dashing) {
                context.lineTo(x, y);
            } else {
                context.moveTo(x, y);
            }
            distance_remaining -= current_dash_length;
            dashing = !dashing;
        }

        if(unfinished_pixels) {
            unfinished_pixels_from_previous = current_dash_length;
        }


    };

    render.x = function (value) {
        if (!arguments.length) return X;
        X = value;
        return render;
    };

    render.y = function (value) {
        if (!arguments.length) return Y;
        Y = value;
        return render;
    };

    render.style = function (value) {
        if (!arguments.length) return style;
        style = value;
        return render;
    };

    render.stippleLength = function (value) {
        if (!arguments.length) return stipple_length;
        stipple_length = value;
        return render;
    };


    return render;
}