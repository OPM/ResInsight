// Copyright (C) 2014 Statoil ASA, Norway.
//
// The file 'base_plot_ordinal_dimension.js' is part of ERT - Ensemble based Reservoir Tool.
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

function BasePlotOrdinalDimension(point_style){
    var scale = null;
    var unit = "";

    if(point_style) {
        scale = d3.scale.ordinal().rangePoints([0, 1], 1).domain(["unknown"]);
    } else {
        scale = d3.scale.ordinal().rangeRoundBands([0, 1], .5).domain(["unknown"]);
    }

    var scaler = function(d) {
        return scale(d);
    };

    function dimension(value) {
        return scaler(value);
    }

    dimension.setDomain = function(values) {
        scale.domain(values);
    };

    dimension.setRange = function(min, max) {
        if(point_style) {
            scale.rangePoints([min, max], 1);
        } else {
            scale.rangeRoundBands([min, max], .5);
        }
    };

    dimension.scale = function() {
        return scale;
    };

    dimension.format = function(axis, max_length){
        axis.tickSize(-max_length, -max_length);

        return dimension;
    };

    dimension.isOrdinal = function() {
        return true;
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