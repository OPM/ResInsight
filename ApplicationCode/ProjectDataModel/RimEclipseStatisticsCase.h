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

#include "RimEclipseCase.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafAppEnum.h"
#include "cvfCollection.h"

class RigMainGrid;
class RigSimWellData;
class RimEclipseResultDefinition;
class RimEclipseStatisticsCaseCollection;
class RimIdenticalGridCaseGroup;


//==================================================================================================
//
// 
//
//==================================================================================================
class RimEclipseStatisticsCase : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseStatisticsCase();
    ~RimEclipseStatisticsCase() override;

    void setMainGrid(RigMainGrid* mainGrid);

    void computeStatistics();
    bool hasComputedStatistics() const;
    void clearComputedStatistics();
    void computeStatisticsAndUpdateViews();

    void updateConnectedEditorsAndReservoirViews();

    bool openEclipseGridFile() override;
    void reloadEclipseGridFile() override;

    RimCaseCollection* parentStatisticsCaseCollection() const;

    enum PercentileCalcType
    {
        NEAREST_OBSERVATION,
        HISTOGRAM_ESTIMATED,
        INTERPOLATED_OBSERVATION
    };

    caf::PdmField< bool >                                          m_calculateEditCommand;
    void  updateFilePathsFromProjectPath(const QString& projectPath, const QString& oldProjectPath) override{}

    void populateResultSelectionAfterLoadingGrid();
 
private:
    void scheduleACTIVEGeometryRegenOnReservoirViews();

    RimIdenticalGridCaseGroup* caseGroup() const;
    std::vector<RimEclipseCase*> getSourceCases() const;

    void populateResultSelection();

    void updateSelectionListVisibilities();
    void updateSelectionSummaryLabel();
    void updatePercentileUiVisibility();

    void setWellResultsAndUpdateViews(const cvf::Collection<RigSimWellData>& sourceCaseSimWellData);

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override ;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly ) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute ) override;

private:
    caf::PdmField< caf::AppEnum< RiaDefines::ResultCatType > >      m_resultType;
    caf::PdmField< caf::AppEnum< RiaDefines::PorosityModelType > >  m_porosityModel;

    caf::PdmField<QString>                                          m_selectionSummary;

    caf::PdmField<std::vector<QString> >                            m_selectedDynamicProperties;
    caf::PdmField<std::vector<QString> >                            m_selectedStaticProperties;
    caf::PdmField<std::vector<QString> >                            m_selectedGeneratedProperties;
    caf::PdmField<std::vector<QString> >                            m_selectedInputProperties;

    caf::PdmField<std::vector<QString> >                            m_selectedFractureDynamicProperties;
    caf::PdmField<std::vector<QString> >                            m_selectedFractureStaticProperties;
    caf::PdmField<std::vector<QString> >                            m_selectedFractureGeneratedProperties;
    caf::PdmField<std::vector<QString> >                            m_selectedFractureInputProperties;

 
    caf::PdmField< bool >                                           m_calculatePercentiles;
    caf::PdmField< caf::AppEnum< PercentileCalcType > >             m_percentileCalculationType;
    caf::PdmField<double >                                          m_lowPercentile;
    caf::PdmField<double >                                          m_midPercentile;
    caf::PdmField<double >                                          m_highPercentile;

    caf::PdmField<QString>                                          m_wellDataSourceCase;

    caf::PdmField< bool >                                           m_useZeroAsInactiveCellValue;

    bool                                                            m_populateSelectionAfterLoadingGrid;
};
