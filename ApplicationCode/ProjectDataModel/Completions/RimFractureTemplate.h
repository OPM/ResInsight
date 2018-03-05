/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RiaEclipseUnitTools.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

class RigFractureGrid;
class RimFractureContainment;
class MinMaxAccumulator;
class PosNegAccumulator;

class FractureWidthAndConductivity
{
public:
    FractureWidthAndConductivity()
        : m_width(0.0)
        , m_permeability(0.0)
    {
    }

    bool isValid() const
    {
        if (m_width != 0.0) return true;
        if (m_permeability != 0.0) return true;

        return false;
    }

    // Unit : meter or feet
    double m_width;

    // Unit : mD (milliDarcy)
    double m_permeability;
};

//==================================================================================================
///  
///  
//==================================================================================================
class RimFractureTemplate : public caf::PdmObject 
{
     CAF_PDM_HEADER_INIT;

public:
    enum FracOrientationEnum
    {
        AZIMUTH,
        ALONG_WELL_PATH,
        TRANSVERSE_WELL_PATH
    };

    enum FracConductivityEnum
    {
        INFINITE_CONDUCTIVITY,
        FINITE_CONDUCTIVITY,
    };

    enum PermeabilityEnum
    {
        USER_DEFINED_PERMEABILITY,
        CONDUCTIVITY_FROM_FRACTURE,
    };

    enum WidthEnum
    {
        USER_DEFINED_WIDTH,
        WIDTH_FROM_FRACTURE,
    };

    enum NonDarcyFlowEnum
    {
        NON_DARCY_NONE,
        NON_DARCY_COMPUTED,
        NON_DARCY_USER_DEFINED,
    };

public:
    RimFractureTemplate();
    virtual ~RimFractureTemplate();

    int                             id() const;
    QString                         name() const;
    QString                         nameAndUnit() const;

    RiaEclipseUnitTools::UnitSystemType fractureTemplateUnit() const;
    FracOrientationEnum             orientationType() const;
    float                           azimuthAngle() const;
    float                           skinFactor() const;
    double                          wellDiameter();
    FracConductivityEnum            conductivityType() const;
    double                          perforationLength();

    virtual void                    fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords,
                                                             std::vector<cvf::uint>*  triangleIndices) = 0;

    virtual std::vector<cvf::Vec3f> fractureBorderPolygon() = 0;
    virtual const RigFractureGrid*  fractureGrid() const = 0;
    const RimFractureContainment*   fractureContainment();

    virtual void                    appendDataToResultStatistics(const QString&     resultName,
                                                                 const QString&     unit,
                                                                 MinMaxAccumulator& minMaxAccumulator,
                                                                 PosNegAccumulator& posNegAccumulator) const = 0;

    virtual std::vector<std::pair<QString, QString>> uiResultNamesWithUnit() const = 0;

    void                            setName(const QString& name);
    void                            setFractureTemplateUnit(RiaEclipseUnitTools::UnitSystemType unitSystem);
    void                            setDefaultWellDiameterFromUnit();

    bool                            isNonDarcyFlowEnabled() const;
    double                          dFactor() const;
    double                          kh() const;

    virtual void                    convertToUnitSystem(RiaEclipseUnitTools::UnitSystem neededUnit);

    virtual void                    loadDataAndUpdate() = 0;

    void                            disconnectAllFracturesAndRedrawViews() const;
    void                            setId(int id);
    void                            setScaleFactors(double width, double height, double dFactor);
    virtual void                    reload() {}

protected:
    virtual caf::PdmFieldHandle*    userDescriptionField() override;
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

private:
    void                            prepareFieldsForUiDisplay();
    virtual FractureWidthAndConductivity widthAndConductivityAtWellPathIntersection() const = 0;

    QString                         dFactorSummary() const;
    double                          effectivePermeability() const;

    double                          fractureWidth() const;

protected:
    caf::PdmField<int>                                 m_id;
    caf::PdmField<QString>                             m_name;
    caf::PdmProxyValueField<QString>                   m_nameAndUnit;

    caf::PdmField<RiaEclipseUnitTools::UnitSystemType> m_fractureTemplateUnit;
    caf::PdmField<caf::AppEnum<FracOrientationEnum>>   m_orientationType;
    caf::PdmField<float>                               m_azimuthAngle;
    caf::PdmField<float>                               m_skinFactor;
    caf::PdmField<double>                              m_perforationLength;
    caf::PdmField<double>                              m_perforationEfficiency;
    caf::PdmField<double>                              m_wellDiameter;
    caf::PdmField<caf::AppEnum<FracConductivityEnum>>  m_conductivityType;
    caf::PdmChildField<RimFractureContainment*>        m_fractureContainment;

    caf::PdmField<caf::AppEnum<NonDarcyFlowEnum>>      m_nonDarcyFlowType;
    caf::PdmField<double>                              m_userDefinedDFactor;

    caf::PdmField<caf::AppEnum<WidthEnum>>             m_fractureWidthType;
    caf::PdmField<double>                              m_fractureWidth;
    caf::PdmField<double>                              m_inertialCoefficient;

    caf::PdmField<caf::AppEnum<PermeabilityEnum>>      m_permeabilityType;
    caf::PdmField<double>                              m_relativePermeability;
    caf::PdmField<double>                              m_userDefinedEffectivePermeability;

    caf::PdmField<double>                              m_relativeGasDensity;
    caf::PdmField<double>                              m_gasViscosity;

    caf::PdmProxyValueField<double>                    m_dFactorDisplayField;
    caf::PdmProxyValueField<QString>                   m_dFactorSummaryText;

    caf::PdmField<double>                              m_heightScaleFactor;
    caf::PdmField<double>                              m_widthScaleFactor;
    caf::PdmField<double>                              m_dFactorScaleFactor;
    caf::PdmField<bool>                                m_scaleApplyButton;
};
