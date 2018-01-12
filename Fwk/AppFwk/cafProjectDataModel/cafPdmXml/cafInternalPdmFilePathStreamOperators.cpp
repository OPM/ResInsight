#include "cafInternalPdmFilePathStreamOperators.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator<<(QTextStream& str, const std::vector<caf::FilePath>& filePathObjects)
{
    QStringList trimmedEntries;

    for (const auto& filePath : filePathObjects)
    {
        QString text = filePath.path().trimmed();

        if (text.isEmpty()) continue;

        trimmedEntries.push_back(text);
    }

    for (const auto& s : trimmedEntries)
    {
        str << s;

        if (s != trimmedEntries.back())
        {
            str << ";";
        }
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

        if (!singleChar.isNull())
        {
            stringSeparatedBySemicolon += singleChar;
        }
    }

    QStringList splitBySemicolon = stringSeparatedBySemicolon.split(";");
    for (const auto& s : splitBySemicolon)
    {
        QString trimmed = s.trimmed();
        if (!trimmed.isEmpty())
        {
            filePathObjects.push_back(trimmed);
        }
    }

    return str;
}

