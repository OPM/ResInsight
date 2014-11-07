// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'histogram.js' is part of ERT - Ensemble based Reservoir Tool.
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


function Histogram(element, x_dimension, y_dimension) {
    var stored_data = null;
    var stored_case_name = "";

    var margin = {left: 40, right: 30, top: 20, bottom: 30};
    var width = 384 - margin.left - margin.right;
    var height = 256 - margin.top - margin.bottom;

    var style = STYLES["default"];

    var custom_value_min = null;
    var custom_value_max = null;

    var x_dimension = x_dimension;
    var y_dimension = y_dimension;


    var histogram_renderer = HistogramRenderer().x(x_dimension).y(y_dimension).margin(0, 1, 0, 1);
    var line_renderer = CanvasPlotLine().x(x_dimension).y(y_dimension);
    var stipple_renderer = CanvasPlotStippledLine().x(x_dimension).y(y_dimension);
    var area_renderer = CanvasPlotArea().x(x_dimension).y(y_dimension);
    var circle_renderer = CanvasCircle().x(x_dimension).y(y_dimension);

    var legend = CanvasPlotLegend();
    var legend_list = [];



    var group = element.append("div")
        .attr("class", "histogram");


    var title = group.append("div")
        .attr("class", "plot-title")
        .text("Histogram");

    var axis_label_group = group.append("div")
        .attr("id", "axis-label-group")

    var x_label = axis_label_group.append("div")
        .attr("class", "x axis-label")
        .text("");

    var y_label = axis_label_group.append("div")
        .attr("class", "y axis-label")
        .text("");

    var histogram_area = group.append("div").attr("class", "plot-area");

    var legend_group = group.append("div")
        .attr("class", "plot-legend-group");

    var histogram_group = histogram_area.append("svg")
        .attr("class", "plot-svg");


    var svg = histogram_group.append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")")
        .style("width", width + "px")
        .style("height", height + "px");

    var canvas = histogram_area.append("canvas")
        .attr("id", "histogram-canvas")
        .attr("width", width)
        .attr("height", height)
        .style("position", "absolute")
        .style("top", (margin.top) + "px")
        .style("left", (margin.left) + "px")
        .style("z-index", 5);




    var y_axis = d3.svg.axis()
        .scale(y_dimension.scale())
        .orient("left");

    var x_axis = d3.svg.axis()
        .scale(x_dimension.scale())
        .orient("bottom");

    histogram_group.append("g")
        .attr("class", "y axis pale")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")")
        .call(y_axis);

    histogram_group.append("g")
        .attr("class", "x axis")
        .attr("transform", "translate(" + margin.left + ", " + (height + margin.top) + ")")
        .call(x_axis);

    var resetLegends = function() {
        legend_list = [];
    };

    var addLegend = function(style, name, render_function) {
        legend_list.push({"style": style, "name": name,"render_function": render_function});
    };

    function formatDate(ctime) {
        var date = new Date(ctime * 1000);
        var year = date.getFullYear();
        var month = date.getMonth() + 1;
        var day = date.getDate();

        if (month < 10) {
            month = "0" + month;
        }

        if (day < 10) {
            day = "0" + day;
        }

        return year + "-" + month + "-" + day;
    }


    function createLogBinFunction (bin_count) {
        function binner(range, values) {
            var thresholds = [];

            var range_diff = range[1] - range[0];

            var from_log = Math.round(Math.log(range[0]) / Math.log(10));
            var to_log = Math.round(Math.log(range[1]) / Math.log(10));

            var from_to_diff = to_log - from_log;
            var log_bin_count = from_to_diff;
            var log_bin_count_next = from_to_diff * 2;

            while(log_bin_count_next < bin_count) {
                log_bin_count = log_bin_count_next;
                log_bin_count_next += from_to_diff;
            }



            if(log_bin_count_next - bin_count < bin_count - log_bin_count) {
                bin_count = log_bin_count_next;
            } else {
                bin_count = log_bin_count;
            }

            var step = from_to_diff / bin_count ;

            var sum = 0;
            for(var i = 0; i <= bin_count; i++) {
                sum += Math.pow(10, i * step);
            }

            var bin_size = range_diff / sum;

            thresholds.push(range[0]);

            for(var i = 1; i < bin_count; i++) {
                var value = thresholds[i - 1] + (bin_size * Math.pow(10, i * step));
                thresholds.push(value);
            }


            thresholds.push(range[1]);

            return thresholds;

        }
        return binner;
    }




    function histogram(data, case_name) {
        if (!arguments.length) {
            if(stored_data == null) {
                return;
            }
            data = stored_data;
            case_name = stored_case_name;
        } else {
            stored_data = data;
            stored_case_name = case_name;
        }

        x_axis.scale(x_dimension.scale());
        x_dimension.format(x_axis, height);
        x_axis.tickSize(0, 0);

        y_axis.scale(y_dimension.scale());
        y_dimension.format(y_axis, width);


        resetLegends();

        title.text(histogram.getTitle());

        if (x_dimension.getUnit() == "") {
            x_label.text("");
        } else {
            x_label.text("X: " + x_dimension.getUnit());
        }

        if (y_dimension.getUnit() == "") {
            y_label.text("");
        } else {
            y_label.text("Y: " + y_dimension.getUnit());
        }

        var context = canvas.node().getContext("2d");
        context.save();
        context.clearRect(0, 0, width, height);


        if(data.hasObservation()) {
            line_renderer.style(STYLES["observation"]);
            var obs = data.observation();
            var top = data.maxCount() + 1;
            stipple_renderer(context, [obs, obs], [0, top]);
            addLegend(STYLES["observation"], "Observation", CanvasPlotLegend.stippledLine);

            var error = data.observationError();
            area_renderer.style(STYLES["observation_area"]);
            area_renderer(context, [obs - error, obs + error, obs + error, obs - error], [top, top, 0, 0]);

            addLegend(STYLES["observation_area"], "Observation error", CanvasPlotLegend.filledCircle);
        }

        if(data.hasRefcase()) {
            line_renderer.style(STYLES["refcase"]);
            line_renderer(context, [data.refcase(), data.refcase()], [0, data.maxCount() + 1]);
            addLegend(STYLES["refcase"], "Refcase", CanvasPlotLegend.simpleLine);
        }

        if(data.hasCaseHistogram(case_name)) {

            var case_histogram = data.caseHistogram(case_name);
            var bin_count = data.numberOfBins();

            var bins = histogram.histogramLayout(bin_count)(case_histogram.samples());

//            var ticks = [];
//            for(var i = 0; i < bins.length; i++) {
//                ticks.push(bins[i].x);
//
//                if(i == bins.length - 1) {
//                    ticks.push(bins[i].x + bins[i].dx);
//                }
//            }
//
//            x_dimension.setTicks(ticks);

            histogram_renderer.style(style);
            histogram_renderer(context, bins);

            addLegend(style, case_name, CanvasPlotLegend.filledCircle);
        }

        legend_group.selectAll(".plot-legend").data(legend_list).call(legend);
        histogram_group.select(".y.axis").call(y_axis);
        var axis = histogram_group.select(".x.axis").call(x_axis);
        x_dimension.relabel(axis);

        context.restore();
    }


    histogram.histogramLayout = function(bin_count) {
        var layout = null;
        var use_log_scale = false;

        if("isLogScale" in x_dimension) {
            use_log_scale = x_dimension.isLogScale();
        }

        if(use_log_scale) {
            layout = d3.layout.histogram()
                .range(x_dimension.scale().domain())
                .bins(createLogBinFunction(bin_count));
        } else {
            layout = d3.layout.histogram()
                .range(x_dimension.scale().domain())
                .bins(bin_count);
        }

        return layout;
    };

    histogram.setSize = function(w, h) {
        w = w - 80;
        h = h - 70;

        width = w - margin.left - margin.right;
        height = h - margin.top - margin.bottom;

        x_dimension.setRange(0, width);
        y_dimension.setRange(height, 0);

        histogram_group.style("width", w + "px");
        histogram_group.style("height", h + "px");

        canvas.attr("width", width).attr("height", height);

        svg.style("width", width + "px");
        svg.style("height", height + "px");

        histogram_group.select(".x.axis")
            .attr("transform", "translate(" + margin.left + ", " + (height + margin.top) + ")");

    };


    histogram.style = function (value) {
        if (!arguments.length) return style;
        style = value;
        return histogram;
    };

    histogram.setValueScales = function(min, max) {
        custom_value_min = min;
        custom_value_max = max;
    };

    histogram.setVisible = function(visible) {
        if(!visible) {
            group.style("display", "none");
        } else {
            group.style("display", "inline-block");
        }
    };

    histogram.getTitle = function(){
        var data = stored_data;
        var report_date = data.reportStepTime();
        if(report_date == 0){
            return data.name();
        } else {
            return data.name() + " @ " + formatDate(data.reportStepTime());
        }
    };

    return histogram;
}