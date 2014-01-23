// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'plot.js' is part of ERT - Ensemble based Reservoir Tool.
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


function Plot(element, data) {
    this.stored_data = [];
    this.margin = {left: 90, right: 20, top: 20, bottom: 30};
    this.root_elemenet = element;

    var group = this.root_elemenet.append("div")
        .attr("class", "plot");


    this.title = group.append("div")
        .attr("class", "plot-title")
        .text(data.name);

    this.plot_group = group.append("svg")
        .attr("class", "plot-svg");

    this.legend_group = group.append("div")
        .attr("class", "plot-legend-group");

    this.width = 1024 - this.margin.left - this.margin.right;
    this.height = 512 - this.margin.top - this.margin.bottom;

    this.svg = this.plot_group
        .append("g")
        .attr("transform", "translate(" + this.margin.left + "," + this.margin.top + ")")
        .style("width", this.width + "px")
        .style("height", this.height + "px")
        .attr("fill", "rgb(255, 128, 0)");

    this.x_scale = d3.time.scale().range([0, this.width]);
    this.y_scale = d3.scale.linear().range([this.height, 0]).nice();

    this.y_axis = d3.svg.axis()
        .scale(this.y_scale)
        .ticks(10)
        .tickPadding(10)
        .orient("left")
        .tickSize(-this.width, -this.width);

    this.x_axis = d3.svg.axis()
        .scale(this.x_scale)
        .ticks(10)
        .tickPadding(10)
        .orient("bottom")
        .tickSubdivide(true);

    this.plot_group.append("g")
        .attr("class", "y axis pale")
        .attr("transform", "translate(" + this.margin.left + "," + this.margin.top + ")")
        .call(this.y_axis);

    this.plot_group.append("g")
        .attr("class", "x axis")
        .attr("transform", "translate(" + this.margin.left + ", " + (this.height + this.margin.top) + ")")
        .call(this.x_axis);


    var self = this;
    this.plot = this.svg.append("g");

    this.adjustY = function(y, std) {
        if(y >= 0) {
            y = Math.max(0, y - std);
        } else {
            y -= std
        }
        return y;
    };

    var top_function = function(d) {
        return d["y"] + d["std"];
    };

    var bottom_function = function(d) {
        return self.adjustY(d["y"], d["std"]);
    };

    this.x = function (d) {
        return self.x_scale(new Date(d["x"] * 1000));
    };

    this.y = function (d) {
        return self.y_scale(d["y"]);
    };

    this.y_min = function (d) {
        return  self.y_scale(bottom_function(d));
    };

    this.y_max = function (d) {
        return self.y_scale(top_function(d));
    };

    this.duration = 0;

    this.std_plot = StdPlot()
        .radius(2.5)
        .x(this.x)
        .y(this.y)
        .y_max(this.y_max)
        .y_min(this.y_min)
        .style("observation-std-point")
        .duration(this.duration);

    this.observation_line = PlotLine()
        .x(this.x)
        .y(this.y)
        .style("observation-plot-line")
        .duration(this.duration);

    this.observation_std_area = PlotArea()
        .x(this.x)
        .y_min(this.y_min)
        .y_max(this.y_max)
        .style("observation-plot-area")
        .duration(this.duration);

    this.refcase_line = PlotLine()
        .x(this.x)
        .y(this.y)
        .style("refcase-plot-line")
        .duration(this.duration);


    this.ensemble_styles = ["ensemble-plot-1", "ensemble-plot-2", "ensemble-plot-3", "ensemble-plot-4", "ensemble-plot-5"];
    this.ensemble_lines = {};
    for (var index in this.ensemble_styles) {
        var style = this.ensemble_styles[index];
        this.ensemble_lines[style] = PlotLine()
            .x(this.x)
            .y(this.y)
            .style(style)
            .duration(this.duration);
    }

    this.legend = PlotLegend();

}

