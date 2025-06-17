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

#include "RifReaderSettings.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfColor3.h"
#include "cvfObject.h"

#include <memory>
#include <set>

class QString;

class RimWellTargetMapping;
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
class RimEclipseResultAddressCollection;
class RimEclipseViewCollection;

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

    std::vector<RimEclipseView*> reservoirViews() const;
    RimEclipseViewCollection*    viewCollection() const;

    std::vector<QString> filesContainingFaults() const;
    void                 setFilesContainingFaults( const std::vector<QString>& val );

    bool         ensureReservoirCaseIsOpen();
    bool         openReservoirCase();
    virtual void closeReservoirCase();
    virtual bool openEclipseGridFile() = 0;
    virtual void reloadEclipseGridFile();
    virtual bool importAsciiInputProperties( const QStringList& fileNames );

    RigEclipseCaseData*       eclipseCaseData();
    const RigEclipseCaseData* eclipseCaseData() const;
    void                      ensureDeckIsParsedForEquilData();
    cvf::Color3f              defaultWellColor( const QString& wellName );

    const RigMainGrid* mainGrid() const;
    bool               isGridSizeEqualTo( const RimEclipseCase* otherCase ) const;

    RigCaseCellResultsData*       results( RiaDefines::PorosityModelType porosityModel );
    const RigCaseCellResultsData* results( RiaDefines::PorosityModelType porosityModel ) const;

    RimReservoirCellResultsStorage*       resultsStorage( RiaDefines::PorosityModelType porosityModel );
    const RimReservoirCellResultsStorage* resultsStorage( RiaDefines::PorosityModelType porosityModel ) const;

    RimEclipseView* createAndAddReservoirView( bool useGlobalViewCollection = false );
    RimEclipseView* createAndAddReservoirView( RimEclipseViewCollection* viewColl );
    RimEclipseView* createCopyAndAddView( const RimEclipseView* sourceView );

    const RigVirtualPerforationTransmissibilities* computeAndGetVirtualPerforationTransmissibilities();

    virtual QString locationOnDisc() const { return QString(); }

    RimCaseCollection*                  parentCaseCollection();
    RimEclipseContourMapViewCollection* contourMapCollection() const;
    RimEclipseInputPropertyCollection*  inputPropertyCollection() const;

    QStringList            timeStepStrings() const override;
    QString                timeStepName( int frameIdx ) const override;
    std::vector<QDateTime> timeStepDates() const override;

    cvf::BoundingBox reservoirBoundingBox() override;
    cvf::BoundingBox activeCellsBoundingBox() const override;
    cvf::BoundingBox allCellsBoundingBox() const override;
    cvf::Vec3d       displayModelOffset() const override;

    double characteristicCellSize() const override;

    std::set<QString> sortedSimWellNames() const;

    void loadAndSynchronizeInputProperties( bool importGridOrFaultData );

    void ensureFaultDataIsComputed();
    bool ensureNncDataIsComputed();
    void createDisplayModelAndUpdateAllViews();
    void computeActiveCellsBoundingBox();

    void              setReaderSettings( RifReaderSettings& readerSettings );
    RifReaderSettings readerSettings() const;

    void addWellTargetMapping( RimWellTargetMapping* wellTargetMapping );

    void updateResultAddressCollection();

    void setReservoirData( RigEclipseCaseData* eclipseCase );

protected:
    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void updateFormationNamesData() override;

    // Internal methods
protected:
    void                      computeCachedData();
    std::vector<QString>      additionalFiles() const;
    RimEclipseViewCollection* globalViewCollection() const;
    void addViewsFromViewCollection( std::vector<RimEclipseView*>& views, const RimEclipseViewCollection* viewColl ) const;

private:
    void                                   createTimeStepFormatString();
    std::vector<Rim3dView*>                allSpecialViews() const override;
    std::vector<RimEclipseContourMapView*> contourMapViews() const;

    void buildResultChildNodes();

protected:
    caf::PdmField<bool>                                    m_flipXAxis;
    caf::PdmField<bool>                                    m_flipYAxis;
    caf::PdmChildField<RimEclipseInputPropertyCollection*> m_inputPropertyCollection;

    RifReaderSettings m_readerSettings;

private:
    caf::PdmField<bool> m_releaseResultMemory;

    cvf::ref<RigEclipseCaseData>    m_rigEclipseCase;
    QString                         m_timeStepFormatString;
    std::map<QString, cvf::Color3f> m_wellToColorMap;

    caf::PdmChildArrayField<RimEclipseResultAddressCollection*> m_resultAddressCollections;

    caf::PdmChildField<RimReservoirCellResultsStorage*> m_matrixModelResults;
    caf::PdmChildField<RimReservoirCellResultsStorage*> m_fractureModelResults;
    caf::PdmChildField<RimEclipseViewCollection*>       m_viewCollection;

    caf::PdmChildArrayField<RimWellTargetMapping*> m_wellTargetMappings;

    caf::PdmField<std::vector<caf::FilePath>> m_filesContainingFaults;

    // Obsolete fields:
    caf::PdmChildArrayField<RimEclipseView*>                m_reservoirViews_OBSOLETE;
    caf::PdmChildField<RimEclipseContourMapViewCollection*> m_contourMapCollection_OBSOLETE;
};
