// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'histogram_renderer.js' is part of ERT - Ensemble based Reservoir Tool.
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


function HistogramRenderer() {
    var margin = {top: 0, right: 0, bottom: 0, left: 0};
    var X = function (d) { return d; };
    var Y = function (d) { return d; };
    var style = STYLES["default"];


    function render(context, histogram_values) {
        context.lineWidth = "1";
        context.strokeStyle = "rgba(0, 0, 0, 0.5)";

        var fill = style["fill"];

        context.fillStyle = STYLES.blendWithWhite(fill, 1.0);

        for(var j = 0; j < histogram_values.length; j++) {
            var value = histogram_values[j];
            if(value.y > 0) {
                var x = X(value.x);
                var y = Y(value.y);
                var w = X(value.x + value.dx) - x;
                var h = Y(0) - y;

                context.fillRect(x + margin.left, y + margin.top, w - margin.left - margin.right, h - margin.top - margin.bottom);
                context.strokeRect(x + margin.left, y + margin.top, w - margin.left - margin.right, h - margin.top - margin.bottom);
            }
        }
    }

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

    render.margin = function (top, right, bottom, left) {
        if (!arguments.length) return margin;
        margin = {top: top, right: right, bottom: bottom, left: left};
        return render;
    };

    return render;
}
