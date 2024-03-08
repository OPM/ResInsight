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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AppEnumMapper::addItem( const std::string& enumKey,
                             int                enumValue,
                             const QString&     text,
                             const QString&     uiText,
                             const QStringList& aliases /*= {} */ )
{
    // Make sure the text is trimmed, as this text is streamed to XML and will be trimmed when read back
    // from XML text https://github.com/OPM/ResInsight/issues/7829
    m_enumMap[enumKey].emplace_back( EnumData( enumValue, text.trimmed(), uiText, aliases ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AppEnumMapper::setDefault( const std::string& enumKey, int enumValue )
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        for ( auto& enumData : it->second )
        {
            if ( enumData.m_enumVal == enumValue )
            {
                enumData.m_isDefault = true;
            }
            else
            {
                enumData.m_isDefault = false;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t AppEnumMapper::size( const std::string& enumKey ) const
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        return it->second.size();
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int AppEnumMapper::defaultEnumValue( const std::string& enumKey ) const
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        for ( const auto& enumData : it->second )
        {
            if ( enumData.m_isDefault )
            {
                return enumData.m_enumVal;
            }
        }

        if ( !it->second.empty() )
        {
            return it->second.front().m_enumVal;
        }
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t AppEnumMapper::index( const std::string& enumKey, int enumValue ) const
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        for ( size_t i = 0; i < it->second.size(); ++i )
        {
            if ( it->second[i].m_enumVal == enumValue )
            {
                return i;
            }
        }
    }
    return std::numeric_limits<size_t>::max();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString AppEnumMapper::text( const std::string& enumKey, int enumValue ) const
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        for ( const auto& enumData : it->second )
        {
            if ( enumData.m_enumVal == enumValue )
            {
                return enumData.m_text;
            }
        }
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString AppEnumMapper::uiText( const std::string& enumKey, int enumValue ) const
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        for ( const auto& enumData : it->second )
        {
            if ( enumData.m_enumVal == enumValue )
            {
                return enumData.m_uiText;
            }
        }
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int AppEnumMapper::fromText( const std::string& enumKey, const QString& text ) const
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        for ( const auto& enumData : it->second )
        {
            if ( enumData.isMatching( text ) )
            {
                return enumData.m_enumVal;
            }
        }
    }
    return defaultEnumValue( enumKey );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int AppEnumMapper::fromIndex( const std::string& enumKey, size_t enumIndex ) const
{
    auto it = m_enumMap.find( enumKey );
    if ( it != m_enumMap.end() )
    {
        if ( enumIndex < it->second.size() )
        {
            return it->second[enumIndex].m_enumVal;
        }
    }
    return defaultEnumValue( enumKey );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
AppEnumMapper::EnumData::EnumData( int enumVal, const QString& text, const QString& uiText, const QStringList& aliases )
    : m_enumVal( enumVal )
    , m_text( text )
    , m_uiText( uiText )
    , m_aliases( aliases )
    , m_isDefault( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AppEnumMapper::EnumData::isMatching( const QString& text ) const
{
    return ( text == m_text || m_aliases.contains( text ) );
}

} //namespace caf
