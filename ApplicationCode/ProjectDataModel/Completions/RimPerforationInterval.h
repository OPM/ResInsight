/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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
#include "Rim3dPropertiesInterface.h"
#include "RimWellPathComponentInterface.h"

#include "RiaEclipseUnitTools.h"

#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <QDate>

class RimWellPathValve;

//==================================================================================================
///  
//==================================================================================================
class RimPerforationInterval : public RimCheckableNamedObject,
                               public Rim3dPropertiesInterface,
                               public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:

    RimPerforationInterval();
    ~RimPerforationInterval() override;

    void                                setStartAndEndMD(double startMD, double endMD);

    void                                enableCustomStartDate(bool enable);
    void                                setCustomStartDate(const QDate& date);

    void                                enableCustomEndDate(bool enable);
    void                                setCustomEndDate(const QDate& date);

    void                                setDiameter(double diameter);
    void                                setSkinFactor(double skinFactor);
    double                              diameter(RiaEclipseUnitTools::UnitSystem unitSystem) const;
    double                              skinFactor() const;

    bool                                isActiveOnDate(const QDateTime& date) const;

    cvf::BoundingBox                    boundingBoxInDomainCoords() const override;

    void                                setUnitSystemSpecificDefaults();

    void                                addValve(RimWellPathValve* valve);
    std::vector<RimWellPathValve*>      valves() const;

    // RimWellPathCompletionInterface overrides
    bool                                isEnabled() const override;
    RiaDefines::WellPathComponentType   componentType() const override;
    QString                             componentLabel() const override;
    QString                             componentTypeLabel() const override;
    cvf::Color3f                        defaultComponentColor() const override;
    double                              startMD() const override;
    double                              endMD() const override;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override;
    void initAfterRead() override;

private:
    caf::PdmField< double >             m_startMD;
    caf::PdmField< double >             m_endMD;
    caf::PdmField< double >             m_diameter;
    caf::PdmField< double >             m_skinFactor;

    caf::PdmField< bool >               m_useCustomStartDate;
    caf::PdmField< QDateTime >          m_startDate;

    caf::PdmField< bool >               m_useCustomEndDate;
    caf::PdmField< QDateTime >          m_endDate;

    caf::PdmChildArrayField<RimWellPathValve*> m_valves;

    caf::PdmField< bool >               m_startOfHistory_OBSOLETE;
};
