#  Copyright (C) 2013  Equinor ASA, Norway.
#
#  The file 'ecl_npv.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

import re
import datetime
import numbers

from ecl.util.util import monkey_the_camel
from ecl.summary import EclSum


class NPVParseKey(object):
    def __init__(self, eclNPV):
        self.baseCase = eclNPV.baseCase
        self.NPV = eclNPV


    def __call__(self, matchObject):
        key = matchObject.group(1)
        smspecNode = self.baseCase.smspec_node(key)
        if smspecNode.isTotal():
            var = key.replace(":", "_")
            self.NPV.addKey(key, var)
            return var + "[i]"
        else:
            raise ValueError("Sorry - the key: %s is not a total key - aborting" % key)


class NPVPriceVector(object):
    def __init__(self, argList):
        self.dateList = []
        if isinstance(argList, list):
            for item in argList:
                if isinstance(item, tuple) and len(item) == 2:
                    self.addItem(item)
                else:
                    raise ValueError("Each element in list must be tuple with two elements")
        else:
            raise ValueError("Constructor argument must be list")



    def add_item(self, item):
        dateItem = item[0]
        try:
            year = dateItem.year
            month = dateItem.month
            day = dateItem.day
            date = datetime.date(year, month, day)
        except AttributeError:
            try:
                tmp = re.split("[ .-/]", dateItem)
                day = int(tmp[0])
                month = int(tmp[1])
                year = int(tmp[2])

                date = datetime.date(year, month, day)
            except Exception:
                raise ValueError("First element was invalid date item")


        if len(self.dateList):
            prevItem = self.dateList[-1]
            if prevItem[0] >= date:
                raise ValueError("The dates must be in a strictly increasing order")

        value = item[1]
        if isinstance(value, numbers.Real) or callable(value):
            self.dateList.append((date, value))
        else:
            raise ValueError("The value argument must be a scalar number or callable")

    @staticmethod
    def assert_date(date):
        try:
            year = date.year
            month = date.month
            day = date.day
            date = datetime.date(year, month, day)

            return date
        except AttributeError:
            return date.date()



    def eval_date(self, dateItem, date):
        value = dateItem[1]
        if callable(value):
            td = date - dateItem[0]
            return value(td.days)
        else:
            return value


    def eval(self, date):
        date = self.assertDate(date)
        startDate = self.dateList[0][0]
        if date >= startDate:
            endDate = self.dateList[-1][0]
            if date >= endDate:
                return self.evalDate(self.dateList[-1], date)
            else:
                index1 = 0
                index2 = len(self.dateList) - 1
                while True:
                    if (index2 - index1) == 1:
                        index = index1
                        break

                    index = (index1 + index2) >> 1
                    item = self.dateList[index]
                    if date >= item[0]:
                        index1 = index
                    else:
                        index2 = index
                return self.evalDate(self.dateList[index], date)
        else:
            raise ValueError("Input date:%s before start of vector" % date)





class EclNPV(object):
    sumKeyRE = re.compile("[\[]([\w:,]+)[\]]")


    def __init__(self, baseCase):
        sum = EclSum(baseCase)
        if sum:
            self.baseCase = sum
        else:
            raise Error("Failed to open ECLIPSE sumamry case:%s" % baseCase)
        self.expression = None
        self.keyList = {}
        self.start = None
        self.end = None
        self.interval = "1Y"


    def eval(self):
        if self.expression is None:
            raise ValueError("Can not eval with an expression to evaluate")
        pass


    def get_expression(self):
        return self.expression


    def set_expression(self, expression):
        self.compiled_expr = self.compile(expression)
        self.expression = expression


    def get_key_list(self):
        return self.keyList.keys()


    def add_key(self, key, var):
        self.keyList[key] = var


    def parse_expression(self, expression):
        self.keyList = {}
        if expression.count("[") != expression.count("]"):
            raise ValueError("Expression:%s invalid - not matching [ and ]" % expression)

        replaceKey = NPVParseKey(self)
        parsedExpression = self.sumKeyRE.sub(replaceKey, expression)
        return parsedExpression


    def compile(self, expression):
        parsedExpression = self.parseExpression(expression)
        self.code = "trange = self.baseCase.timeRange(self.start, self.end, self.interval)\n"
        for (key,var) in self.keyList.items():
            self.code += "%s = self.baseCase.blockedProduction(\"%s\", trange)\n" % (var, key)

        self.code += "npv = 0\n"
        self.code += """
for i in range(len(trange) - 1):
   npv += %s
varDict[\"npv\"] = npv
""" % parsedExpression



    def eval_npv(self):
        byteCode = compile(self.code, "<string>", 'exec')
        varDict = {}
        eval(byteCode)
        return varDict["npv"]



monkey_the_camel(NPVPriceVector, 'addItem', NPVPriceVector.add_item)
monkey_the_camel(NPVPriceVector, 'assertDate', NPVPriceVector.assert_date, staticmethod)
monkey_the_camel(NPVPriceVector, 'evalDate', NPVPriceVector.eval_date)

monkey_the_camel(EclNPV, 'getExpression', EclNPV.get_expression)
monkey_the_camel(EclNPV, 'setExpression', EclNPV.set_expression)
monkey_the_camel(EclNPV, 'getKeyList', EclNPV.get_key_list)
monkey_the_camel(EclNPV, 'addKey', EclNPV.add_key)
monkey_the_camel(EclNPV, 'parseExpression', EclNPV.parse_expression)
monkey_the_camel(EclNPV, 'evalNPV', EclNPV.eval_npv)
