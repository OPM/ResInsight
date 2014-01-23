// Copyright (C) 2013 Statoil ASA, Norway.
//
// The file 'link_list.js' is part of ERT - Ensemble based Reservoir Tool.
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

function DataTypeList(element, data, max_width, max_height) {
    var margin = {top:10, right:10, bottom:10, left:10};
    var width = max_width - margin.left - margin.right;
    var height = max_height - margin.top - margin.bottom;

    this.root_elemenet = element;
    this.click_function = null;

    var title = this.root_elemenet
        .append("div")
        .style("width", max_width + "px")
        .attr("class", "list-title")
        .text("Data types");

    this.table = this.root_elemenet
        .append("div")
        .style("overflow-y", "scroll")
        .style("overflow-x", "hidden")
        .style("margin", "0px")
        .style("padding", "0px")
        .style("width", max_width + "px")
        .style("height", height + "px")
        .append("table")
        .style("width", (max_width - 30) + "px")
        .style("table-layout", "fixed");

    this.fillTable(data);
}

DataTypeList.prototype.fillTable = function(data) {
    var self = this;

    var tbody = this.table.append("tbody");

    var rows = tbody.selectAll(".list-item")
        .data(data)
        .enter()
        .append("tr");

    rows.append("td")
        .attr("class", "list-item")
        .on("click", function(d, i) {
            if (self.click_function != null) {
                self.click_function(d["name"]);
            }
        })
        .text(function (d) {
            if(d["highlight"]) {
                return "* " + d["name"];
            }
            return d["name"];
        });

};

DataTypeList.prototype.updateList = function (data) {
    this.table.select("tbody").remove("tbody");
    this.fillTable(data);
};

DataTypeList.prototype.setClickFunction = function(click_function) {
    this.click_function = click_function;
};

