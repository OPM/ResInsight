function StdPlot() {
    var radius = 1;
    var x = function(d) { return d[0];};
    var y = function(d) { return d[1];};
    var y_max = function(d) { return d[1] + 1;};
    var y_min = function(d) { return d[1] - 1;};
    var duration = 250;
    var plot_style = "";

    function createLine(selection, name, x1, x2, y1, y2) {
        var line = selection.selectAll("." + name).data(function(d) {return [d];});

        line.transition()
            .duration(duration)
            .attr("x1", x1)
            .attr("x2", x2)
            .attr("y1", y1)
            .attr("y2", y2);

        line.enter()
            .append("line")
            .attr("class", name)
            .attr("x1", x1)
            .attr("x2", x2)
            .attr("y1", y1)
            .attr("y2", y2)
            .style("opacity", 0.0)
            .transition()
            .duration(duration)
            .style("opacity", 1.0);

        line.exit()
            .transition()
            .duration(duration)
            .style("opacity", 0)
            .remove();

        return line
    }

    function plot(selection) {
        selection.enter().append("g").attr("class", plot_style);

        selection.exit().transition().duration(duration).style("opacity", 0).remove();

        var circle = selection.selectAll(".std-value").data(function(d) {return [d];});


        circle.transition()
            .duration(duration)
            .attr("cx", x)
            .attr("cy", y);

        circle.enter()
            .append("circle")
            .attr("class", "std-value")
            .attr("cx", x)
            .attr("cy", y)
            .attr("r", radius).style("opacity", 0.0)
            .transition()
            .duration(duration)
            .style("opacity", 1.0);


        circle.exit()
            .transition()
            .duration(duration)
            .style("opacity", 0)
            .remove();



        var cap_x1 = function(d) { return x(d) - radius;};
        var cap_x2 = function(d) { return x(d) + radius;};
        var y_m_r = function(d) { return y(d) - radius;};
        var y_p_r = function(d) { return y(d) + radius;};

        var top_line = createLine(selection, "top-center-line", x, x, y_m_r, y_max);
        var top_cap_line = createLine(selection, "top-cap-line", cap_x1, cap_x2, y_max, y_max);

        var bottom_line = createLine(selection, "bottom-center-line", x, x, y_p_r, y_min);
        var bottom_cap_line = createLine(selection, "bottom-cap-line", cap_x1, cap_x2, y_min, y_min);
    }


    plot.radius = function(value) {
        if (!arguments.length) return radius;
        radius = value;
        return plot;
    };

    plot.duration = function(value) {
        if (!arguments.length) return duration;
        duration = value;
        return plot;
    };

    plot.x = function(value) {
        if (!arguments.length) return x;
        x = value;
        return plot;
    };

    plot.y = function(value) {
        if (!arguments.length) return y;
        y = value;
        return plot;
    };

    plot.y_max = function(value) {
        if (!arguments.length) return y_max;
        y_max = value;
        return plot;
    };

    plot.y_min = function(value) {
        if (!arguments.length) return y_min;
        y_min = value;
        return plot;
    };

    plot.style = function(value) {
        if (!arguments.length) return plot_style;
        plot_style = value;
        return plot;
    };

    return plot;
}