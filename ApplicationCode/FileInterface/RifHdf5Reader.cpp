/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RifHdf5Reader.h"

#include <QStringList>
#include <QDateTime>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifHdf5Reader::RifHdf5Reader(const QString& fileName)
    : m_fileName(fileName)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifHdf5Reader::~RifHdf5Reader()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifHdf5Reader::dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values) const
{
    QStringList myProps = propertyNames();

//    if (std::find(begin(myProps), end(myProps), result) != end(myProps))
    if (myProps.indexOf(result) != -1)
    {
        for (size_t i = 0; i < 16336; i++)
        {
            values->push_back(i);
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifHdf5Reader::timeSteps() const
{
    std::vector<QDateTime> times;

    QDateTime dt;
    times.push_back(dt);
    times.push_back(dt.addDays(1));
    times.push_back(dt.addDays(2));

    return times;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RifHdf5Reader::propertyNames() const
{
    QStringList myProps;

    myProps.push_back("msj1");
    myProps.push_back("msj2");
    myProps.push_back("msj3");

    return myProps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifHdf5Reader::resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts)
{
    *resultNames = propertyNames();

    for (size_t i = 0; i < propertyNames().size(); i++)
    {
        resultDataItemCounts->push_back(16336);
    }
}
