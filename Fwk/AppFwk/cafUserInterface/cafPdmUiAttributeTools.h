#pragma once

#include <QString>

#include <optional>

namespace caf
{
class PdmUiItem;

namespace PdmUiAttributeTools
{
    // Resolves a string attribute that was set either as a plain string (QString, const char*) or as a
    // std::function<QString()> callback evaluated on each call. Returns nullopt if the key is not set
    // or holds another type.
    std::optional<QString> stringAttribute( const PdmUiItem* uiItem, const QString& key, const QString& uiConfigName = "" );
} // namespace PdmUiAttributeTools

} // end namespace caf
