/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include <QDialog>

namespace caf
{
    class PdmObject;
    class PdmUiPropertyView;
}


//==================================================================================================
//
// 
//
//==================================================================================================
class RiuPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    RiuPreferencesDialog(QWidget* parent, caf::PdmObject* object, const QString& windowTitle);

private:
    void setupUi();

private:
    QString                     m_windowTitle;
    caf::PdmObject*             m_pdmObject;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;
};
