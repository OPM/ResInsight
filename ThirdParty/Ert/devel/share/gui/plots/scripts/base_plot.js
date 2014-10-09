// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'base_plot.js' is part of ERT - Ensemble based Reservoir Tool.
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

function BasePlot(element, x_dimension, y_dimension) {
    this.stored_data = null;
    this.margin = {left: 90, right: 20, top: 20, bottom: 30};
    this.root_elemenet = element;

    this.render_observations = true;
    this.render_refcase = true;

    this.custom_y_min = null;
    this.custom_y_max = null;
    this.custom_x_min = null;
    this.custom_x_max = null;

    this.render_finished = false;
    this.render_callback_finished = false;
    this.rendering_finished_callback = null;

    this.pre_render_callback = null;
    this.render_callback = null;

    this.dimension_x = x_dimension;
    this.dimension_y = y_dimension;

    this.vertical_error_bar = true;
    this.error_bar_only = false;

    var group = this.root_elemenet.append("div")
        .attr("class", "plot");

    this.title = group.append("div")
        .attr("class", "plot-title")
        .text(this.getTitle());

    this.axis_label_group = group.append("div")
        .attr("id", "axis-label-group")

    this.x_label = this.axis_label_group.append("div")
        .attr("class", "x axis-label")
        .text("")

    this.y_label = this.axis_label_group.append("div")
        .attr("class", "y axis-label")
        .text("");

    var plot_area = group.append("div").attr("class", "plot-area");

    this.width = 1024 - this.margin.left - this.margin.right;
    this.height = 512 - this.margin.top - this.margin.bottom;

    this.canvas = plot_area.append("canvas")
        .attr("id", "plot-canvas")
        .attr("width", this.width)
        .attr("height", this.height)
        .style("position", "absolute")
        .style("top", (this.margin.top) + "px")
        .style("left", (this.margin.left) + "px")
        .style("z-index", 5);

    this.overlay_canvas = plot_area.append("canvas")
        .attr("id", "plot-overlay-canvas")
        .attr("width", this.width)
        .attr("height", this.height)
        .style("position", "absolute")
        .style("top", (this.margin.top) + "px")
        .style("left", (this.margin.left) + "px")
        .style("z-index", 5);

    this.plot_group = plot_area.append("svg")
        .attr("class", "plot-svg")
        .style("z-index", 10);

    this.legend_group = group.append("div")
        .attr("class", "plot-legend-group");

    this.y_axis = d3.svg.axis()
        .scale(this.dimension_y.scale())
        .orient("left");

    this.dimension_y.format(this.y_axis, this.width);

    this.x_axis = d3.svg.axis()
        .scale(this.dimension_x.scale())
        .orient("bottom");

    this.dimension_x.format(this.x_axis, this.height);

    this.plot_group.append("g")
        .attr("class", "y axis pale")
        .attr("transform", "translate(" + this.margin.left + "," + this.margin.top + ")")
        .call(this.y_axis);

    this.plot_group.append("g")
        .attr("class", "x axis pale")
        .attr("transform", "translate(" + this.margin.left + ", " + (this.height + this.margin.top) + ")")
        .call(this.x_axis);


    this.x = this.dimension_x;
    this.y = this.dimension_y;

    this.legend = CanvasPlotLegend();
    this.legend_list = [];

    this.line_renderer = CanvasPlotLine().x(this.x).y(this.y);
    this.area_renderer = CanvasPlotArea().x(this.x).y(this.y);
    this.error_bar_renderer = CanvasErrorBar().x(this.x).y(this.y);
    this.stippled_line_renderer = CanvasPlotStippledLine().x(this.x).y(this.y);
}

BasePlot.prototype.resize = function(width, height) {
    //Some magic margins...
    width = width - 80;
    height = height - 70;

    this.width = width - this.margin.left - this.margin.right;
    this.height = height - this.margin.top - this.margin.bottom;

    this.dimension_x.format(this.x_axis, this.height);
    this.dimension_y.format(this.y_axis, this.width);

    this.dimension_x.setRange(0, this.width);
    this.dimension_y.setRange(this.height, 0);

    this.plot_group.style("width", width + "px").style("height", height + "px");

    this.canvas.attr("width", this.width).attr("height", this.height);
    this.overlay_canvas.attr("width", this.width).attr("height", this.height);

    this.plot_group.select(".x.axis").attr("transform", "translate(" + this.margin.left + ", " + (this.height + this.margin.top) + ")");
};

