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

#include "RiaDefines.h"
#include "RimCase.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfColor3.h"
#include "cvfObject.h"

#include <set>

class QString;

class RigCaseCellResultsData;
class RigEclipseCaseData;
class RigGridBase;
class RigMainGrid;
class RigVirtualPerforationTransmissibilities;
class RimCaseCollection;
class RimEclipseContourMapView;
class RimEclipseContourMapViewCollection;
class RimEclipseInputPropertyCollection;
class RimEclipseView;
class RimIdenticalGridCaseGroup;
class RimReservoirCellResultsStorage;

//==================================================================================================
//
// Interface for reservoirs.
//
//==================================================================================================
class RimEclipseCase : public RimCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseCase();
    ~RimEclipseCase() override;

    // Fields:
    caf::PdmChildArrayField<RimEclipseView*> reservoirViews;

    std::vector<QString> filesContainingFaults() const;
    void                 setFilesContainingFaults( const std::vector<QString>& val );

    bool         ensureReservoirCaseIsOpen();
    bool         openReserviorCase();
    virtual bool openEclipseGridFile() = 0;
    virtual bool importAsciiInputProperties( const QStringList& fileNames );

    RigEclipseCaseData*       eclipseCaseData();
    const RigEclipseCaseData* eclipseCaseData() const;
    void                      ensureDeckIsParsedForEquilData();
    cvf::Color3f              defaultWellColor( const QString& wellName );

    const RigMainGrid* mainGrid() const;

    RigCaseCellResultsData*       results( RiaDefines::PorosityModelType porosityModel );
    const RigCaseCellResultsData* results( RiaDefines::PorosityModelType porosityModel ) const;

    RimReservoirCellResultsStorage*       resultsStorage( RiaDefines::PorosityModelType porosityModel );
    const RimReservoirCellResultsStorage* resultsStorage( RiaDefines::PorosityModelType porosityModel ) const;

    RimEclipseView* createAndAddReservoirView();
    RimEclipseView* createCopyAndAddView( const RimEclipseView* sourceView );

    const RigVirtualPerforationTransmissibilities* computeAndGetVirtualPerforationTransmissibilities();

    virtual QString locationOnDisc() const { return QString(); }

    RimCaseCollection*                  parentCaseCollection();
    RimEclipseContourMapViewCollection* contourMapCollection();
    RimEclipseInputPropertyCollection*  inputPropertyCollection();

    QStringList            timeStepStrings() const override;
    QString                timeStepName( int frameIdx ) const override;
    std::vector<QDateTime> timeStepDates() const override;

    cvf::BoundingBox reservoirBoundingBox() override;
    cvf::BoundingBox activeCellsBoundingBox() const override;
    cvf::BoundingBox allCellsBoundingBox() const override;
    cvf::Vec3d       displayModelOffset() const override;

    virtual void reloadEclipseGridFile() = 0;

    double characteristicCellSize() const override;

    std::set<QString> sortedSimWellNames() const;

    void loadAndSyncronizeInputProperties( bool importGridOrFaultData );

    void ensureFaultDataIsComputed();
    bool ensureNncDataIsComputed();
    void createDisplayModelAndUpdateAllViews();

protected:
    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void updateFormationNamesData() override;

    // Internal methods
protected:
    void                 computeCachedData();
    void                 setReservoirData( RigEclipseCaseData* eclipseCase );
    std::vector<QString> additionalFiles() const;

private:
    void                    createTimeStepFormatString();
    std::vector<Rim3dView*> allSpecialViews() const override;

protected:
    caf::PdmField<bool>                                    m_flipXAxis;
    caf::PdmField<bool>                                    m_flipYAxis;
    caf::PdmChildField<RimEclipseInputPropertyCollection*> m_inputPropertyCollection;

private:
    caf::PdmField<std::vector<caf::FilePath>> m_filesContainingFaults;
    caf::PdmField<bool>                       m_releaseResultMemory;

    caf::PdmChildField<RimEclipseContourMapViewCollection*> m_contourMapCollection;

    cvf::ref<RigEclipseCaseData>    m_rigEclipseCase;
    QString                         m_timeStepFormatString;
    std::map<QString, cvf::Color3f> m_wellToColorMap;

    caf::PdmChildField<RimReservoirCellResultsStorage*> m_matrixModelResults;
    caf::PdmChildField<RimReservoirCellResultsStorage*> m_fractureModelResults;

    // Obsolete fields
protected:
    caf::PdmField<QString> m_caseName_OBSOLETE;

private:
    caf::PdmField<std::vector<QString>> m_filesContainingFaults_OBSOLETE;
};
