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


function OverviewPlot(element, x_dimension, y_dimension) {
    this.plot = new BasePlot(element, x_dimension, y_dimension);

    this.horizontal_draw_direction = true;

    var self = this;

    var renderEnsemble = function (context, data) {
        if (data.hasEnsembleData()) {
            var case_list = data.caseList();

            for (var i = 0; i < case_list.length; i++) {
                var style = STYLES[("ensemble_" + (i + 1))];
                var case_name = case_list[i];
                var ensemble_data = data.ensembleData(case_name);

                var values = ensemble_data.xValues();
                if (!self.horizontal_draw_direction) {
                    values = ensemble_data.yValues();
                }

                var y_min_values = ensemble_data.yMinValues();
                var y_max_values = ensemble_data.yMaxValues();

                var x_area_values = CanvasPlotArea.mergePoints(values, values);
                var y_area_values = CanvasPlotArea.mergePoints(y_min_values, y_max_values);


                self.plot.area_renderer.style(style);
                if (self.horizontal_draw_direction) {
                    self.plot.area_renderer(context, x_area_values, y_area_values);
                } else {
                    self.plot.area_renderer(context, y_area_values, x_area_values);
                }

                self.plot.addLegend(style, case_name, CanvasPlotLegend.filledCircle);
            }
            self.plot.renderCallbackFinishedRendering();
        }
    };


    this.plot.setRenderCallback(renderEnsemble);
}

OverviewPlot.prototype.resize = function (width, height) {
    this.plot.resize(width, height);
};

OverviewPlot.prototype.setScales = function (x_min, x_max, y_min, y_max) {
    this.plot.setScales(x_min, x_max, y_min, y_max);
};

OverviewPlot.prototype.setData = function (data) {
    this.plot.setData(data);
};


OverviewPlot.prototype.setVerticalErrorBar = function (vertical) {
    this.plot.setVerticalErrorBar(vertical);
};

OverviewPlot.prototype.setHorizontalDrawDirection = function (horizontal) {
    this.horizontal_draw_direction = horizontal;
};

OverviewPlot.prototype.setCustomSettings = function (settings) {
    this.plot.setCustomSettings(settings);
};

OverviewPlot.prototype.setRenderingFinishedCallback = function(callback) {
    this.plot.setRenderingFinishedCallback(callback);
};

OverviewPlot.prototype.renderNow = function(){
    this.plot.render();
};

OverviewPlot.prototype.getTitle = function(){
    return this.plot.getTitle();
};

