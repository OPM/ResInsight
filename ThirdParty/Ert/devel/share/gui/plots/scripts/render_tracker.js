// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'render_tracker.js' is part of ERT - Ensemble based Reservoir Tool.
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

function IncrementalRenderTracker() {
    this.rendering_start = null;
    this.max_loop_time = 500;
    this.is_running = false;
    this.should_stop = false;

    this.loop_start = null;

}

IncrementalRenderTracker.prototype.start = function(render_function) {
    if(this.is_running) {
//        console.log("Rendering -> activating kill!");
        this.should_stop = true;
    }

    this.run(this, render_function);
};


IncrementalRenderTracker.prototype.run = function(self, render_function) {
    if(!this.is_running) {
        this.rendering_start = Date.now();
        this.is_running = true;
        this.should_stop = false;

        render_function();

    } else {
//        console.log("Waiting!");
        window.setTimeout(function() { self.run(self, render_function); }, 15);
    }
};


IncrementalRenderTracker.prototype.shouldStop = function() {
    return this.should_stop;
};


IncrementalRenderTracker.prototype.loopStart = function() {
    this.loop_start = Date.now();
};

IncrementalRenderTracker.prototype.shouldLoopStop = function() {
    return (Date.now() - this.loop_start) > this.max_loop_time;
};

IncrementalRenderTracker.prototype.stoppedRendering = function() {
    this.is_running = false;
    this.should_stop = false;
//    console.log("Rendering time: " + this.runningTime() + " ms");
};

IncrementalRenderTracker.prototype.isRunning = function() {
    return this.is_running;
};

IncrementalRenderTracker.prototype.runningTime = function() {
    return (Date.now() - this.rendering_start);
};

IncrementalRenderTracker.prototype.forceStop = function() {
    this.should_stop = true;
};











