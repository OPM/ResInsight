#include "gtest/gtest.h"

#include "RiaVersionInfo.h"

#include "cafClassTypeName.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObjectHandle.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void writeTextToFile(const QString& filePath, const QString& text)
{
    QFile exportFile(filePath);
    if (exportFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&exportFile);

        stream << text;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString versionHeaderText()
{
    QString text;

    QDateTime dt = QDateTime::currentDateTime();

    text += QString("// ResInsight version string : %1\n").arg(STRPRODUCTVER);
    text += QString("// Report generated : %1\n").arg(QDateTime::currentDateTime().toString());
    text += "//\n";
    text += "//\n";
    text += "\n";

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(ListKeywords, ListAllObjectKeywords)
{
    auto instance = caf::PdmDefaultObjectFactory::instance();

    QString     textString;
    QTextStream stream(&textString);

    textString = versionHeaderText();

    std::vector<QString> classKeywords = instance->classKeywords();
    for (auto keyword : classKeywords)
    {
        stream << keyword << "\n";
    }

    QString filePath = "c:/temp/ri-objectKeywords.txt";
    //writeTextToFile(filePath, textString);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(ListKeywords, ListAllObjectKeywordsAndFieldKeywords)
{
    auto instance = caf::PdmDefaultObjectFactory::instance();

    QString     textString;
    QTextStream stream(&textString);

    bool includeClassName = true;

    textString = versionHeaderText();

    std::vector<QString> classKeywords = instance->classKeywords();
    for (auto keyword : classKeywords)
    {
        caf::PdmObjectHandle* myClass = instance->create(keyword);

        stream << keyword;

        if (includeClassName)
        {
            QString className = qStringTypeName(*myClass);

            stream << " - " << className;
        }

        stream << "\n";

        std::vector<caf::PdmFieldHandle*> fields;
        myClass->fields(fields);

        for (auto f : fields)
        {
            stream << "  " << f->keyword() << "\n";
        }

        stream << "\n";

        delete myClass;
    }

    QString filePath = "c:/temp/ri-fieldKeywords.txt";
    //writeTextToFile(filePath, textString);
}
