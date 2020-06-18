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
