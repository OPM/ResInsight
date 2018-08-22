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

#include "cafCmdFeature.h"

#include <QPointer>

class RiuCreateMultipleFractionsUi;

namespace caf {
    class PdmUiPropertyViewDialog;
}

//==================================================================================================
///
//==================================================================================================
class RicCreateMultipleFracturesFeature : public caf::CmdFeature
{
    Q_OBJECT
    CAF_CMD_HEADER_INIT;

public:
    RicCreateMultipleFracturesFeature() {}

    void appendFractures();
    void replaceFractures();

private slots:
    void slotDeleteAndAppendFractures();
    void slotAppendFractures();
    void slotClose();
    void slotCancel();

private:
    virtual void onActionTriggered(bool isChecked) override;
    virtual void setupActionLook(QAction* actionToSetup) override;
    virtual bool isCommandEnabled() override;

    RiuCreateMultipleFractionsUi* multipleFractionsUi() const;

private:
    QPointer<caf::PdmUiPropertyViewDialog> m_dialog;
    QString m_copyOfObject;
};
