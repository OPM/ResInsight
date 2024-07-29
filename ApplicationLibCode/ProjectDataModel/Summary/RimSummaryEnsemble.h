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

    void                                 removeCase( RimSummaryCase* summaryCase, bool notifyChange = true );
    void                                 addCase( RimSummaryCase* summaryCase );
    virtual std::vector<RimSummaryCase*> allSummaryCases() const;
    RimSummaryCase*                      firstSummaryCase() const;

    void    setName( const QString& name );
    QString name() const;
    void    ensureNameIsUpdated();

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

    RiaSummaryAddressAnalyzer* addressAnalyzer();

    void                      computeMinMax( const RifEclipseSummaryAddress& address );
    void                      setMinMax( const RifEclipseSummaryAddress& address, double min, double max );
    std::pair<double, double> minMax( const RifEclipseSummaryAddress& address );

private:
    caf::PdmFieldHandle* userDescriptionField() override;
    QString              nameAndItemCount() const;
    void                 updateIcon();

    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onCaseNameChanged( const SignalEmitter* emitter );

    void buildChildNodes();
    void clearChildNodes();

protected:
    virtual void onLoadDataAndUpdate();
    void         defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void         defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void buildMetaData();

    caf::PdmChildArrayField<RimSummaryCase*> m_cases;

private:
    caf::PdmField<QString>                           m_name;
    caf::PdmField<bool>                              m_autoName;
    caf::PdmProxyValueField<QString>                 m_nameAndItemCount;
    caf::PdmField<bool>                              m_isEnsemble;
    caf::PdmChildField<RimSummaryAddressCollection*> m_dataVectorFolders;

    caf::PdmField<int> m_ensembleId;

    size_t m_commonAddressCount; // if different address count among cases, set to 0

    mutable std::vector<RigEnsembleParameter>  m_cachedSortedEnsembleParameters;
    std::unique_ptr<RiaSummaryAddressAnalyzer> m_analyzer;

    std::map<RifEclipseSummaryAddress, std::pair<double, double>> m_minMaxValues;
};
