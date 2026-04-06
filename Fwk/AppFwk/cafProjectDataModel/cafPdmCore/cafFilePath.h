#pragma once

#include <QMetaType>
#include <QString>

class QTextStream;

namespace caf
{
//==================================================================================================
//
//==================================================================================================
class FilePath
{
public:
    FilePath();
    FilePath( const QString& filePath );

    QString path() const;
    void    setPath( const QString& valueText );

    bool operator==( const FilePath& other ) const;

private:
    QString m_filePath;
};

} // end namespace caf

//==================================================================================================
// Overload of QTextStream
//==================================================================================================
QTextStream& operator>>( QTextStream& str, caf::FilePath& filePath );
QTextStream& operator<<( QTextStream& str, const caf::FilePath& filePath );

Q_DECLARE_METATYPE( caf::FilePath );

#include "cafPdmFieldTraits.h"

namespace caf
{

inline QVariant pdmToVariant( const FilePath& value )
{
    return QVariant( value.path() );
}

inline void pdmFromVariant( const QVariant& v, FilePath& out )
{
    out.setPath( v.toString() );
}

template <>
struct PdmVariantEqualImpl<FilePath>
{
    static bool equal( const QVariant& a, const QVariant& b ) { return a.toString() == b.toString(); }
};

} // namespace caf
