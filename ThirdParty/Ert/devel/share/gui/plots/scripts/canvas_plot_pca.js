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


function PcaPlot(element, x_dimension, y_dimension) {
    this.plot = new BasePlot(element, x_dimension, y_dimension);
    this.plot.setRenderObservations(false);
    this.plot.setRenderRefcase(false);

    this.horizontal_draw_direction = true;
    this.group_scale = d3.scale.ordinal();

    var self = this;

    var preRenderCallback = function(data) {
        if (data.hasEnsembleData()) {
            if (!self.horizontal_draw_direction) {
                self.plot.setLogScaleOnDimensionX(data.shouldUseLogScale());
            } else {
                self.plot.setLogScaleOnDimensionY(data.shouldUseLogScale());
            }

            self.plot.setXDomain(data.minX(), data.maxX(), data.observationData().xValues());
            self.plot.setYDomain(data.minY(), data.maxY(), data.observationData().yValues());
        }
    };

    var renderCallback = function (context, data) {
        if (data.hasEnsembleData()) {
            var obs_data = data.observationData();
            var obs_x_values = obs_data.xValues();
            var obs_y_values = obs_data.yValues();

            var case_list = data.caseList();

            for (var i = 0; i < case_list.length; i++) {
                var style = STYLES[("ensemble_" + (i + 1))];
                var case_name = case_list[i];

                var ensemble_data = data.ensembleData(case_name);

                var x_values = ensemble_data.xValues();
                var y_values = ensemble_data.yValues();

                var realization_count = y_values.length;
                var pc_count = x_values.length;
                if (!self.horizontal_draw_direction) {
                    realization_count = x_values.length;
                    pc_count = y_values.length;
                }

                var circle_renderer = self.plot.createCircleRenderer();

                var circle_scale = circle_renderer.x().scale();
                self.group_scale.domain(case_list).rangePoints([0, circle_scale.rangeBand()], 1);

                var scaler = function(value) {
                    var pc = value[0];
                    var case_name = value[1];
                    return circle_scale(pc) + self.group_scale(case_name);
                };

                circle_renderer.x(scaler);

                circle_renderer.style(style);
                circle_renderer.fillCircle(true);
                circle_renderer.radius(5);

                for (var j = 0; j < realization_count; j++) {
                    for(var pc = 0; pc < pc_count; pc++) {
                        if (self.horizontal_draw_direction) {
                            circle_renderer(context, [x_values[pc], case_name], y_values[j][pc])

                        } else {
                            circle_renderer(context, x_values[j][pc], [y_values[pc], case_name])
                        }
                    }
                }


                var cross_renderer = self.plot.createCrossRenderer();

                cross_renderer.x(scaler);

                cross_renderer.style(STYLES["observation"]);
                cross_renderer.radius(5);


                for(var pc = 0; pc < pc_count; pc++) {
                    if (self.horizontal_draw_direction) {
                        cross_renderer(context, [obs_x_values[pc], case_name], obs_y_values[pc])

                    } else {
                        cross_renderer(context, obs_x_values[pc], [obs_y_values[pc], case_name])
                    }
                }

                self.plot.addLegend(style, case_name, CanvasPlotLegend.filledCircle);
            }

            self.plot.addLegend(STYLES["observation"], "Observation", CanvasPlotLegend.cross);

            self.plot.renderCallbackFinishedRendering();
        }
    };

    this.plot.setPreRenderCallback(preRenderCallback);
    this.plot.setRenderCallback(renderCallback);
}

PcaPlot.prototype.resize = function (width, height) {
    this.plot.resize(width, height);
};

PcaPlot.prototype.setScales = function (x_min, x_max, y_min, y_max) {
    this.plot.setScales(x_min, x_max, y_min, y_max);
};

PcaPlot.prototype.setData = function (data) {
    this.plot.setData(data);
};

PcaPlot.prototype.setHorizontalDrawDirection = function (horizontal) {
    this.horizontal_draw_direction = horizontal;
};

PcaPlot.prototype.setCustomSettings = function (settings) {
    this.plot.setCustomSettings(settings);
};

PcaPlot.prototype.renderNow = function(){
    this.plot.render();
};

PcaPlot.prototype.getTitle = function(){
    return this.plot.getTitle();
};

PcaPlot.prototype.setRenderingFinishedCallback = function(callback) {
    this.plot.setRenderingFinishedCallback(callback);
};
