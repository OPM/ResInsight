import numpy
from pandas import MultiIndex, DataFrame, pandas
from ert.test import ExtendedTestCase


class NumpyAndPandasTest(ExtendedTestCase):

    def test_numpy(self):
        data = numpy.empty(shape=(10, 10), dtype=numpy.float64)
        data.fill(numpy.nan)

        self.assertTrue(numpy.isnan(data[0][0]))
        self.assertTrue(numpy.isnan(data[9][9]))

        with self.assertRaises(IndexError):
            v = data[10][9]

        data[5][5] = 1.0

        self.assertEqual(data[5][5], 1.0)

        data[0] = 5.0

        test_data = numpy.empty(shape=10)
        test_data.fill(5.0)

        self.assertTrue(numpy.array_equal(data[0], test_data))

        data = numpy.transpose(data)

        self.assertTrue(numpy.array_equal(data[:,0], test_data))

        row = data[0]
        row[5] = 11
        self.assertEqual(data[0][5], 11)


    def test_pandas_join(self):

        multi_index = MultiIndex.from_product([[1, 2], ["A", "B", "C"]], names=["REALIZATION", "LABEL"])

        data = DataFrame(data=[[1, 2, 3], [2, 4, 6], [4, 8, 12]] * 2, index=multi_index, columns=["C1", "C2", "C3"])

        new_column = DataFrame(data=[4.0, 4.4, 4.8], index=[1, 2, 3], columns=["C4"])
        new_column.index.name = "REALIZATION"

        result = data.join(new_column, how='inner')

        self.assertFloatEqual(result["C4"][1]["A"], 4.0)
        self.assertFloatEqual(result["C4"][1]["B"], 4.0)
        self.assertFloatEqual(result["C4"][1]["C"], 4.0)

        self.assertFloatEqual(result["C4"][2]["A"], 4.4)
        self.assertFloatEqual(result["C4"][2]["B"], 4.4)
        self.assertFloatEqual(result["C4"][2]["C"], 4.4)



    def test_pandas_concatenate(self):

        d1 = DataFrame(data=[2, 4, 6, 8], columns=["A"], index=[1, 2, 3, 4])
        d2 = DataFrame(data=[[1, 1.1], [3, 3.3], [5, 5.5], [7, 7.7], [9, 9.9]], columns=["A", "B"], index=[1, 2, 3, 4, 5])

        result = pandas.concat([d1, d2], keys=[1, 2])

        self.assertEqual(result["A"][1][2], 4)
        self.assertEqual(result["A"][2][2], 3)
        self.assertTrue(numpy.isnan(result["B"][1][1]))
        self.assertFloatEqual(result["B"][2][4], 7.7)


    def test_pandas_extend_index(self):
        d1 = DataFrame(data=[2, 4, 6, 8], columns=["A"], index=[1, 2, 3, 4])
        d1.index.name = "first"

        d1["second"] = "default"
        d1.set_index(["second"], append=True, inplace=True)
        self.assertEqual(d1.index.names, ["first", "second"])

        d1 = d1.reorder_levels(["second", "first"])
        self.assertEqual(d1.index.names, ["second", "first"])