Plot.prototype.resize = function(width, height) {
    //Some magic margins...
    width = width - 80;
    height = height - 70;

    this.width = width - this.margin.left - this.margin.right;
    this.height = height - this.margin.top - this.margin.bottom;

    this.x_scale.range([0, this.width]);
    this.y_scale.range([this.height, 0]).nice();

    this.y_axis.tickSize(-this.width, -this.width);

    this.plot_group.style("width", width + "px");
    this.plot_group.style("height", height + "px");

    this.svg.style("width", this.width + "px");
    this.svg.style("height", this.height + "px");

    this.setData(this.stored_data);

    this.plot_group.select(".x.axis")
        .attr("transform", "translate(" + this.margin.left + ", " + (this.height + this.margin.top) + ")");
};



Plot.prototype.setData = function(data) {
    this.stored_data = data;

    this.title.text(data["name"]);

    var min = data["min_y"];
    var max = data["max_y"];
    this.y_scale.domain([min, max]).nice();
    this.x_scale.domain([new Date(data["min_x"] * 1000), new Date(data["max_x"] * 1000)]).nice();

    var legends = [];

    var observation_std_points;
    var observation_line;
    var observation_std_area;


    if(data["observations"] != null) {

        var observation_samples = data["observations"]["samples"];

        if(data["observations"]["continuous_line"]) {
            observation_line = this.plot.selectAll(".observation-plot-line").data([observation_samples]);
            observation_std_area = this.plot.selectAll(".observation-plot-area").data([observation_samples]);
            observation_std_points = this.plot.selectAll(".observation-std-point").data([]);

            legends.push({"style": "observation-plot-line", "name": "Observation"});
            legends.push({"style": "observation-plot-area", "name": "Observation error"});
        } else {
            observation_line = this.plot.selectAll(".observation-plot-line").data([]);
            observation_std_area = this.plot.selectAll(".observation-plot-area").data([]);
            observation_std_points = this.plot.selectAll(".observation-std-point").data(observation_samples);

            legends.push({"style": "observation-std-point", "name": "Observation error bar"});
        }

        observation_line.call(this.observation_line);
        observation_std_area.call(this.observation_std_area);
        observation_std_points.call(this.std_plot);
    } else {
        observation_line = this.plot.selectAll(".observation-plot-line").data([]);
        observation_std_area = this.plot.selectAll(".observation-plot-area").data([]);
        observation_std_points = this.plot.selectAll(".observation-std-point").data([]);

        observation_line.call(this.observation_line);
        observation_std_area.call(this.observation_std_area);
        observation_std_points.call(this.std_plot);
    }


    var refcase_line;

    if(data["refcase"] != null) {
        var refcase_samples = data["refcase"]["samples"];
        refcase_line = this.plot.selectAll(".refcase-plot-line").data([refcase_samples]);
        refcase_line.call(this.refcase_line);

        legends.push({"style": "refcase-plot-line", "name": "Refcase"});

    } else {
        refcase_line = this.plot.selectAll(".refcase-plot-line").data([]);
        refcase_line.call(this.refcase_line);
    }

    for(var ensemble_index in data["ensemble_names"]) {
        var ensemble_style = this.ensemble_styles[ensemble_index];
        var name = data["ensemble_names"][ensemble_index];

        var ensemble_samples = [];
        for (var index = 0; index < data["ensemble"][ensemble_index].length; index++) {
            ensemble_samples.push(data["ensemble"][ensemble_index][index]["samples"]);
        }
        var ensemble_lines = this.plot.selectAll("." + ensemble_style).data(ensemble_samples);
        ensemble_lines.call(this.ensemble_lines[ensemble_style]);

        legends.push({"style": ensemble_style, "name": data["ensemble_names"][ensemble_index]});
    }

    var from = 0;
    if (data["ensemble_names"] != null) {
        from = data["ensemble_names"].length;
    }

    for(var style_index = from; style_index < this.ensemble_styles.length; style_index++) {
        var style = this.ensemble_styles[style_index];
        var removed_ensemble_lines = this.plot.selectAll("." + style).data([]);
        removed_ensemble_lines.call(this.ensemble_lines[style]);
    }

    this.legend_group.selectAll(".plot-legend").data(legends).call(this.legend);

    this.plot_group.select(".y.axis").transition().duration(this.duration).call(this.y_axis);
    this.plot_group.select(".x.axis").transition().duration(this.duration).call(this.x_axis);
};