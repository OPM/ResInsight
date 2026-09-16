/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "cafPdmScriptResponse.h"

#include <QString>

#include <expected>

class RimCase;
class Rim3dView;
class RimFractureTemplate;

namespace caf
{
class PdmObjectHandle;
}

//==================================================================================================
/// Helpers used by the legacy command file interface (Ricf*) to forward to the object-oriented
/// command methods (Rimc*). The legacy interface identifies objects by integer ids, while the
/// object methods operate directly on objects. These helpers translate between the two.
//==================================================================================================
namespace RicfForwarding
{
/// Find a grid case by id. A negative id resolves to the first Eclipse result case in the project.
std::expected<RimCase*, QString> findCase( int caseId );

/// Find a view by id in the given case. A negative id is not accepted.
std::expected<Rim3dView*, QString> findView( RimCase* rimCase, int viewId );

/// Find a 3D view by id anywhere in the project. A negative id is not accepted.
std::expected<Rim3dView*, QString> findView( int viewId );

/// Find a fracture template by id.
std::expected<RimFractureTemplate*, QString> findFractureTemplate( int templateId );

/// Convert the result of a Rimc method execution to a script response, logging errors with the command name.
caf::PdmScriptResponse toScriptResponse( const std::expected<caf::PdmObjectHandle*, QString>& result, const QString& commandName );

/// Create an error response and log the message with the command name.
caf::PdmScriptResponse errorResponse( const QString& message, const QString& commandName );

} // namespace RicfForwarding
