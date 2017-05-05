/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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



//==================================================================================================
///  
///  
//==================================================================================================
class RimFishbonesMultipleSubs : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum LocationType
    {
        FB_SUB_COUNT_END,
        FB_SUB_SPACING_END,
        FB_SUB_USER_DEFINED
    };

    enum LateralsOrientationType
    {
        FB_LATERAL_ORIENTATION_FIXED,
        FB_LATERAL_ORIENTATION_RANDOM
    };

public:
    RimFishbonesMultipleSubs();
    virtual ~RimFishbonesMultipleSubs();

    std::vector<double> locationOfSubs() const;

    double              rotationAngle(size_t index) const;
    double              exitAngle() const;
    double              buildAngle() const;

    double              tubingRadius() const;
    double              lateralCountPerSub() const;
    std::vector<double> lateralLengths() const;

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void initAfterRead() override;

private:
    void                        recomputeIntallationRotationAngles();

    static std::vector<double>  locationsFromStartSpacingAndCount(double start, double spacing, size_t count);
    static int                  randomValue(int min, int max);

private:
    caf::PdmField<size_t>               m_lateralCountPerSub;
    caf::PdmField<QString>              m_lateralLength;

    caf::PdmField<double>               m_lateralExitAngle;
    caf::PdmField<double>               m_lateralBuildAngle;

    caf::PdmField<double>               m_skinFactor;

    caf::PdmField<double>               m_lateralHoleRadius;
    caf::PdmField<double>               m_lateralTubingRadius;

    caf::PdmField<double>               m_lateralOpenHoleRoghnessFactor;
    caf::PdmField<double>               m_lateralTubingRoghnessFactor;

    caf::PdmField<double>               m_lateralLengthFraction;
    caf::PdmField<double>               m_lateralInstallFraction;

    caf::PdmField<size_t>               m_icdCount;
    caf::PdmField<double>               m_icdOrificeRadius;

    caf::PdmField<caf::AppEnum<LocationType> >    m_subsLocationMode;
    caf::PdmField<double>               m_rangeStart;
    caf::PdmField<double>               m_rangeEnd;
    caf::PdmField<double>               m_rangeSubSpacing;
    caf::PdmField<size_t>               m_rangeSubCount;

    caf::PdmField<caf::AppEnum<LateralsOrientationType> >    m_subsOrientationMode;


    caf::PdmField<std::vector<double>>  m_locationOfSubs; // Given in measured depth

    caf::PdmField<std::vector<double>>  m_installationRotationAngles;
    caf::PdmField<double>               m_fixedInstallationRotationAngle;

};
