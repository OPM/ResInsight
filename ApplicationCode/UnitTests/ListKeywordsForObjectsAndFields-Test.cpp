#include "gtest/gtest.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObjectHandle.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void writeTextToFile(const QString& filePath, const QString& text)
{
    QFile exportFile(filePath);
    if (exportFile.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&exportFile);

        stream << text;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(ListKeywords, ListAllObjectKeywords)
{
    auto instance = caf::PdmDefaultObjectFactory::instance();

    QString     textString;
    QTextStream stream(&textString);

    std::vector<QString> classKeywords = instance->classKeywords();
    for (auto keyword : classKeywords)
    {
        stream << keyword << "\n";
    }

    QString filePath = "c:/temp/ri-objectKeywords.txt";
    // writeTextToFile(filePath, textString);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(ListKeywords, ListAllObjectKeywordsAndFieldKeywords)
{
    auto instance = caf::PdmDefaultObjectFactory::instance();

    QString     textString;
    QTextStream stream(&textString);

    std::vector<QString> classKeywords = instance->classKeywords();
    for (auto keyword : classKeywords)
    {
        stream << keyword << "\n";

        caf::PdmObjectHandle* myClass = instance->create(keyword);

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
    // writeTextToFile(filePath, textString);
}
