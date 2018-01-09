/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include "RigFemResultPosEnum.h"
#include "RigFemResultAddress.h"

class RimGeoMechView;
class RimGeoMechPropertyFilter;
class RifGeoMechReaderInterface;
class RigGeoMechCaseData;
class RimGeoMechCase;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechResultDefinition : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;
public:
    RimGeoMechResultDefinition(void);
    virtual ~RimGeoMechResultDefinition(void);

    void                                              setGeoMechCase(RimGeoMechCase* geomCase);

    RigGeoMechCaseData*                               ownerCaseData();
    bool                                              hasResult(); 
    void                                              loadResult();

    RigFemResultAddress                               resultAddress();

    RigFemResultPosEnum                               resultPositionType()  { return m_resultPositionType();}
    QString                                           resultFieldName()     { return m_resultFieldName();}
    QString                                           resultComponentName() { return m_resultComponentName();}
    void                                              setResultAddress(const RigFemResultAddress& resultAddress);

    QString                                           resultFieldUiName();
    QString                                           resultComponentUiName();

    bool                                              hasCategoryResult()  { return m_resultPositionType() == RIG_FORMATION_NAMES; }

protected:
    virtual void                                      updateLegendCategorySettings() {};
    virtual void                                      defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:

    // Overridden PDM methods

    virtual QList<caf::PdmOptionItemInfo>             calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, 
                                                                            bool * useOptionsOnly) override;
    virtual void                                      fieldChangedByUi(const caf::PdmFieldHandle* changedField, 
                                                                       const QVariant& oldValue, 
                                                                       const QVariant& newValue) override;
    virtual void                                      initAfterRead() override;

    // Metadata and option build tools

    std::map<std::string, std::vector<std::string> >  getResultMetaDataForUIFieldSetting();
    static void                                       getUiAndResultVariableStringList(QStringList* uiNames, 
                                                                                       QStringList* variableNames, 
                                                                                       const std::map<std::string, std::vector<std::string> >& fieldCompNames, 
                                                                                       bool isTimeLapseResultList, 
                                                                                       int baseFrameIdx);
    static QString                                    composeFieldCompString(const QString& resultFieldName, 
                                                                             const QString& resultComponentName);

    static QString                                    convertToUiResultFieldName(QString resultFieldName,
                                                                                 bool isTimeLapseResultList,
                                                                                 int baseFrameIdx);
    static QString                                    convertToUIComponentName(QString resultComponentName,
                                                                               bool isTimeLapseResultList,
                                                                               int baseFrameIdx);

    // Data Fields

    caf::PdmField<caf::AppEnum<RigFemResultPosEnum> > m_resultPositionType;
    caf::PdmField<QString>                            m_resultFieldName;
    caf::PdmField<QString>                            m_resultComponentName;
    caf::PdmField<bool>                               m_isTimeLapseResult;
    caf::PdmField<int>                                m_timeLapseBaseTimestep;

    // UI Fields only

    friend class RimGeoMechPropertyFilter;  // Property filter needs the ui fields
    friend class RimWellLogExtractionCurve; // Curve needs the ui fields
    friend class RimGeoMechCellColors;      // Needs the ui fields
 
    caf::PdmField<caf::AppEnum<RigFemResultPosEnum> > m_resultPositionTypeUiField;
    caf::PdmField<QString>                            m_resultVariableUiField;
    caf::PdmField<bool>                               m_isTimeLapseResultUiField;
    caf::PdmField<int>                                m_timeLapseBaseTimestepUiField;


    caf::PdmPointer<RimGeoMechCase>                   m_geomCase;
};
