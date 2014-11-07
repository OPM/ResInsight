// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'canvas_plot_overview.js' is part of ERT - Ensemble based Reservoir Tool.
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


function StatisticsPlot(element, x_dimension, y_dimension) {
    this.plot = new BasePlot(element, x_dimension, y_dimension);

    var horizontal_draw_direction = true;

    var line_renderer = this.plot.createLineRenderer();
    var stippled_line_renderer = this.plot.createStippledLineRenderer();
    var area_renderer = this.plot.createAreaRenderer();
    var dark_area_renderer = this.plot.createAreaRenderer();


    var self = this;

    var renderEnsemble = function (context, data) {
        if (data.hasEnsembleData()) {
            var case_list = data.caseList();

            for (var i = 0; i < case_list.length; i++) {
                var style = STYLES[("ensemble_" + (i + 1))];
                var case_name = case_list[i];
                var ensemble_data = data.ensembleData(case_name);

                var values = ensemble_data.xValues();
                if (!horizontal_draw_direction) {
                    values = ensemble_data.yValues();
                }

                var y_min_values = ensemble_data.yMinValues();
                var y_max_values = ensemble_data.yMaxValues();
                var median = ensemble_data.xPercentile(0.5);
                var p10 = ensemble_data.xPercentile(0.1);
                var p33 = ensemble_data.xPercentile(0.33);
                var p67 = ensemble_data.xPercentile(0.67);
                var p90 = ensemble_data.xPercentile(0.9);

                var area_values = CanvasPlotArea.mergePoints(values, values);

                var area_style = {
                        stroke: "rgba(255,255, 255, 0.0)",
                        fill: style["fill"],
                        stroke_width: 1
                };

                var dark_area_style = {
                            stroke: "rgba(255,255, 255, 0.0)",
                            fill: STYLES.darker(style["fill"]),
                            stroke_width: 1
                    };

                var stipple_style = {
                        stroke: style["stroke"],
                        fill: "rgba(255, 255, 255, 0.0)",
                        stroke_width: style["stroke_width"],
                        line_cap: "butt"
                };

                line_renderer.style(style);
                stippled_line_renderer.style(stipple_style);
                stippled_line_renderer.stippleLength(5);
                area_renderer.style(area_style);
                dark_area_renderer.style(dark_area_style);

                if (horizontal_draw_direction) {
                    stippled_line_renderer(context, values, y_max_values);
                    stippled_line_renderer(context, values, y_min_values);

                    area_renderer(context, area_values, CanvasPlotArea.mergePoints(p10, p90));
                    dark_area_renderer(context, area_values, CanvasPlotArea.mergePoints(p33, p67));

                    stippled_line_renderer(context, values, median);

                } else {
                    stippled_line_renderer(context, y_max_values, values);
                    stippled_line_renderer(context, y_min_values, values);

                    area_renderer(context, CanvasPlotArea.mergePoints(p10, p90), area_values);
                    dark_area_renderer(context, CanvasPlotArea.mergePoints(p33, p67), area_values);

                    stippled_line_renderer(context, median, values);
                }

                self.plot.addLegend(area_style, "P10 - P90", CanvasPlotLegend.filledCircle);
                self.plot.addLegend(dark_area_style, "P33 - P67", CanvasPlotLegend.filledCircle);
                self.plot.addLegend(stipple_style, "Min - Median - Max", CanvasPlotLegend.stippledLine);
                self.plot.addLegend(style, case_name, CanvasPlotLegend.filledCircle);
            }
            self.plot.renderCallbackFinishedRendering();
        }
    };


    this.plot.setRenderCallback(renderEnsemble);
}

StatisticsPlot.prototype.resize = function (width, height) {
    this.plot.resize(width, height);
};

StatisticsPlot.prototype.setScales = function (x_min, x_max, y_min, y_max) {
    this.plot.setScales(x_min, x_max, y_min, y_max);
};

StatisticsPlot.prototype.setData = function (data) {
    this.plot.setData(data);
};


StatisticsPlot.prototype.setVerticalErrorBar = function (vertical) {
    this.plot.setVerticalErrorBar(vertical);
};

StatisticsPlot.prototype.setHorizontalDrawDirection = function (horizontal) {
    this.horizontal_draw_direction = horizontal;
};

StatisticsPlot.prototype.setCustomSettings = function (settings) {
    this.plot.setCustomSettings(settings);
};

StatisticsPlot.prototype.setRenderingFinishedCallback = function(callback) {
    this.plot.setRenderingFinishedCallback(callback);
};

StatisticsPlot.prototype.renderNow = function(){
    this.plot.render();
};

StatisticsPlot.prototype.getTitle = function(){
    return this.plot.getTitle();
};

