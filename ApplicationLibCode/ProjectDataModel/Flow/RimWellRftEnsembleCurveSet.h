/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimEnsembleCurveSetColorManager.h"

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cvfColor3.h"

class RiuCvfOverlayItemWidget;
class RimSummaryEnsemble;
class RimEclipseCase;
class RifReaderEnsembleStatisticsRft;
class RifReaderRftInterface;
class RimCurveSetAppearance;
class RimSummaryCase;
class RimRegularLegendConfig;

class RimWellRftEnsembleCurveSet : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellRftEnsembleCurveSet();
    ~RimWellRftEnsembleCurveSet() override;

    void                setEnsemble( RimSummaryEnsemble* ensemble );
    RimSummaryEnsemble* ensemble() const;
    QString             ensembleName() const;

    void         setCurveColor( const cvf::Color3f& color );
    cvf::Color3f curveColor( RimSummaryEnsemble* ensemble, const RimSummaryCase* summaryCase ) const;

    RimRegularLegendConfig* legendConfig();

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCase() const;

    RifReaderRftInterface* statisticsEclipseRftReader();
    void                   clearEnsembleStatistics();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 initAfterRead() override;

    void updatePlot( const SignalEmitter* emitter );

private:
    caf::PdmPtrField<RimEclipseCase*>          m_eclipseCase;
    caf::PdmPtrField<RimSummaryEnsemble*>      m_ensemble;
    caf::PdmProxyValueField<QString>           m_ensembleName;
    caf::PdmChildField<RimCurveSetAppearance*> m_appearance;

    std::unique_ptr<RifReaderEnsembleStatisticsRft> m_statisticsEclipseRftReader;

    caf::PdmField<RimEnsembleCurveSetColorManager::ColorModeEnum> m_ensembleColorMode_OBSOLETE;
    caf::PdmField<QString>                                        m_ensembleParameter_OBSOLETE;
};
