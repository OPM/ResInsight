// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'canvas_plot.js' is part of ERT - Ensemble based Reservoir Tool.
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


function Plot(element, x_dimension, y_dimension) {
    this.plot = new BasePlot(element, x_dimension, y_dimension);

    this.horizontal_draw_direction = true;
    this.waiting_for_render_restart = false;


    this.line_renderers = [];
    for (var index = 1; index <= 5; index++) {
        var renderer = this.plot.createLineRenderer();
        renderer.style(STYLES["ensemble_" + (index)]);
        this.line_renderers.push(renderer)
    }

    this.circle_renderers = [];
    for (var index = 1; index <= 5; index++) {
        var renderer = this.plot.createCircleRenderer();
        renderer.style(STYLES["ensemble_" + (index)]);
        this.circle_renderers.push(renderer)
    }

    this.tracker = new IncrementalRenderTracker();

    var self = this;

    var progressivePreRenderer = function (context, data) {
        var case_list = data.caseList();

        for (var case_index = 0; case_index < case_list.length; case_index++) {
            var style = STYLES["ensemble_" + (case_index + 1)];
            var case_name = case_list[case_index];

            self.plot.addLegend(style, case_name, CanvasPlotLegend.simpleLine);
        }

        self.progressiveRenderer(context, data, self, 0, 0);
    };

    var renderEnsembleProgressively = function (context, data) {
        if (data.hasEnsembleData()) {
            self.tracker.start(function () {
                progressivePreRenderer(context, data);
            });
        }
    };

    this.progressiveRenderer = function (context, data, self, case_index, realization) {
        if (self.tracker.shouldStop()) {
            self.tracker.stoppedRendering();
            return;
        }

        var case_list = data.caseList();
        var case_name = case_list[case_index];
        var line_renderer = self.line_renderers[case_index];
        var circle_renderer = self.circle_renderers[case_index];

        var ensemble_data = data.ensembleData(case_name);
        var x_values = ensemble_data.xValues();
        var y_values = ensemble_data.yValues();

        var realization_count = y_values.length;
        if (!self.horizontal_draw_direction) {
            realization_count = x_values.length;
        }

        self.tracker.loopStart();
        for (var i = realization; i < realization_count; i++) {
            if (self.horizontal_draw_direction) {
                if(x_values.length == 1) {
                    circle_renderer(context, x_values[0], y_values[i][0])
                } else {
                    line_renderer(context, x_values, y_values[i]);
                }

            } else {
                if(y_values.length == 1) {
                    circle_renderer(context, x_values[i][0], y_values[0])
                } else {
                    line_renderer(context, x_values[i], y_values);
                }
            }

            realization++;

            if (self.tracker.shouldLoopStop()) {
                break;
            }
        }

        if (realization == realization_count) {
            case_index++;
            realization = 0;
        }

        if (case_index < case_list.length) {
            window.setTimeout(function () {
                self.progressiveRenderer(context, data, self, case_index, realization);
            }, 15);
        } else {
            self.tracker.stoppedRendering();
            self.plot.renderCallbackFinishedRendering();
        }
    };


    var renderEnsembleDirect = function(context, data) {
        if(data.hasEnsembleData()) {
            var case_list = data.caseList();

            for(var case_index = 0; case_index < case_list.length; case_index++) {
                var style = STYLES["ensemble_" + (case_index + 1)];
                var case_name = case_list[case_index];
                var line_renderer = self.line_renderers[case_index];
                var circle_renderer = self.circle_renderers[case_index];

                var ensemble_data = data.ensembleData(case_name);
                var x_values = ensemble_data.xValues();
                var y_values = ensemble_data.yValues();

                var realization_count = y_values.length;
                if (!self.horizontal_draw_direction) {
                    realization_count = x_values.length;
                }


                for (var i = 0; i < realization_count; i++) {
                    if (self.horizontal_draw_direction) {
                        if(x_values.length == 1) {
                            circle_renderer(context, x_values[0], y_values[i][0])
                        } else {
                            line_renderer(context, x_values, y_values[i]);
                        }

                    } else {
                        if(y_values.length == 1) {
                            circle_renderer(context, x_values[i][0], y_values[0])
                        } else {
                            line_renderer(context, x_values[i], y_values);
                        }
                    }

                }

                self.plot.addLegend(style, case_name, CanvasPlotLegend.simpleLine);
            }
            self.plot.renderCallbackFinishedRendering();
        }
    };

    this.plot.setRenderCallback(renderEnsembleProgressively);
    //this.plot.setRenderCallback(renderEnsembleDirect);
}

Plot.prototype.resize = function (width, height) {
    this.plot.resize(width, height);
};

Plot.prototype.setScales = function (x_min, x_max, y_min, y_max) {
    this.plot.setScales(x_min, x_max, y_min, y_max);
};

Plot.prototype.setData = function (data) {
    this.plot.setData(data);
};

Plot.prototype.setVerticalErrorBar = function (vertical) {
    this.plot.setVerticalErrorBar(vertical);
};

Plot.prototype.setHorizontalDrawDirection = function (horizontal) {
    this.horizontal_draw_direction = horizontal;
};

Plot.prototype.setCustomSettings = function (settings) {
    this.plot.setCustomSettings(settings);
};

Plot.prototype.setRenderingFinishedCallback = function(callback) {
    this.plot.setRenderingFinishedCallback(callback);
};

Plot.prototype.renderNow = function(){
    if(!this.tracker.isRunning()) {
        this.waiting_for_render_restart = false;
        this.plot.render();
    } else {
        if(!this.waiting_for_render_restart) {
            this.tracker.forceStop();
            this.waiting_for_render_restart = true;

            window.setTimeout(function () {
                self.renderNow();
            }, 15);
        }
    }
};

Plot.prototype.getTitle = function(){
    return this.plot.getTitle();
};
