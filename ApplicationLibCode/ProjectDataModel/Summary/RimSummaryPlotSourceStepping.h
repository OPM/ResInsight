/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimSummaryDataSourceStepping.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QString>

#include <map>
#include <set>

class RimSummaryCase;
class RimSummaryCurve;
class RifSummaryReaderInterface;
class RimSummaryCaseCollection;
class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RimSummaryPlotSourceStepping : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SourceSteppingDimension
    {
        SUMMARY_CASE,
        ENSEMBLE,
        WELL,
        GROUP,
        REGION,
        QUANTITY,
        BLOCK,
        AQUIFER
    };

public:
    RimSummaryPlotSourceStepping();

    void setSourceSteppingType( RimSummaryDataSourceStepping::Axis sourceSteppingType );
    void setSourceSteppingObject( caf::PdmObject* sourceObject );

    void applyNextStep();
    void applyPrevStep();

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

    RifEclipseSummaryAddress  stepAddress( RifEclipseSummaryAddress addr, int direction );
    RimSummaryCase*           stepCase( int direction );
    RimSummaryCaseCollection* stepEnsemble( int direction );

    void syncWithStepper( RimSummaryPlotSourceStepping* other );

    RimSummaryPlotSourceStepping::SourceSteppingDimension stepDimension() const;
    void setStepDimension( RimSummaryPlotSourceStepping::SourceSteppingDimension dimension );

    void updateStepIndex( int direction );

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmValueField* fieldToModify();

    std::set<RifEclipseSummaryAddress> adressesForSourceStepping() const;

    std::set<RifEclipseSummaryAddress>  addressesForCurvesInPlot() const;
    std::set<RimSummaryCase*>           summaryCasesCurveCollection() const;
    std::set<RimSummaryCaseCollection*> ensembleCollection() const;

    std::vector<caf::PdmFieldHandle*> activeFieldsForDataSourceStepping();
    std::vector<caf::PdmFieldHandle*> toolbarFieldsForDataSourceStepping();

    bool isXAxisStepping() const;
    bool isYAxisStepping() const;

    void modifyCurrentIndex( caf::PdmValueField* valueField, int indexOffset, bool notifyChange = true );

    std::vector<RimSummaryCase*> summaryCasesForSourceStepping();

    RimSummaryDataSourceStepping* dataSourceSteppingObject() const;

    std::map<QString, QString> optionsForQuantity( std::set<RifEclipseSummaryAddress> addresses );

private:
    caf::PdmPointer<caf::PdmObject> m_objectForSourceStepping;

    caf::PdmField<QString>                               m_indexLabel;
    caf::PdmField<caf::AppEnum<SourceSteppingDimension>> m_stepDimension;

    caf::PdmPtrField<RimSummaryCase*>           m_summaryCase;
    caf::PdmPtrField<RimSummaryCaseCollection*> m_ensemble;

    caf::PdmField<QString> m_wellName;
    caf::PdmField<QString> m_groupName;
    caf::PdmField<int>     m_region;
    caf::PdmField<QString> m_quantity;
    caf::PdmField<QString> m_placeholderForLabel;

    caf::PdmField<QString> m_cellBlock;
    caf::PdmField<QString> m_segment;
    caf::PdmField<QString> m_completion;

    caf::PdmField<int> m_aquifer;

    caf::PdmField<bool> m_includeEnsembleCasesForCaseStepping;

    RimSummaryDataSourceStepping::Axis m_sourceSteppingType;
};
