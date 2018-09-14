/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicCreateMultipleFracturesOptionItemUi.h"

#include <QPointer>

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

class RimEclipseCase;
class RimWellPath;
class RigMainGrid;

namespace caf
{
    class PdmUiPropertyViewDialog;
}

//==================================================================================================
///
//==================================================================================================
class LocationForNewFracture
{
public:
    LocationForNewFracture(RimFractureTemplate* fractureTemplate, RimWellPath* wellPath, double measuredDepth)
        : fractureTemplate(fractureTemplate)
        , wellPath(wellPath)
        , measuredDepth(measuredDepth)
    {
    }

    bool operator<(const LocationForNewFracture& loc)
    {
        return measuredDepth < loc.measuredDepth;
    }

    RimFractureTemplate* fractureTemplate;
    RimWellPath*         wellPath;
    double               measuredDepth;
};

//==================================================================================================
///
//==================================================================================================
class RiuCreateMultipleFractionsUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    static const QString ADD_FRACTURES_BUTTON_TEXT;
    static const QString REPLACE_FRACTURES_BUTTON_TEXT;

    RiuCreateMultipleFractionsUi();

    void setParentDialog(QPointer<caf::PdmUiPropertyViewDialog> dialog);
    void setValues(RimEclipseCase* eclipseCase, double minimumDistanceFromWellTip, int maxFracturesPerWell);
    void resetValues();

    std::vector<RicCreateMultipleFracturesOptionItemUi*> options() const;

    void insertOptionItem(RicCreateMultipleFracturesOptionItemUi* insertAfterThisObject,
                          RicCreateMultipleFracturesOptionItemUi* objectToInsert);

    void deleteOptionItem(RicCreateMultipleFracturesOptionItemUi* optionsItem);

    void clearOptions();

    void addWellPath(RimWellPath* wellPath);

    void clearWellPaths();

    std::vector<LocationForNewFracture> locationsForNewFractures() const;

    void updateButtonsEnableState();

private:
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;
    virtual void
                 defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget) override;
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field,
                                       QString                    uiConfigName,
                                       caf::PdmUiEditorAttribute* attribute) override;

    QString summaryText() const;

private:
    caf::PdmPtrField<RimEclipseCase*>                                m_sourceCase;
    caf::PdmField<double>                                            m_minDistanceFromWellTd;
    caf::PdmField<int>                                               m_maxFracturesPerWell;
    caf::PdmChildArrayField<RicCreateMultipleFracturesOptionItemUi*> m_options;

    caf::PdmProxyValueField<QString> m_fractureCreationSummary;

    std::vector<RimWellPath*>       m_wellPaths;

    QPointer<caf::PdmUiPropertyViewDialog> m_dialog;
};
