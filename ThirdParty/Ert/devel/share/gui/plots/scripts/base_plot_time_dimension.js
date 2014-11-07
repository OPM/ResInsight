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

function BasePlotTimeDimension(){
//    var scale = d3.scale.linear().range([1, 0]).domain([0, 1]).nice();
    var scale = d3.time.scale().range([0, 1]).domain([1, 0]);
    var time_scale = d3.time.scale().range([0, 1]).domain([1, 0]);

    var unit = "Date";

    var scaler = function(d) {
        return scale(d);
    };

    var customTimeFormat = d3.time.format.multi([
        [".%L", function(d) { return d.getMilliseconds(); }],
        [":%S", function(d) { return d.getSeconds(); }],
        ["%I:%M", function(d) { return d.getMinutes(); }],
        ["%I %p", function(d) { return d.getHours(); }],
        ["%a %d", function(d) { return d.getDay() && d.getDate() != 1; }],
        ["%b %d", function(d) { return d.getDate() != 1; }],
        ["%b", function(d) { return d.getMonth(); }],
        ["%Y", function() { return true; }]
    ]);

    function dimension(value) {
        return scaler(value);
    }

    dimension.setDomain = function(min, max) {
        time_scale.domain([new Date(min * 1000), new Date(max * 1000)]).nice();
        var domain = time_scale.domain();
        scale.domain([domain[0].getTime() / 1000, domain[1].getTime() / 1000]);
    };

    dimension.setRange = function(min, max) {
        scale.range([min, max]);
        time_scale.range([min, max]);
    };

    dimension.scale = function() {
        return time_scale;
    };

    dimension.isOrdinal = function() {
        return false;
    };

    dimension.format = function(axis, max_length){
        axis.tickFormat(customTimeFormat);

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