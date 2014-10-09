// Copyright (C) 2014 Statoil ASA, Norway.
//
// The file 'base_plot_time_dimension.js' is part of ERT - Ensemble based Reservoir Tool.
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

function BasePlotIntegerDimension(){
    var scale = d3.time.scale().range([0, 1]).domain([1, 0]);
    var unit = "";

    var scaler = function(d) {
        return scale(d);
    };

    var count_format = d3.format("d");

    function dimension(value) {
        return scaler(value);
    }

    dimension.setDomain = function(min, max) {
        if(min == max) {
            min = Math.floor(min - 0.1 * min);
            max = Math.ceil(max + 0.1 * max);
        }

        scale.domain([min, max]);

    };

    dimension.setRange = function(min, max) {
        scale.range([min, max]);
    };

    dimension.scale = function() {
        return scale;
    };

    dimension.isOrdinal = function() {
        return false;
    };

    dimension.format = function(axis, max_length){
        axis.ticks(5)
            .tickSize(-max_length, -max_length)
            .tickFormat(count_format);

        return dimension;
    };

    dimension.relabel = function (axis) {

    };

    dimension.setUnit = function(unit_in) {
        unit = unit_in;
    };

    dimension.getUnit = function() {
        return unit;
    };

    return dimension;
}