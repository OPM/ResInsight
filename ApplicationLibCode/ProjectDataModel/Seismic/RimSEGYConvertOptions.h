/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023    Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>
#include <QStringList>

#include <utility>

class RimSEGYConvertOptions : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSEGYConvertOptions();
    ~RimSEGYConvertOptions() override;

    QString inputFilename() const;
    void    setInputFilename( QString filename );

    QString outputFilename() const;
    void    setOutputFilename( QString filename );

    QString headerFormatFilename() const;

    std::pair<bool, double> sampleStartOverride() const;

    QString     programDirectory() const;
    QString     convertCommand() const;
    QStringList convertCommandParameters() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<caf::FilePath>           m_inputFilename;
    caf::PdmField<caf::FilePath>           m_outputFilename;
    caf::PdmField<std::pair<bool, double>> m_sampleStartOverride;
    caf::PdmField<caf::FilePath>           m_headerFormatFilename;
    caf::PdmField<QString>                 m_sampleUnit;
};
