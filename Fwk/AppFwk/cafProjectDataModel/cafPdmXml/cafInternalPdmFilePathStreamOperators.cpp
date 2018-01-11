#include "cafInternalPdmFilePathStreamOperators.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator<<(QTextStream& str, const std::vector<caf::FilePath>& filePathObjects)
{
    for (const auto& filePath : filePathObjects)
    {
        str << filePath << ";";
    }

    return str;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator>>(QTextStream& str, std::vector<caf::FilePath>& filePathObjects)
{
    QString stringSeparatedBySemicolon;

    while (str.status() == QTextStream::Ok)
    {
        // Read QChar to avoid white space trimming when reading QString
        QChar singleChar;
        str >> singleChar;

        stringSeparatedBySemicolon += singleChar;
    }

    QStringList splitBySemicolon = stringSeparatedBySemicolon.split(";");
    for (const auto& s : splitBySemicolon )
    {
        filePathObjects.push_back(s);
    }

    return str;
}

