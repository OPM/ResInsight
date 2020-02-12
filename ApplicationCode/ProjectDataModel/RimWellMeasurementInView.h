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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <vector>

class RimWellMeasurement;
class RimRegularLegendConfig;
class RimWellPath;
class RimWellPathCollection;
class RiuViewer;

class RimWellMeasurementInView : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurementInView();
    ~RimWellMeasurementInView() override;

    RimRegularLegendConfig* legendConfig();
    QString                 measurementKind() const;
    void                    setMeasurementKind( const QString& measurementKind );
    bool                    isWellChecked( const QString& wellName ) const;
    void                    setAllWellsSelected();
    void                    setAllQualitiesSelected();

    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

    bool hasCategoryResult() const;

    void             rangeValues( double* lowerBound, double* upperBound ) const;
    std::vector<int> qualityFilter() const;
    double           radiusScaleFactor() const;

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    bool        updateLegendData();
    static bool hasMeasurementKindForWell( const RimWellPath*                      wellPath,
                                           const RimWellPathCollection*            wellPathCollection,
                                           const std::vector<RimWellMeasurement*>& measurements,
                                           const QString&                          measurementKind );

private:
    static QString              convertToSerializableString( const std::vector<QString>& strings );
    static std::vector<QString> convertFromSerializableString( const QString& string );

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;
    caf::PdmField<QString>                      m_measurementKind;
    caf::PdmField<std::vector<QString>>         m_wells;
    caf::PdmField<double>                       m_lowerBound;
    caf::PdmField<double>                       m_upperBound;
    caf::PdmField<std::vector<int>>             m_qualityFilter;
    caf::PdmField<QString>                      m_wellsSerialized;
    caf::PdmField<QString>                      m_availableWellsSerialized;
    caf::PdmField<double>                       m_radiusScaleFactor;

    void              selectNewWells( const std::set<QString>& wells );
    void              setAvailableWells( const std::set<QString>& wells );
    std::set<QString> getAvailableWells() const;

    double m_minimumResultValue;
    double m_maximumResultValue;
};
