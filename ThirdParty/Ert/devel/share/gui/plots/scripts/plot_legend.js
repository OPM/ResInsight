// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'plot_legend.js' is part of ERT - Ensemble based Reservoir Tool.
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


function PlotLegend() {

    var size = 12;

    function legend(selection) {
        var groups = selection.enter()
            .append("div")
            .attr("class", "plot-legend");


        groups.append("svg")
            .attr("width", size + "px")
            .attr("height", size + "px")
            .append("rect")
            .attr("width",  (size - 2) + "px")
            .attr("height", (size - 2) + "px")
            .attr("transform", "translate(1,1)")
            .attr("class", function(d) {
                return "legend-marker " + d["style"];
            });


        selection.selectAll(".legend-marker")
            .data(function(d) {return [d];})
            .transition()
            .attr("class", function(d) {
                return "legend-marker " + d["style"];
            });

        groups.append("div")
            .attr("class", "plot-legend-label")
            .text(function(d) {
                return d["name"];
            });

        var labels = selection.selectAll(".plot-legend-label")
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
    return legend;

}