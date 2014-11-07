// Copyright (C) 2014 Statoil ASA, Norway.
//
// The file 'base_plot_value_dimension.js' is part of ERT - Ensemble based Reservoir Tool.
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

function BasePlotValueDimension(flip_range) {
    if (!arguments.length) {
        var flip_range = false;
    }

    var is_log_scale = false;
    var tick_count = 10;
    var ticks = null;
    var unit = "";

    var scale = d3.scale.linear().range([1, 0]).domain([0, 1]).nice();
    var log_scale = d3.scale.log().range([1, 0]).domain([0, 1]).nice();

    var value_format = d3.format(".4g");
//    var value_log_format = d3.format("e");
//
//    var value_log_format_function = function (d) {
//        var x = Math.log(d) / Math.log(10) + 1e-6;
//        return Math.abs(x - Math.floor(x)) < 0.3 ? value_log_format(d) : "";
//    };

    var scaler = function (d) {
        if (is_log_scale) {
            return log_scale(d);
        } else {
            return scale(d);
        }
    };

    function dimension(value) {
        return scaler(value);
    }

    function powerOfTen(d) {
        return d / Math.pow(10, Math.ceil(Math.log(d) / Math.LN10 - 1e-12)) === 1;
    }

    dimension.setDomain = function (min, max) {
        if (min == max) {
            min = min - 0.1 * min;
            max = max + 0.1 * max;
        }

        if (flip_range) {
            var tmp = min;
            min = max;
            max = tmp;
        }
        scale.domain([min, max]).nice();


        if(min <= 0.0) {
            min = 0.000000001;
        }

        if(max <= 0.0) {
            max = 0.000000016;
        }

        var from_log = Math.floor(Math.log(min) / Math.log(10));
        var to_log = Math.ceil(Math.log(max) / Math.log(10));
        var from = Math.pow(10, from_log);
        var to = Math.pow(10, to_log);
        log_scale.domain([from, to]);
    };

    dimension.setRange = function (min, max) {
        scale.range([min, max]).nice();
        log_scale.range([min, max]).nice();
    };

    dimension.scale = function () {
        if (is_log_scale) {
            return log_scale;
        } else {
            return scale;
        }
    };

    dimension.isOrdinal = function () {
        return false;
    };

    dimension.format = function (axis, max_length) {

        if (is_log_scale) {
            axis.tickValues(null)
                .tickPadding(10)
                .ticks(1)
                .tickSize(-max_length, -max_length);
//                .tickFormat(value_log_format_function);
        } else {
            axis.tickPadding(10)
                .tickSize(-max_length, -max_length)
                .tickFormat(value_format)
                .ticks(tick_count)
                .tickValues(ticks);
        }

        return dimension;
    };

    dimension.setIsLogScale = function (use_log_scale) {
        is_log_scale = use_log_scale;
    };

    dimension.isLogScale = function () {
        return is_log_scale;
    };

    dimension.relabel = function (axis) {
        if (is_log_scale) {
            axis.selectAll(".tick text")
                .text(null)
                .filter(powerOfTen)
                .text(function (d) {
                    return "1.0×10";
                })
                .append("tspan")
                .style("font-size", "70%")
                .attr("dy", "-.7em")
                .text(function (d) {
                    return Math.round(Math.log(d) / Math.LN10);
                });
        }
    };

    dimension.setTicks = function(tick_list) {
        ticks = tick_list;
    };

    dimension.setUnit = function(unit_in) {
        unit = unit_in;
    };

    dimension.getUnit = function() {
        return unit;
    };

    return dimension;
}
