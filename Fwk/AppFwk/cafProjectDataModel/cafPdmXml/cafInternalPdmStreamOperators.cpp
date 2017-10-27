#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// Specialized read operation for Bool`s
//--------------------------------------------------------------------------------------------------
QTextStream& operator >> (QTextStream& str, bool& value)
{
    QString text;
    str >> text;
    if (text == "True" || text == "true" || text == "1" || text == "Yes" || text == "yes") value = true;
    else value = false;

    return str;
}

QTextStream& operator << (QTextStream& str, const bool& value)
{
    if (value) str << "True ";
    else str << "False ";

    return str;
}


//--------------------------------------------------------------------------------------------------
/// Specialized read operation for QDateTimes`s
//--------------------------------------------------------------------------------------------------
#include <QDateTime>
QTextStream&  operator >> (QTextStream& str, QDateTime& value)
{
    QString text;
    str >> text;
    value = QDateTime::fromString(text, "yyyy_MM_dd-HH:mm:ss");
    value.setTimeSpec(Qt::UTC);
    return str;
}

QTextStream&  operator << (QTextStream& str, const QDateTime& value)
{
    QString text = value.toString("yyyy_MM_dd-HH:mm:ss");
    str << text;
    return str;
}
