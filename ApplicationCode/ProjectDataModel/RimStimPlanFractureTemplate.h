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
    
    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    void                                setFileName(const QString& fileName);
    const QString&                      fileName();
    QString                             fileNameWoPath();
    void                                readStimPlanXMLFile(QString * errorMessage);

    double                              getAttributeValueDouble(QXmlStreamReader &xmlStream, QString parameterName);
    QString                             getAttributeValueString(QXmlStreamReader &xmlStream, QString parameterName);
    std::vector<double>                 getGriddingValues(QXmlStreamReader &xmlStream);
    std::vector<std::vector<double>>    getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream);

    void                                fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* polygonIndices);
    std::vector<cvf::Vec3f>             fracturePolygon();

protected:
    virtual void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

private:
    void                                updateUiTreeName();

    caf::PdmField<QString>                      m_StimPlanFileName;
    cvf::ref<RigStimPlanFractureDefinition>     m_StimPlanFractureDefinitionData;


};
