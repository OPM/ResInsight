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

#include "RigEnsembleParameter.h"

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cvfColor3.h"

#include <QPointer>

class RiuCvfOverlayItemWidget;
class RimSummaryEnsemble;
class RimEclipseCase;
class RifReaderEnsembleStatisticsRft;
class RifReaderRftInterface;

class RimWellRftEnsembleCurveSet : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using ColorMode     = RimEnsembleCurveSetColorManager::ColorMode;
    using ColorModeEnum = RimEnsembleCurveSetColorManager::ColorModeEnum;

public:
    RimWellRftEnsembleCurveSet();
    ~RimWellRftEnsembleCurveSet() override;

    RimSummaryEnsemble* ensemble() const;
    void                setEnsemble( RimSummaryEnsemble* ensemble );
    ColorMode           colorMode() const;
    void                setColorMode( ColorMode mode );
    void                initializeLegend();
    cvf::Color3f        caseColor( const RimSummaryCase* summaryCase ) const;
    QString             currentEnsembleParameter() const;

    void                       setEnsembleParameter( const QString& parameterName );
    RimRegularLegendConfig*    legendConfig();
    RigEnsembleParameter::Type currentEnsembleParameterType() const;

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCase() const;

    RifReaderRftInterface* statisticsEclipseRftReader();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    QString              ensembleName() const;
    std::vector<QString> parametersWithVariation() const;
    void                 clearEnsembleStatistics();

private:
    caf::PdmPtrField<RimEclipseCase*>           m_eclipseCase;
    caf::PdmPtrField<RimSummaryEnsemble*>       m_ensemble;
    caf::PdmProxyValueField<QString>            m_ensembleName;
    caf::PdmField<ColorModeEnum>                m_ensembleColorMode;
    caf::PdmField<QString>                      m_ensembleParameter;
    caf::PdmChildField<RimRegularLegendConfig*> m_ensembleLegendConfig;

    std::unique_ptr<RifReaderEnsembleStatisticsRft> m_statisticsEclipseRftReader;

protected:
    void initAfterRead() override;
};
