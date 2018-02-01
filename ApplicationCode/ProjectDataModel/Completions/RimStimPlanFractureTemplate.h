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

#include "RimFractureTemplate.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>
#include "RigFractureCell.h"
#include "RigFractureGrid.h"

class RigStimPlanFractureDefinition;
class RimStimPlanLegendConfig;
class RigFractureCell;
class RigFractureGrid;

//==================================================================================================
///  
///  
//==================================================================================================
class RimStimPlanFractureTemplate : public RimFractureTemplate 
{
     CAF_PDM_HEADER_INIT;

public:
    RimStimPlanFractureTemplate();
    virtual ~RimStimPlanFractureTemplate();
    
    int                                     activeTimeStepIndex();

    void                                    loadDataAndUpdate(); 
    void                                    setDefaultsBasedOnXMLfile();

    void                                    setFileName(const QString& fileName);
    const QString&                          fileName();
    QString                                 fileNameWithOutPath();

    void                                    updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);


    // Fracture geometry
     
    const RigFractureGrid*                  fractureGrid() const override;
    void                                    updateFractureGrid();
    void                                    fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords,
                                                                     std::vector<cvf::uint>* triangleIndices, 
                                                                     RiaEclipseUnitTools::UnitSystem  neededUnit) override;
    std::vector<cvf::Vec3f>                 fractureBorderPolygon(RiaEclipseUnitTools::UnitSystem neededUnit) override;

    // Result Access

    std::vector<double>                     timeSteps();
    std::vector<std::pair<QString, QString> > uiResultNamesWithUnit() const;
    std::vector<std::vector<double>>        resultValues(const QString& uiResultName, const QString& unitName, size_t timeStepIndex) const;
    std::vector<double>                     fractureGridResults(const QString& resultName, const QString& unitName, size_t timeStepIndex) const;
    bool                                    hasConductivity() const;

    virtual void appendDataToResultStatistics(const QString& uiResultName, const QString& unit,
                                               MinMaxAccumulator& minMaxAccumulator,
                                               PosNegAccumulator& posNegAccumulator) const override;

    QString                                 mapUiResultNameToFileResultName(const QString& uiResultName) const;
    void                                    setDefaultConductivityResultIfEmpty();

protected:
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;

private:
    void                                    updateUiTreeName();

    bool                                    setBorderPolygonResultNameToDefault();
    void                                    setDepthOfWellPathAtFracture();
    void                                    setPerforationLength();
    QString                                 getUnitForStimPlanParameter(QString parameterName);

private:
    caf::PdmField<int>                      m_activeTimeStepIndex;
    caf::PdmField<QString>                  m_conductivityResultNameOnFile;

    caf::PdmField<double>                   m_wellPathDepthAtFracture;
    caf::PdmField<QString>                  m_borderPolygonResultName;

    caf::PdmField<QString>                  m_stimPlanFileName;
    cvf::ref<RigStimPlanFractureDefinition> m_stimPlanFractureDefinitionData;
    caf::PdmField<double>                   m_conductivityScalingFactor;
    cvf::ref<RigFractureGrid>               m_fractureGrid;
    bool                                    m_readError;
};
