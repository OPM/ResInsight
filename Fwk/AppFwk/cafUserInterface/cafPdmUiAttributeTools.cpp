#include "cafPdmUiAttributeTools.h"

#include "cafPdmUiItem.h"

#include <functional>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<QString>
    PdmUiAttributeTools::stringAttribute( const PdmUiItem* uiItem, const QString& key, const QString& uiConfigName )
{
    if ( auto callback = uiItem->attribute<std::function<QString()>>( key, uiConfigName ) )
    {
        if ( callback.value() ) return callback.value()();
        return std::nullopt;
    }

    return uiItem->attribute<QString>( key, uiConfigName );
}

} // end namespace caf
