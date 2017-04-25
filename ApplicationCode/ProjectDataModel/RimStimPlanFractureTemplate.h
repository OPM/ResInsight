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
class RigStimPlanCell;

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

    caf::PdmField<QString>                  parameterForPolygon;
    caf::PdmField<int>                      activeTimeStepIndex;
    caf::PdmField<bool>                     showStimPlanMesh;
    
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    void                                    setFileName(const QString& fileName);
    const QString&                          fileName();
    QString                                 fileNameWithOutPath();

    void                                    fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices, caf::AppEnum< RimDefines::UnitSystem > fractureUnit) override;
    std::vector<cvf::Vec3f>                 fracturePolygon(caf::AppEnum< RimDefines::UnitSystem > fractureUnit);
    void sortPolygon(std::vector<cvf::Vec3f> &polygon);

    size_t                                  stimPlanGridNumberOfRows();
    size_t                                  stimPlanGridNumberOfColums();


    std::vector<double>                     getNegAndPosXcoords();
    std::vector<double>                     adjustedDepthCoordsAroundWellPathPosition();
    std::vector<double>                     getStimPlanTimeValues();
    std::vector<std::pair<QString, QString> > getStimPlanPropertyNamesUnits() const;
    void                                    computeMinMax(const QString& resultName, const QString& unitName, double* minValue, double* maxValue) const;
    
    void                                    getStimPlanDataAsPolygonsAndValues(std::vector<std::vector<cvf::Vec3d> > &cellsAsPolygons, std::vector<double> &parameterValue, const QString& resultName, const QString& unitName, size_t timeStepIndex);
    void                                    setupStimPlanCells();
    const std::vector<RigStimPlanCell>&     getStimPlanCells();
    std::vector<cvf::Vec3d>                 getStimPlanRowPolygon(size_t i);
    std::vector<cvf::Vec3d>                 getStimPlanColPolygon(size_t j);

    std::pair<size_t, size_t>               getStimPlanCellAtWellCenter();

    size_t                                  getGlobalIndexFromIJ(size_t i, size_t j); //TODO: should be const?
    const RigStimPlanCell&                  stimPlanCellFromIndex(size_t index) const;

    //TODO: Functions for finding perforated stimPlanCells
    //Radial flow: Single cell (at 0,0) 




    void                                    loadDataAndUpdate(); //TODO: Update m_stimPlanCells
    void                                    setDefaultsBasedOnXMLfile();
    std::vector<std::vector<double>>        getDataAtTimeIndex(const QString& resultName, const QString& unitName, size_t timeStepIndex) const;
    std::vector<std::vector<double>>        getMirroredDataAtTimeIndex(const QString& resultName, const QString& unitName, size_t timeStepIndex) const;
    
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;


protected:
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;


private:
    void                                    updateUiTreeName();

    void                                    readStimPlanXMLFile(QString * errorMessage);
    size_t                                  readStimplanGridAndTimesteps(QXmlStreamReader &xmlStream);
    static double                           getAttributeValueDouble(QXmlStreamReader &xmlStream, QString parameterName);
    static QString                          getAttributeValueString(QXmlStreamReader &xmlStream, QString parameterName);
    void                                    getGriddingValues(QXmlStreamReader &xmlStream, std::vector<double>& gridValues, size_t& startNegValues);
    std::vector<std::vector<double>>        getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream, size_t startingNegValuesXs);

    bool                                    numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep);
    bool                                    setPropertyForPolygonDefault();
    void                                    setDepthOfWellPathAtFracture();
    QString                                 getUnitForStimPlanParameter(QString parameterName);

    caf::PdmField<QString>                      m_stimPlanFileName;
    cvf::ref<RigStimPlanFractureDefinition>     m_stimPlanFractureDefinitionData;
    std::vector<RigStimPlanCell>                m_stimPlanCells;
    std::pair<size_t, size_t>                   wellCenterStimPlanCellIJ;
};
