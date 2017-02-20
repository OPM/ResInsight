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

class RigStimPlanFractureDefinition;
class RimStimPlanLegendConfig;

//==================================================================================================
///  
///  
//==================================================================================================
class RimStimPlanFractureTemplate : public RimFractureTemplate 
{
     CAF_PDM_HEADER_INIT;

public:
    RimStimPlanFractureTemplate(void);
    virtual ~RimStimPlanFractureTemplate(void);
    
    caf::PdmField<double>                   wellPathDepthAtFracture;
    
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    void                                    setFileName(const QString& fileName);
    const QString&                          fileName();
    QString                                 fileNameWithOutPath();

    void                                    fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices);
    std::vector<cvf::Vec3f>                 fracturePolygon();

    std::vector<double>                     getNegAndPosXcoords();
    std::vector<double>                     adjustedDepthCoordsAroundWellPathPosition();
    std::vector<double>                     getStimPlanTimeValues();
    std::vector<std::pair<QString, QString> > getStimPlanPropertyNamesUnits() const;
    void                                    computeMinMax(const QString& resultName, const QString& unitName, double* minValue, double* maxValue) const;

    void                                    loadDataAndUpdate();

    std::vector<std::vector<double>>        getDataAtTimeIndex(const QString& resultName, const QString& unitName, size_t timeStepIndex) const;

protected:
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;


private:
    void                                    updateUiTreeName();

    void                                    readStimPlanXMLFile(QString * errorMessage);


    void                                    readStimplanGridAndTimesteps(QXmlStreamReader &xmlStream);


    static double                           getAttributeValueDouble(QXmlStreamReader &xmlStream, QString parameterName);
    static QString                          getAttributeValueString(QXmlStreamReader &xmlStream, QString parameterName);
    static std::vector<double>              getGriddingValues(QXmlStreamReader &xmlStream);
    std::vector<std::vector<double>>        getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream);

    caf::PdmField<QString>                      m_stimPlanFileName;
    cvf::ref<RigStimPlanFractureDefinition>     m_stimPlanFractureDefinitionData;

    caf::PdmChildArrayField<RimStimPlanLegendConfig*>   m_legendConfigurations;

    bool numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep);
    void setDepthOfWellPathAtFracture();
};
