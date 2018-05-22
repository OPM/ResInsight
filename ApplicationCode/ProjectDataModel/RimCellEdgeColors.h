/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cafAppEnum.h"
#include "cafFixedArray.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RigCaseCellResultsData;
class RimEclipseCase;
class RimEclipseCellColors;
class RimEclipseView;
class RimRegularLegendConfig;

class RimCellEdgeMetaData
{
public:
    size_t  m_resultIndex;
    QString m_resultVariable;
    bool    m_isStatic;
};

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellEdgeColors :  public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCellEdgeColors();
    virtual ~RimCellEdgeColors();

    enum EdgeFaceType
    {
        X, NEG_X, 
        Y, NEG_Y, 
        Z, NEG_Z
    };

    enum PropertyType
    {
        MULTI_AXIS_STATIC_PROPERTY,
        ANY_SINGLE_PROPERTY
    };

    typedef  caf::AppEnum<RimCellEdgeColors::EdgeFaceType> EdgeFaceEnum;

    void                                  setReservoirView(RimEclipseView* ownerReservoirView);
    void                                  setEclipseCase(RimEclipseCase* eclipseCase);

    void                                  setResultVariable(const QString& variableName);
    QString                               resultVariable() const;
    QString                               resultVariableUiName() const;
    QString                               resultVariableUiShortName() const;

    caf::PdmField<bool>                   enableCellEdgeColors;

    double                                ignoredScalarValue() { return m_ignoredResultScalar; }
    void                                  gridScalarIndices(size_t resultIndices[6]);
    void                                  cellEdgeMetaData(std::vector<RimCellEdgeMetaData>* metaData);

    void                                  loadResult();
    bool                                  hasResult() const; 
    bool                                  hasCategoryResult() const;
    bool                                  isUsingSingleVariable() const;

    RimEclipseCellColors*                 singleVarEdgeResultColors();
    RimRegularLegendConfig*                      legendConfig();
    PropertyType                          propertyType() const;

    void                                  minMaxCellEdgeValues(double& min, double& max);
    void                                  posNegClosestToZero(double& pos, double& neg);

    void                                  updateUiFieldsFromActiveResult();

protected:

    virtual void                          initAfterRead();
    virtual void                          fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly );
    virtual void                          defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                          defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");

    QStringList                           findResultVariableNames();

private:
    void                                  resetResultIndices();
    void                                  updateIgnoredScalarValue();

    void                                  gridScalarResultNames(std::vector<QString>* resultNames);

    virtual caf::PdmFieldHandle*          objectToggleField();

    caf::PdmField<QString>                m_resultVariable;
    caf::PdmField<bool>                   useXVariable;
    caf::PdmField<bool>                   useYVariable;
    caf::PdmField<bool>                   useZVariable;

    caf::FixedArray<std::pair<QString, size_t>, 6 > m_resultNameToIndexPairs;
    caf::PdmPointer<RimEclipseView>                 m_reservoirView;
    double                                          m_ignoredResultScalar;


    caf::PdmField<caf::AppEnum< PropertyType > >    m_propertyType;
    caf::PdmChildField<RimRegularLegendConfig*>            m_legendConfig;
    caf::PdmChildField<RimEclipseCellColors*>       m_singleVarEdgeResultColors;

};

