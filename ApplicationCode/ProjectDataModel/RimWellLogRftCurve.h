/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimWellLogCurve.h"

#include "RifEclipseRftAddress.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

#include <map>

class RifReaderRftInterface;
class RigEclipseWellLogExtractor;
class RimEclipseResultCase;
class RimObservedFmuRftData;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimWellPath;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogRftCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    enum DerivedMDSource
    {
        NO_SOURCE,
        WELL_PATH,
        OBSERVED_DATA
    };

public:
    RimWellLogRftCurve();
    ~RimWellLogRftCurve() override;

    QString wellName() const override;
    QString wellLogChannelName() const override;

    void                  setEclipseResultCase( RimEclipseResultCase* eclipseResultCase );
    RimEclipseResultCase* eclipseResultCase() const;

    void            setSummaryCase( RimSummaryCase* summaryCase );
    RimSummaryCase* summaryCase() const;

    void                      setEnsemble( RimSummaryCaseCollection* ensemble );
    RimSummaryCaseCollection* ensemble() const;

    void                   setObservedFmuRftData( RimObservedFmuRftData* observedFmuRftData );
    RimObservedFmuRftData* observedFmuRftData() const;

    void                 setRftAddress( RifEclipseRftAddress address );
    RifEclipseRftAddress rftAddress() const;

    void setDefaultAddress( QString wellName );
    void updateWellChannelNameAndTimeStep();

    void setSimWellBranchData( bool branchDetection, int branchIndex );

protected:
    // Overrides from RimWellLogPlotCurve
    QString createCurveAutoName() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    // Pdm overrrides
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue ) override;

    std::vector<QString> perPointLabels() const;

private:
    RifReaderRftInterface* rftReader() const;

    RigEclipseWellLogExtractor* extractor();

    bool                createWellPathIdxToRftFileIdxMapping();
    size_t              rftFileIndex( size_t wellPathIndex );
    std::vector<size_t> sortedIndicesInRftFile();

    std::vector<double> xValues();
    std::vector<double> errorValues();
    std::vector<double> tvDepthValues();
    std::vector<double> measuredDepthValues();

    bool deriveMeasuredDepthValuesFromWellPath( const std::vector<double>& tvDepthValues,
                                                std::vector<double>&       derivedMDValues );
    bool deriveMeasuredDepthFromObservedData( const std::vector<double>& tvDepthValues,
                                              std::vector<double>&       derivedMDValues );

private:
    caf::PdmPtrField<RimEclipseResultCase*>     m_eclipseResultCase;
    caf::PdmPtrField<RimSummaryCase*>           m_summaryCase;
    caf::PdmPtrField<RimSummaryCaseCollection*> m_ensemble;
    caf::PdmPtrField<RimObservedFmuRftData*>    m_observedFmuRftData;
    caf::PdmField<QDateTime>                    m_timeStep;
    caf::PdmField<QString>                      m_wellName;
    caf::PdmField<int>                          m_branchIndex;
    caf::PdmField<bool>                         m_branchDetection;

    std::map<size_t, size_t>                                                 m_idxInWellPathToIdxInRftFile;
    caf::PdmField<caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>> m_wellLogChannelName;
};
