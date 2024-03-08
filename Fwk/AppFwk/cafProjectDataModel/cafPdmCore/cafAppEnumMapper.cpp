#include "cafAppEnumMapper.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
AppEnumMapper* AppEnumMapper::instance()
{
    static AppEnumMapper* singleton = new AppEnumMapper;
    return singleton;
}

} //namespace caf
