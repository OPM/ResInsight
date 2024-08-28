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

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RifReaderSettings.h"

#include "cvfCollection.h"
#include "cvfObject.h"

#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>

#include <memory>
#include <set>
#include <vector>

class RigEclipseCaseData;
class RigFault;

//==================================================================================================
//
// Data interface base class
//
//==================================================================================================
class RifReaderInterface : public cvf::Object
{
public:
    RifReaderInterface();
    ~RifReaderInterface() override {}

    bool          isFaultImportEnabled() const;
    bool          isImportOfCompleteMswDataEnabled() const;
    bool          isNNCsEnabled() const;
    bool          includeInactiveCellsInFaultGeometry() const;
    bool          loadWellDataEnabled() const;
    const QString faultIncludeFileAbsolutePathPrefix() const;
    bool          onlyLoadActiveCells() const;

    void setReaderSettings( RifReaderSettings readerSettings );

    virtual bool open( const QString& fileName, RigEclipseCaseData* eclipseCase ) = 0;

    virtual bool staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values ) = 0;
    virtual bool dynamicResult( const QString&                result,
                                RiaDefines::PorosityModelType matrixOrFracture,
                                size_t                        stepIndex,
                                std::vector<double>*          values )                                                                       = 0;

    void                 setFilenamesWithFaults( const std::vector<QString>& filenames ) { m_filenamesWithFaults = filenames; }
    std::vector<QString> filenamesWithFaults() { return m_filenamesWithFaults; }

    void setTimeStepFilter( const std::vector<size_t>& fileTimeStepIndices );

    virtual std::set<RiaDefines::PhaseType> availablePhases() const;

    virtual void updateFromGridCount( size_t gridCount ){};

protected:
    bool   isTimeStepIncludedByFilter( size_t timeStepIndex ) const;
    size_t timeStepIndexOnFile( size_t timeStepIndex ) const;
    void   importFaults( const QStringList& fileSet, cvf::Collection<RigFault>* faults );

private:
    std::vector<QString> m_filenamesWithFaults;

    std::vector<size_t> m_fileTimeStepIndices;

    RifReaderSettings m_readerSettings;
};
