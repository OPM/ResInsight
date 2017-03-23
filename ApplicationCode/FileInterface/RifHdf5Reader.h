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

#pragma once

#include "RifHdf5ReaderInterface.h"

#include <QString>

//==================================================================================================
// 
//
//==================================================================================================
class RifHdf5Reader : public RifHdf5ReaderInterface
{
public:
    explicit RifHdf5Reader(const QString& fileName);
    virtual ~RifHdf5Reader();

    bool    dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values) const;


    std::vector<QDateTime> timeSteps() const;


    virtual QStringList propertyNames() const override;


    virtual void resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts) override;

private:
    QString m_fileName;
};
