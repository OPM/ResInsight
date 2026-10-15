//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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

#include "cafPdmUiLineEditor.h"

#include <memory>

namespace caf
{
class PickEventHandler;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiPickableLineEditor : public PdmUiLineEditor
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiPickableLineEditor() {}
    ~PdmUiPickableLineEditor() override;

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        // bool. Register the pick event handler when true
        static inline const QString ENABLE_PICKING = QStringLiteral( "enablePicking" );
        // std::shared_ptr<PickEventHandler>. Handler receiving pick events for the field
        static inline const QString PICK_EVENT_HANDLER = QStringLiteral( "pickEventHandler" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::ENABLE_PICKING, Keys::PICK_EVENT_HANDLER };

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

private:
    bool                              m_enablePicking = false;
    std::shared_ptr<PickEventHandler> m_pickEventHandler;
};

} // end namespace caf

Q_DECLARE_METATYPE( std::shared_ptr<caf::PickEventHandler> )
