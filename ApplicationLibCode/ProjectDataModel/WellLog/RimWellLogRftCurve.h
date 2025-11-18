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

#include <QDateTime>
#include <map>

class RifReaderRftInterface;
class RigEclipseWellLogExtractor;
class RimEclipseCase;
class RimObservedFmuRftData;
class RimSummaryCase;
class RimSummaryEnsemble;
class RimWellPath;
class RimPressureDepthData;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogRftCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    enum class RftDataType
    {
        RFT_DATA,
        RFT_SEGMENT_DATA
    };

public:
    RimWellLogRftCurve();
    ~RimWellLogRftCurve() override;

    void    setWellName( const QString& wellName );
    QString wellName() const override;

    QString wellLogChannelUiName() const override;
    QString wellLogChannelUnits() const override;

    void      setTimeStep( const QDateTime& dateTime );
    QDateTime timeStep() const;

    void setSegmentBranchIndex( int branchIndex );
    void setSegmentBranchType( RiaDefines::RftBranchType branchType );

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCase() const;

    void            setSummaryCase( RimSummaryCase* summaryCase );
    RimSummaryCase* summaryCase() const;

    void                setEnsemble( RimSummaryEnsemble* ensemble );
    RimSummaryEnsemble* ensemble() const;

    void                   setObservedFmuRftData( RimObservedFmuRftData* observedFmuRftData );
    RimObservedFmuRftData* observedFmuRftData() const;

    void                  setPressureDepthData( RimPressureDepthData* observedFmuRftData );
    RimPressureDepthData* pressureDepthData() const;

    void                 setRftAddress( RifEclipseRftAddress address );
    RifEclipseRftAddress rftAddress() const;

    void setDefaultAddress( QString wellName );

    void setSimWellBranchData( bool branchDetection, int branchIndex );

    void enableColorFromResultName( bool enable );
    void assignColorFromResultName( const QString& resultName );

    void setScaleFactor( double factor );

protected:
    QString     createCurveAutoName() override;
    QString     createCurveNameFromTemplate( const QString& templateText ) override;
    QStringList supportedCurveNameVariables() const override;

    void                  onLoadDataAndUpdate( bool updateParentPlot ) override;
    RiaDefines::PhaseType phaseType() const override;

    // Pdm overrides
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    std::vector<QString> perPointLabels() const;

private:
    RifReaderRftInterface* rftReader() const;

    RigEclipseWellLogExtractor* extractor();

    void updateWellChannelNameAndTimeStep();

    std::map<QString, QString> createCurveNameKeyValueMap() const;

    std::vector<double> xValues();
    std::vector<double> errorValues();
    std::vector<double> tvDepthValues();
    std::vector<double> measuredDepthValues( QString& prefixText );

    bool deriveMeasuredDepthFromObservedData( const std::vector<double>& tvDepthValues, std::vector<double>& derivedMDValues );

    int segmentBranchIndex() const;

private:
    caf::PdmPtrField<RimEclipseCase*>        m_eclipseCase;
    caf::PdmPtrField<RimSummaryCase*>        m_summaryCase;
    caf::PdmPtrField<RimSummaryEnsemble*>    m_ensemble;
    caf::PdmPtrField<RimObservedFmuRftData*> m_observedFmuRftData;
    caf::PdmPtrField<RimPressureDepthData*>  m_pressureDepthData;
    caf::PdmField<QDateTime>                 m_timeStep;
    caf::PdmField<QString>                   m_wellName;
    caf::PdmField<int>                       m_branchIndex;
    caf::PdmField<bool>                      m_branchDetection;
    caf::PdmField<bool>                      m_curveColorByPhase;

    caf::PdmField<double> m_scaleFactor;

    caf::PdmField<caf::AppEnum<RimWellLogRftCurve::RftDataType>> m_rftDataType;

    caf::PdmField<QString>                                 m_segmentResultName;
    caf::PdmField<int>                                     m_segmentBranchIndex;
    caf::PdmField<caf::AppEnum<RiaDefines::RftBranchType>> m_segmentBranchType;

    caf::PdmField<caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>> m_wellLogChannelName;
};
