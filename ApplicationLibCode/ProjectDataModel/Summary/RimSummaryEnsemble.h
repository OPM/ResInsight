/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "Summary/RiaSummaryDefines.h"

#include "RifEclipseSummaryAddress.h"

#include "RigEnsembleParameter.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <QString>

#include <memory>
#include <utility>
#include <vector>

class RimSummaryCase;
class RimSummaryAddressCollection;
class RiaSummaryAddressAnalyzer;
class RimPathPatternFileSet;

//==================================================================================================
///
//==================================================================================================
class RimSummaryEnsemble : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<>                caseNameChanged;
    caf::Signal<RimSummaryCase*> caseRemoved;

public:
    RimSummaryEnsemble();
    ~RimSummaryEnsemble() override;

    void removeCase( RimSummaryCase* summaryCase, bool notifyChange = true );
    void addCase( RimSummaryCase* summaryCase );
    void replaceCases( const std::vector<RimSummaryCase*>& summaryCases );

    virtual std::vector<RimSummaryCase*> allSummaryCases() const;
    RimSummaryCase*                      firstSummaryCase() const;

    QString name() const;

    void                                        setNameTemplate( const QString& name );
    void                                        updateName( const std::set<QString>& existingEnsembleNames );
    void                                        setUsePathKey1( bool useKey1 );
    void                                        setUsePathKey2( bool useKey2 );
    virtual std::pair<std::string, std::string> nameKeys() const;
    virtual QString                             nameTemplateText() const;

    void                             setGroupingMode( RiaDefines::EnsembleGroupingMode groupingMode );
    RiaDefines::EnsembleGroupingMode groupingMode() const;

    bool                                       isEnsemble() const;
    void                                       setAsEnsemble( bool isEnsemble );
    virtual std::set<RifEclipseSummaryAddress> ensembleSummaryAddresses() const;
    virtual std::set<time_t>                   ensembleTimeSteps() const;

    void setEnsembleId( int ensembleId );
    int  ensembleId() const;
    bool hasEnsembleParameters() const;

    std::vector<RigEnsembleParameter> variationSortedEnsembleParameters( bool excludeNoVariation = false ) const;
    std::vector<std::pair<RigEnsembleParameter, double>> correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address ) const;
    std::vector<std::pair<RigEnsembleParameter, double>> correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address,
                                                                                              time_t selectedTimeStep ) const;
    std::vector<std::pair<RigEnsembleParameter, double>> parameterCorrelations( const RifEclipseSummaryAddress&  address,
                                                                                time_t                           selectedTimeStep,
                                                                                const std::vector<QString>&      selectedParameters = {},
                                                                                const std::set<RimSummaryCase*>& selectedCases = {} ) const;

    std::vector<std::pair<RigEnsembleParameter, double>>
        parameterCorrelationsAllTimeSteps( const RifEclipseSummaryAddress& address, const std::vector<QString>& selectedParameters = {} ) const;

    RigEnsembleParameter ensembleParameter( const QString& paramName ) const;
    void                 calculateEnsembleParametersIntersectionHash();

    void loadDataAndUpdate();

    static bool validateEnsembleCases( const std::vector<RimSummaryCase*> cases );
    bool        operator<( const RimSummaryEnsemble& rhs ) const;

    RiaDefines::EclipseUnitSystem unitSystem() const;

    void onCalculationUpdated();

    void updateReferringCurveSets();
    void updateReferringCurveSetsZoomAll();

    RiaSummaryAddressAnalyzer* addressAnalyzer();

    void                      computeMinMax( const RifEclipseSummaryAddress& address );
    void                      setMinMax( const RifEclipseSummaryAddress& address, double min, double max );
    std::pair<double, double> minMax( const RifEclipseSummaryAddress& address );

protected:
    virtual void onLoadDataAndUpdate();

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void buildMetaData();
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    bool isAutoNameChecked() const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 initAfterRead() override;
    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    QString nameAndItemCount() const;
    void    updateIcon();

    void updateReferringCurveSets( bool doZoomAll );

    void onCaseNameChanged( const SignalEmitter* emitter );

    void buildChildNodes();
    void clearChildNodes();

    QString ensembleDescription() const;

    void populatePathPattern();

protected:
    caf::PdmChildArrayField<RimSummaryCase*> m_cases;
    caf::PdmField<QString>                   m_name;

private:
    caf::PdmField<QString>           m_nameTemplateString;
    caf::PdmField<bool>              m_autoName;
    caf::PdmField<bool>              m_useKey1;
    caf::PdmField<bool>              m_useKey2;
    caf::PdmProxyValueField<QString> m_ensembleDescription;

    caf::PdmField<caf::AppEnum<RiaDefines::EnsembleGroupingMode>> m_groupingMode;

    caf::PdmProxyValueField<QString>                 m_nameAndItemCount;
    caf::PdmField<bool>                              m_isEnsemble;
    caf::PdmChildField<RimSummaryAddressCollection*> m_dataVectorFolders;

    caf::PdmChildField<RimPathPatternFileSet*> m_pathPatternFileSet;

    caf::PdmField<int> m_ensembleId;

    size_t m_commonAddressCount; // if different address count among cases, set to 0

    mutable std::vector<RigEnsembleParameter>  m_cachedSortedEnsembleParameters;
    std::unique_ptr<RiaSummaryAddressAnalyzer> m_analyzer;

    std::map<RifEclipseSummaryAddress, std::pair<double, double>> m_minMaxValues;
};