BasePlot.prototype.setScales = function(x_min, x_max, y_min, y_max) {

    if(this.custom_y_min != y_min || this.custom_y_max != y_max ||
        this.custom_x_min != x_min || this.custom_x_max != x_max) {
        this.custom_y_min = y_min;
        this.custom_y_max = y_max;
        this.custom_x_min = x_min;
        this.custom_x_max = x_max;
    }
};

BasePlot.prototype.setYDomain = function(min_y, max_y, ordinals) {
    if (arguments.length == 3 && this.dimension_y.isOrdinal()) {
        this.dimension_y.setDomain(ordinals);
    } else {
        var min = min_y;
        var max = max_y;

        if (this.custom_y_min != null) {
            min = this.custom_y_min;
        }

        if (this.custom_y_max != null) {
            max = this.custom_y_max;
        }

        this.dimension_y.setDomain(min, max);
    }
};

BasePlot.prototype.setXDomain = function(min_x, max_x, ordinals) {
    if (arguments.length == 3 && this.dimension_x.isOrdinal()) {
        this.dimension_x.setDomain(ordinals);
    } else {
        var min = min_x;
        var max = max_x;
        if (this.custom_x_min != null) {
            min = this.custom_x_min;
        }

        if (this.custom_x_max != null) {
            max = this.custom_x_max;
        }

        this.dimension_x.setDomain(min, max);
    }
};

BasePlot.prototype.setData = function(data) {
    this.stored_data = data;
};

BasePlot.prototype.getTitle = function(){
    if(this.stored_data != null && "name" in this.stored_data){
        return this.stored_data.name();
    } else {
        return "No data";
    }
};

BasePlot.prototype.render = function() {
    if(this.stored_data == null) {
        return;
    }

    var data = this.stored_data;
    this.render_finished = false;
    this.render_callback_finished = false;

    this.title.text(this.getTitle());

    if (this.dimension_x.getUnit() == "") {
        this.x_label.text("");
    } else {
        this.x_label.text("X: " + this.dimension_x.getUnit());
    }

    if (this.dimension_y.getUnit() == "") {
        this.y_label.text("");
    } else {
        this.y_label.text("Y: " + this.dimension_y.getUnit());
    }

    if(data.hasBoundaries()) {
        this.setYDomain(data.minY(), data.maxY());
        this.setXDomain(data.minX(), data.maxX());
    }

    if(this.pre_render_callback != null) {
        this.pre_render_callback(data);
    }

    this.resetLegends();

    var axis = this.plot_group.select(".y.axis").call(this.y_axis);
    this.dimension_y.relabel(axis);

    axis = this.plot_group.select(".x.axis").call(this.x_axis);
    this.dimension_x.relabel(axis);

    this.canvas.attr("width", this.width).attr("height", this.height);
    this.overlay_canvas.attr("width", this.width).attr("height", this.height);

    var context = this.canvas.node().getContext("2d");
    context.save();
    context.clearRect(0, 0, this.width, this.height);

    var overlay_context = this.overlay_canvas.node().getContext("2d");
    overlay_context.save();
    overlay_context.clearRect(0, 0, this.width, this.height);

    this.render_callback(context, data);

    if(this.render_observations) {
        this.renderObservations(overlay_context, data);
    }

    if(this.render_refcase) {
        this.renderRefcase(overlay_context, data);
    }

    overlay_context.restore();
    context.restore();
    this.finishedRendering();

};

BasePlot.prototype.setRenderCallback = function(callback) {
    this.render_callback = callback;
};

BasePlot.prototype.setPreRenderCallback = function(callback) {
    this.pre_render_callback = callback;
};


BasePlot.prototype.resetLegends = function() {
    this.legend_list = [];
};

BasePlot.prototype.addLegend = function(style, name, render_function) {
    this.legend_list.push({"style": style, "name": name,"render_function": render_function});
};


