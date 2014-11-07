// Copyright (C) 2014 Statoil ASA, Norway.
//
// The file 'canvas_plot_distribution.js' is part of ERT - Ensemble based Reservoir Tool.
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


function DistributionPlot(element, x_dimension, y_dimension) {
    this.plot = new BasePlot(element, x_dimension, y_dimension);
    this.horizontal_draw_direction = true;

    var self = this;

    var preRenderCallback = function(data) {
        if (data.hasEnsembleData()) {
            if (!self.horizontal_draw_direction) {
                self.plot.setLogScaleOnDimensionX(data.shouldUseLogScale());
            } else {
                self.plot.setLogScaleOnDimensionY(data.shouldUseLogScale());
            }

            self.plot.setXDomain(data.minX(), data.maxX(), data.caseList());
            self.plot.setYDomain(data.minY(), data.maxY(), data.caseList());
        }
    };

    var renderCallback = function (context, data) {
        if (data.hasEnsembleData()) {
            var case_list = data.caseList();

            for (var i = 0; i < case_list.length; i++) {
                var style = STYLES[("ensemble_" + (i + 1))];
                var case_name = case_list[i];
                var ensemble_data = data.ensembleData(case_name);


                var values;
                if (!self.horizontal_draw_direction) {
                    values = ensemble_data.xValues();
                } else {
                    values = ensemble_data.yValues();
                }

                var circle_renderer = self.plot.createCircleRenderer();
                circle_renderer.style(style);
                circle_renderer.fillCircle(true);
                circle_renderer.radius(5);

                for(var j = 0; j < values.length; j++) {
                    var value = values[j];
                    if(isNumber(value)) {
                        if (self.horizontal_draw_direction) {
                            circle_renderer(context, case_name, value);
                        } else {
                            circle_renderer(context, value, case_name);
                        }
                    }
                }

                self.plot.addLegend(style, case_name, CanvasPlotLegend.filledCircle);
            }
            self.plot.renderCallbackFinishedRendering();
        }
    };

    this.plot.setPreRenderCallback(preRenderCallback);
    this.plot.setRenderCallback(renderCallback);
}

DistributionPlot.prototype.resize = function (width, height) {
    this.plot.resize(width, height);
};

DistributionPlot.prototype.setScales = function (x_min, x_max, y_min, y_max) {
    this.plot.setScales(x_min, x_max, y_min, y_max);
};

DistributionPlot.prototype.setData = function (data) {
    this.plot.setData(data);
};

DistributionPlot.prototype.setHorizontalDrawDirection = function (horizontal) {
    this.horizontal_draw_direction = horizontal;
};

DistributionPlot.prototype.setCustomSettings = function (settings) {
    this.plot.setCustomSettings(settings);
};

DistributionPlot.prototype.renderNow = function(){
    this.plot.render();
};

DistributionPlot.prototype.getTitle = function(){
    return this.plot.getTitle();
};
