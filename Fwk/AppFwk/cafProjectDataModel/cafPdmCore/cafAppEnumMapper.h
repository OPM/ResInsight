#pragma once

#include <QString>
#include <QStringList>

#include <map>
#include <optional>
#include <string>

namespace caf
{

//==================================================================================================
//
//==================================================================================================
class AppEnumMapper
{
private:
    class EnumData
    {
    public:
        EnumData( int enumVal, const QString& text, const QString& uiText, const QStringList& aliases );

        bool isMatching( const QString& text ) const;

        int         m_enumVal;
        QString     m_text;
        QString     m_uiText;
        QStringList m_aliases;
        bool        m_isDefault;
    };

public:
    static AppEnumMapper* instance();

    void addItem( const std::string& enumKey,
                  int                enumValue,
                  const QString&     text,
                  const QString&     uiText,
                  const QStringList& aliases = {} );

    void addDefaultItem( const std::string& enumKey,
                         int                enumValue,
                         const QString&     text,
                         const QString&     uiText,
                         const QStringList& aliases = {} );

    size_t  size( const std::string& enumKey ) const;
    size_t  index( const std::string& enumKey, int enumValue ) const;
    QString text( const std::string& enumKey, int enumValue ) const;
    QString uiText( const std::string& enumKey, int enumValue ) const;

    int defaultEnumValue( const std::string& enumKey ) const;
    int enumValue( const std::string& enumKey, const QString& text ) const;
    int enumValue( const std::string& enumKey, int enumIndex ) const;

private:
    std::map<std::string, std::vector<EnumData>> m_enumMap;
};

} // end namespace caf
