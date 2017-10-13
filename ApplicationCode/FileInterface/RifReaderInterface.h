/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaPorosityModel.h"

#include "cvfBase.h"
#include "cvfObject.h"

#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>

#include <vector>


class RigEclipseCaseData;
class RifReaderSettings;


//==================================================================================================
//
// Data interface base class
//
//==================================================================================================
class RifReaderInterface : public cvf::Object
{
public:
    RifReaderInterface()            { }
    virtual ~RifReaderInterface()   { }

    bool                        isFaultImportEnabled();
    bool                        isImportOfCompleteMswDataEnabled();
    bool                        isNNCsEnabled();
    const QString               faultIncludeFileAbsolutePathPrefix();

    virtual bool                open(const QString& fileName, RigEclipseCaseData* eclipseCase) = 0;
   
    virtual bool                staticResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values) = 0;
    virtual bool                dynamicResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, size_t stepIndex, std::vector<double>* values) = 0;

    void                        setFilenamesWithFaults(const std::vector<QString>& filenames)   { m_filenamesWithFaults = filenames; }
    std::vector<QString>        filenamesWithFaults()                                           { return m_filenamesWithFaults; }

    void                        setTimeStepFilter(const std::vector<size_t>& fileTimeStepIndices);

protected:
    bool                        isTimeStepIncludedByFilter(size_t timeStepIndex) const;
    size_t                      timeStepIndexOnFile(size_t timeStepIndex) const;

private:
    const RifReaderSettings*    readerSettings() const;

private:
    std::vector<QString>        m_filenamesWithFaults;
    
    std::vector<size_t>         m_fileTimeStepIndices;
};
