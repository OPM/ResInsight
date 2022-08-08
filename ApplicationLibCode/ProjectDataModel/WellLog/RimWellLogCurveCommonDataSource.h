/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafPdmUiOrdering.h"
#include "cafTristate.h"

#include <QDateTime>

class RimCase;
class RimWellLogCurve;
class RimWellLogPlot;
class RimWellLogTrack;
class RimWellPath;
class RimSummaryCase;
class RifReaderRftInterface;

//==================================================================================================
///
//==================================================================================================
class RimWellLogCurveCommonDataSource : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    class DoubleComparator
    {
    public:
        DoubleComparator( double eps = 1.0e-6 );
        bool operator()( const double& lhs, const double& rhs ) const;

    private:
        double m_eps;
    };

    RimWellLogCurveCommonDataSource();

    void setCaseType( RiaDefines::CaseType caseType );

    RimCase*        caseToApply() const;
    void            setCaseToApply( RimCase* val );
    RimSummaryCase* summaryCaseToApply() const;
    void            setSummaryCaseToApply( RimSummaryCase* val );
    int             trajectoryTypeToApply() const;
    void            setTrajectoryTypeToApply( int val );
    RimWellPath*    wellPathToApply() const;
    void            setWellPathToApply( RimWellPath* val );
    int             branchIndexToApply() const;
    void            setBranchIndexToApply( int val );
    caf::Tristate   branchDetectionToApply() const;
    void            setBranchDetectionToApply( caf::Tristate::State val );
    caf::Tristate   wbsSmoothingToApply() const;
    void            setWbsSmoothingToApply( caf::Tristate::State val );
    double          wbsSmoothingThreshold() const;
    void            setWbsSmoothingThreshold( double smoothingThreshold );

    QString simWellNameToApply() const;
    void    setSimWellNameToApply( const QString& val );
    int     timeStepToApply() const;
    void    setTimeStepToApply( int val );

    void resetDefaultOptions();
    void analyseCurvesAndTracks( const std::vector<RimWellLogCurve*>& curves, const std::vector<RimWellLogTrack*>& tracks );
    void analyseCurvesAndTracks();
    void applyDataSourceChanges( const std::vector<RimWellLogCurve*>& curves, const std::vector<RimWellLogTrack*>& tracks );
    void applyDataSourceChanges();
    void applyPrevCase();
    void applyNextCase();

    void applyPrevWell();
    void applyNextWell();

    void                              applyPrevTimeStep();
    void                              applyNextTimeStep();
    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

    static QString smoothingUiOrderinglabel();

private:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          modifyCurrentIndex( caf::PdmValueField* field, int indexOffset );

    RifReaderRftInterface* rftReader();

private:
    RiaDefines::CaseType m_caseType;

    caf::PdmPtrField<RimCase*>        m_case;
    caf::PdmPtrField<RimSummaryCase*> m_summaryCase;
    caf::PdmField<int>                m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>    m_wellPath;
    caf::PdmField<QString>            m_simWellName;
    caf::PdmField<int>                m_branchIndex;
    caf::PdmField<caf::Tristate>      m_branchDetection;
    caf::PdmField<int>                m_timeStep;
    caf::PdmField<caf::Tristate>      m_wbsSmoothing;
    caf::PdmField<double>             m_wbsSmoothingThreshold;

    caf::PdmField<QDateTime> m_rftTimeStep;
    caf::PdmField<QString>   m_rftWellName;
    caf::PdmField<int>       m_rftSegmentBranchIndex;

    std::set<RimCase*>                 m_uniqueCases;
    std::set<RimSummaryCase*>          m_uniqueSummaryCases;
    std::set<int>                      m_uniqueTrajectoryTypes;
    std::set<RimWellPath*>             m_uniqueWellPaths;
    std::set<QString>                  m_uniqueWellNames;
    std::set<int>                      m_uniqueTimeSteps;
    std::set<bool>                     m_uniqueBranchDetection;
    std::set<int>                      m_uniqueBranchIndices;
    std::set<bool>                     m_uniqueWbsSmoothing;
    std::set<double, DoubleComparator> m_uniqueWbsSmoothingThreshold;

    std::set<QDateTime> m_uniqueRftTimeSteps;
    std::set<QString>   m_uniqueRftWellNames;
    std::set<int>       m_uniqueRftBranchIndices;
};
