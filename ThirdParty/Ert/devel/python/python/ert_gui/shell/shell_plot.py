from math import ceil, sqrt, floor
import itertools
from matplotlib.patches import Rectangle

import matplotlib.pyplot as plt
from pandas import DataFrame
import pylab
import numpy
from scipy.stats import gaussian_kde


class ShellPlot(object):



    def __init__(self, name):
        super(ShellPlot, self).__init__()
        clist = plt.rcParams['axes.color_cycle']
        clist = ["#386CB0", "#7FC97F", "#FDC086", "#F0027F", "#BF5B17"]
        self.__color_cycle = itertools.cycle(clist)

        self.figure = plt.figure()
        self.figure.autofmt_xdate()
        plt.title(name)

        self.__legend_items = []
        self.__legend_labels = []


    def nextColor(self):
        return self.__color_cycle.next()


    def plotObservations(self, data, value_column, color='k'):
        data = data.dropna()
        plt.errorbar(x=data.index.values, y=data[value_column], yerr=data["STD_%s" % value_column],
                         fmt='none', ecolor=color, alpha=0.8)


    def plot(self, data, value_column, color=None, legend_label=''):
        if color is None:
            color = self.nextColor()

        data = data.reset_index()
        data = data.pivot(index="Date", columns="Realization", values=value_column)

        plt.ylabel("Value")
        plt.xlabel("Date")
        plt.xticks(rotation=30)
        lines = plt.plot_date(x=data.index.values, y=data, color=color, alpha=0.8, marker=None, linestyle="-")

        if len(lines) > 0:
            self.__legend_items.append(lines[0])
            self.__legend_labels.append(legend_label)


    def plotGenData(self, data, color=None, legend_label=''):
        if color is None:
            color = self.nextColor()

        plt.ylabel("Value")
        plt.xlabel("Index")
        plt.xticks(rotation=30)
        lines = plt.plot(data.index.values, data, color=color, alpha=0.8, marker=None, linestyle="-")

        if len(lines) > 0:
            self.__legend_items.append(lines[0])
            self.__legend_labels.append(legend_label)


    def showLegend(self):
        plt.legend(self.__legend_items, self.__legend_labels)


    def plotArea(self, data, value_column, color=None, legend_label=''):
        if color is None:
            color = self.nextColor()

        data = data.reset_index()
        data = data.pivot(index="Date", columns="Realization", values=value_column)

        df = DataFrame()

        df["Minimum"] = data.min(axis=1)
        df["Maximum"] = data.max(axis=1)

        plt.fill_between(df.index.values, df["Minimum"].values, df["Maximum"].values, alpha=0.5, color=color)
        plt.ylabel("Value")
        plt.xlabel("Date")
        plt.xticks(rotation=30)

        r = Rectangle((0, 0), 1, 1, color=color) # creates rectangle patch for legend use.

        self.__legend_items.append(r)
        self.__legend_labels.append(legend_label)


    def plotQuantiles(self, data, value_column, color=None, legend_label=''):
        if color is None:
            color = self.nextColor()

        data = data.reset_index()
        data = data.pivot(index="Date", columns="Realization", values=value_column)

        df = DataFrame()

        df["Minimum"] = data.min(axis=1)
        df["Maximum"] = data.max(axis=1)
        df["Mean"] = data.mean(axis=1)
        df["p10"] = data.quantile(0.1, axis=1)
        df["p33"] = data.quantile(0.33, axis=1)
        df["p50"] = data.quantile(0.50, axis=1)
        df["p67"] = data.quantile(0.67, axis=1)
        df["p90"] = data.quantile(0.90, axis=1)

        plt.plot(df.index.values, df["Minimum"].values, alpha=1, linestyle="--", color=color)
        plt.plot(df.index.values, df["Maximum"].values, alpha=1, linestyle="--", color=color)
        plt.plot(df.index.values, df["p50"].values, alpha=1, linestyle="--", color=color)
        plt.fill_between(df.index.values, df["p10"].values, df["p90"].values, alpha=0.3, color=color)
        plt.fill_between(df.index.values, df["p33"].values, df["p67"].values, alpha=0.5, color=color)

        plt.ylabel("Value")
        plt.xlabel("Date")
        plt.xticks(rotation=30)

        r = Rectangle((0, 0), 1, 1, color=color) # creates rectangle patch for legend use.

        self.__legend_items.append(r)
        self.__legend_labels.append(legend_label)


    def histogram(self, data, name, log_on_x=False, color=None):
        if color is None:
            color = self.nextColor()

        bins = int(ceil(sqrt(len(data.index))))

        if log_on_x:
            bins = ShellPlot._histogramLogBins(data, bins)

        plt.hist(data[name].values, alpha=0.8, bins=bins, color=color)
        plt.ylabel("Count")

        if log_on_x:
            plt.xticks(bins, ["$10^{%s}$" % (int(value) if value.is_integer() else "%.1f" % value) for value in bins]) #LaTeX formatting

    def density(self, data, name, legend_label='', color=None):
        if color is None:
            color = self.nextColor()

        values = data[name].values
        sample_range = values.max() - values.min()
        indexes = numpy.linspace(values.min() - 0.5 * sample_range, values.max() + 0.5 * sample_range, 1000)
        gkde = gaussian_kde(values)
        evaluated_gkde = gkde.evaluate(indexes)

        plt.ylabel("Density")
        lines = plt.plot(indexes, evaluated_gkde, linewidth=2, color=color)

        if len(lines) > 0:
            self.__legend_items.append(lines[0])
            self.__legend_labels.append(legend_label)


    @staticmethod
    def _histogramLogBins(data, bin_count):
        """
        @type data: pandas.DataFrame
        @rtype: int
        """
        data = data[data.columns[0]]

        min_value = int(floor(float(data.min())))
        max_value = int(ceil(float(data.max())))

        log_bin_count = max_value - min_value

        if log_bin_count < bin_count:
            next_bin_count = log_bin_count * 2

            if bin_count - log_bin_count > next_bin_count - bin_count:
                log_bin_count = next_bin_count
            else:
                log_bin_count = bin_count

        return numpy.linspace(min_value, max_value, log_bin_count)