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

#include "H5Cpp.h"


//==================================================================================================
// 
//
//==================================================================================================
class RifHdf5Reader : public RifHdf5ReaderInterface
{
public:
    explicit RifHdf5Reader(const QString& fileName);
    virtual ~RifHdf5Reader();

    std::vector<QDateTime>  timeSteps() const override;
    virtual QStringList     propertyNames() const override;
    bool                    dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values) const override;

private:
	int                      getIntAttribute(H5::H5File file, std::string groupName, std::string attributeName) const;
	double                   getDoubleAttribute(H5::H5File file, std::string groupName, std::string attributeName) const;
	std::string              getStringAttribute(H5::H5File file, std::string groupName, std::string attributeName) const;
	std::vector<std::string> getSubGroupNames(H5::H5File file, std::string baseGroupName) const;
	std::vector<double>      getStepTimeValues(H5::H5File file, std::string baseGroupName) const;
	std::vector<std::string> getResultNames(H5::H5File file, std::string baseGroupName) const;
	void                     getElementResultValues(H5::H5File file, std::string groupName, std::vector<double>* resultValues) const;

private:
    QString m_fileName;
};
