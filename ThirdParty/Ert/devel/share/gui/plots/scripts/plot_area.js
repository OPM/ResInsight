// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'plot_area.js' is part of ERT - Ensemble based Reservoir Tool.
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


function PlotArea() {
    var x = function(d) { return d[0];};
    var y_min = function(d) { return d[1];};
    var y_max = function(d) { return d[1];};
    var duration = 250;
    var plot_style = "";


    function plot(selection) {
        var area = d3.svg.area().x(x).y0(y_min).y1(y_max).interpolate("basis");

        selection.transition()
            .duration(duration)
            .attr("d", area);

        selection.enter()
            .append("path")
            .attr("class", plot_style)
            .attr("d", area)
            .style("opacity", 0.0)
            .transition()
            .duration(duration)
            .style("opacity", 1.0);

        selection.exit()
            .transition()
            .duration(duration)
            .style("opacity", 0.0)
            .remove();

    }


    plot.duration = function(value) {
        if (!arguments.length) return duration;
        duration = value;
        return plot;
    };

    plot.x = function(value) {
        if (!arguments.length) return x;
        x = value;
        return plot;
    };

    plot.y_min = function(value) {
        if (!arguments.length) return y_min;
        y_min = value;
        return plot;
    };

    plot.y_max = function(value) {
        if (!arguments.length) return y_max;
        y_max = value;
        return plot;
    };

    plot.style = function(value) {
        if (!arguments.length) return plot_style;
        plot_style = value;
        return plot;
    };

    return plot;

}