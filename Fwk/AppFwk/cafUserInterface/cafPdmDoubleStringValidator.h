//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include <QDoubleValidator>
#include <QString>

namespace caf
{
//////////////////////////////////////////////////////////////////////////
/// Class that validates text strings containing double values but allows
/// a default text string that isn't necessarily a double.
/// Example use is the "1*" value in ECLIPSE output files or "N/A".
//////////////////////////////////////////////////////////////////////////
class PdmDoubleStringValidator : public QDoubleValidator
{
    Q_OBJECT
public:
    PdmDoubleStringValidator(const QString& defaultString)
        : QDoubleValidator(nullptr), m_defaultString(defaultString)
    {
    }

    State validate(QString& inputString, int& position) const override;
    void  fixup(QString& inputString) const override;

private:
    QString m_defaultString;
};
}