BasePlot.prototype.renderObservations = function(context, data) {
    if(data.hasObservationData()) {
        var obs_data = data.observationData();
        if(obs_data.isContinuous() && !this.error_bar_only) {
            var x_values = obs_data.xValues();
            var y_values = obs_data.yValues();
            var std_values = obs_data.stdValues();

            var obs_x_area_samples = [];
            var obs_y_area_samples = [];

            for (var index = 0; index < x_values.length; index++) {
                obs_x_area_samples.push(x_values[index]);
                obs_y_area_samples.push(y_values[index] + std_values[index]);
            }

            for (var index = x_values.length - 1; index >= 0; index--) {
                obs_x_area_samples.push(x_values[index]);
                obs_y_area_samples.push(y_values[index] - std_values[index]);
            }

            this.area_renderer.style(STYLES["observation_area"]);
            this.area_renderer(context, obs_x_area_samples, obs_y_area_samples);

            this.stippled_line_renderer.style(STYLES["observation"]);
            this.stippled_line_renderer(context, x_values, y_values);

            this.addLegend(STYLES["observation"], "Observation", CanvasPlotLegend.stippledLine);
            this.addLegend(STYLES["observation_area"], "Observation error", CanvasPlotLegend.filledCircle);

        } else {

            var obs_x_samples = obs_data.xValues();
            var obs_y_samples = obs_data.yValues();
            var obs_std_samples = obs_data.stdValues();

            for (var index = 0; index < obs_x_samples.length; index++) {
                var x = obs_x_samples[index];
                var y = obs_y_samples[index];
                var error = obs_std_samples[index];

                this.error_bar_renderer.style(STYLES["observation_error_bar"]);
                this.error_bar_renderer(context, x, y, error, this.vertical_error_bar);
            }
            this.addLegend(STYLES["observation_error_bar"], "Observation error bar", CanvasPlotLegend.errorBar);
        }
    }
};


BasePlot.prototype.renderRefcase = function(context, data) {
    if(data.hasRefcaseData()) {
        var refcase_data = data.refcaseData();
        var style = STYLES["refcase"];

        this.line_renderer.style(style);
        this.line_renderer(context, refcase_data.xValues(), refcase_data.yValues());

        this.addLegend(style, "Refcase", CanvasPlotLegend.simpleLine);
    }
};

BasePlot.prototype.createLineRenderer = function() {
    return CanvasPlotLine().x(this.x).y(this.y);
};

BasePlot.prototype.createCircleRenderer = function() {
    return CanvasCircle().x(this.x).y(this.y);
};

BasePlot.prototype.createCrossRenderer = function() {
    return CanvasCross().x(this.x).y(this.y);
};

BasePlot.prototype.createStippledLineRenderer = function() {
    return CanvasPlotStippledLine().x(this.x).y(this.y);
};

BasePlot.prototype.createAreaRenderer = function() {
    return CanvasPlotArea().x(this.x).y(this.y);
};


BasePlot.prototype.setVerticalErrorBar = function(vertical){
    this.vertical_error_bar = vertical;

};

BasePlot.prototype.setCustomSettings = function (settings) {
    if ("error_bar_only" in settings) {
        this.error_bar_only = settings["error_bar_only"];
    }

    if ("show_observations" in settings) {
        this.setRenderObservations(settings["show_observations"]);
    }

    if ("show_refcase" in settings) {
        this.setRenderRefcase(settings["show_refcase"]);
    }
};

BasePlot.prototype.renderCallbackFinishedRendering = function(){
    this.render_callback_finished = true;
    this.emitFinishedRendering()
};

BasePlot.prototype.finishedRendering = function(){
    this.render_finished = true;
    this.emitFinishedRendering();
};

BasePlot.prototype.emitFinishedRendering = function(){
    if(this.rendering_finished_callback != null){
        if(this.render_finished && this.render_callback_finished) {
            this.legend_group.selectAll(".plot-legend").data(this.legend_list).call(this.legend);
            this.rendering_finished_callback();
        }
    }
};

BasePlot.prototype.setRenderingFinishedCallback = function(callback) {
    this.rendering_finished_callback = callback;
};

BasePlot.prototype.setLogScaleOnDimensionX = function(use_log_scale) {
    this.dimension_x.setIsLogScale(use_log_scale);
    this.x_axis.scale(this.dimension_x.scale());
    this.dimension_x.format(this.x_axis, this.height);
};

BasePlot.prototype.setLogScaleOnDimensionY = function(use_log_scale) {
    this.dimension_y.setIsLogScale(use_log_scale);
    this.y_axis.scale(this.dimension_y.scale());
    this.dimension_y.format(this.y_axis, this.width);
};

BasePlot.prototype.setRenderObservations = function(enabled) {
    this.render_observations = enabled;
};

BasePlot.prototype.setRenderRefcase = function(enabled) {
    this.render_refcase = enabled;
};

function isNumber(n) {
    return !isNaN(parseFloat(n)) && isFinite(n);
}
