//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfProgramOptions.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class ProgramOptions::OptionSpec : public Object
{
public:
    OptionSpec(const String& name, const String& valueSyntax, const String& descr, ValueReq valReq, OptionFlags optflags)
        : m_name(name), m_valueSyntax(valueSyntax), m_descr(descr), m_valueReq(valReq), m_optionFlags(optflags)
    {}

public:
    const String      m_name;
    const String      m_valueSyntax;
    const String      m_descr;
    const ValueReq    m_valueReq;
    const OptionFlags m_optionFlags;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class ProgramOptions::ParsedOption : public Object
{
public:
    ParsedOption(const OptionSpec* optionSpec)
        : m_spec(optionSpec)
    {}

    /// Check if the option has enough actual values compared to the requirements in spec
    bool hasEnoughValues() const
    {
        const size_t numParsedValues = m_values.size();

        CVF_ASSERT(m_spec.notNull());
        if (m_spec->m_valueReq == NO_VALUE)
        {
            CVF_ASSERT(numParsedValues == 0);
            return true;
        }
        else if (m_spec->m_valueReq == SINGLE_VALUE)
        {
            CVF_ASSERT(numParsedValues <= 1);
            return (numParsedValues == 1) ? true : false;
        }
        else if (m_spec->m_valueReq == OPTIONAL_MULTI_VALUE)
        {
            return true;
        }
        else
        {
            return (numParsedValues >= 1) ? true : false;
        }
    }

    // Check if we're full
    bool canAcceptMoreValues() const
    {
        const size_t numParsedValues = m_values.size();

        CVF_ASSERT(m_spec.notNull());
        if (m_spec->m_valueReq == NO_VALUE)
        {
            CVF_ASSERT(numParsedValues == 0);
            return false;
        }
        else if (m_spec->m_valueReq == SINGLE_VALUE)
        {
            CVF_ASSERT(numParsedValues <= 1);
            return (numParsedValues == 0) ? true : false;
        }
        else
        {
            return true;
        }
    }

public:
    cref<OptionSpec>    m_spec;
    std::vector<String> m_values;
};



//==================================================================================================
///
/// \class cvf::Option
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructs an invalid/empty option
//--------------------------------------------------------------------------------------------------
Option::Option()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Option::Option(const String& name, const std::vector<String>& values)
:   m_name(name),
    m_values(values)
{
    CVF_ASSERT(!m_name.isEmpty());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Option::name() const
{
    return m_name;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t Option::valueCount() const
{
    return m_values.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Option::value(size_t valueIndex) const
{
    CVF_ASSERT(valueIndex < m_values.size());
    return m_values[valueIndex];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Option::safeValue(size_t valueIndex) const
{
    if (valueIndex < m_values.size())
    {
        return m_values[valueIndex];
    }
    else
    {
        return String();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<String> Option::values() const
{
    return m_values;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Option::combinedValues() const
{
    String combined;

    for (size_t i = 0; i < m_values.size(); i++)
    {
        if (i > 0) combined += " ";
        combined += m_values[i];
    }

    return combined;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Option::isValid() const
{
    return m_name.isEmpty() ? false : true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Option::operator bool_type() const
{
    return isValid() ? &Option::this_type_does_not_support_comparisons : 0; 
}



//==================================================================================================
///
/// \class cvf::ProgramOptions
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ProgramOptions::ProgramOptions()
:   m_optionPrefix(DOUBLE_DASH)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ProgramOptions::~ProgramOptions()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgramOptions::setOptionPrefix(OptionPrefix prefix)
{
    m_optionPrefix = prefix;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ProgramOptions::registerOption(const String& optionName, ValueReq valueReq, OptionFlags optionFlags)
{
    return registerOption(optionName, "", "", valueReq, optionFlags);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ProgramOptions::registerOption(const String& optionName, const String& valueSyntax, const String& description, ValueReq valueReq, OptionFlags optionFlags)
{
    if (optionName.isEmpty())
    {
        return false;
    }

    if (findOptionSpec(optionName))
    {
        return false;
    }

    ref<OptionSpec> newOpt = new OptionSpec(optionName, valueSyntax, description, valueReq, optionFlags);
    m_optionSpecs.push_back(newOpt.p());

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ProgramOptions::parse(const std::vector<String>& commandLineArguments)
{
    const String prefixStr = prefixString();
    const size_t prefixLen = prefixStr.size();

    m_parsedOptions.clear();
    m_positionalParams.clear();
    m_unknownOptions.clear();
    m_optionsWithMissingValues.clear();

    ref<ParsedOption> currParseOption;

    const size_t numCmdLineArgs = commandLineArguments.size();
    size_t ia = 1;
    while (ia < numCmdLineArgs)
    {
        String arg = commandLineArguments[ia];
        arg = arg.trimmed();
        if (arg.isEmpty())
        {
            ia++;
            continue;
        }

        if (arg.startsWith(prefixStr))
        {
            // Finish the option currently being parsed if any
            if (currParseOption.notNull())
            {
                if (currParseOption->hasEnoughValues())
                {
                    addNewParsedOption(currParseOption.p());
                }
                else
                {
                    m_optionsWithMissingValues.push_back(currParseOption->m_spec->m_name);
                }

                currParseOption = NULL;
            }
            
            if (arg.size() > prefixLen)
            {
                const String optName = arg.subString(prefixLen);
                const OptionSpec* optSpec = findOptionSpec(optName);
                if (optSpec)
                {
                    currParseOption = new ParsedOption(optSpec);
                }
                else
                {
                    m_unknownOptions.push_back(optName);
                }
            }
        }
        else
        {
            // Determine if this arg belongs as a value to the option currently being parsed or if it is a positional parameter
            if (currParseOption.notNull())
            {
                // Simple flags should never get here
                CVF_ASSERT(currParseOption->m_spec->m_valueReq != NO_VALUE);

                CVF_ASSERT(currParseOption->canAcceptMoreValues());
                currParseOption->m_values.push_back(arg);
            }
            else
            {
                m_positionalParams.push_back(arg);
            }
        }

        if (currParseOption.notNull())
        {
            if (!currParseOption->canAcceptMoreValues())
            {
                addNewParsedOption(currParseOption.p());
                currParseOption = NULL;
            }
        }
        
        ia++;
    }

    // Finish the option currently being parsed if any
    if (currParseOption.notNull())
    {
        if (currParseOption->hasEnoughValues())
        {
            addNewParsedOption(currParseOption.p());
        }
        else
        {
            m_optionsWithMissingValues.push_back(currParseOption->m_spec->m_name);
        }

        currParseOption = NULL;
    }


    if (m_unknownOptions.empty() && m_optionsWithMissingValues.empty())
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgramOptions::addNewParsedOption(ParsedOption* parsedOption)
{
    const OptionSpec* optSpec = parsedOption->m_spec.p();
    CVF_ASSERT(optSpec);

    // Check if option already exists
    ref<ParsedOption> existingOption = findParsedOption(optSpec->m_name);


    if (existingOption.notNull())
    {
        if (optSpec->m_optionFlags.testFlag(COMBINE_REPEATED))
        {
            // Add incoming values to the already existing ones
            existingOption->m_values.insert(existingOption->m_values.end(), parsedOption->m_values.begin(), parsedOption->m_values.end());
        }
        else
        {
            // Last occurence wins
            // Erase existing and add ourselves at the end
            m_parsedOptions.erase(existingOption.p());
            m_parsedOptions.push_back(parsedOption);
        }
    }
    else
    {
        // First time we see this option so just add it
        m_parsedOptions.push_back(parsedOption);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ProgramOptions::hasOption(const String& optionName) const
{
    if (findParsedOption(optionName))
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Option ProgramOptions::option(const String& optionName) const
{
    const ParsedOption* parsedOption = findParsedOption(optionName);
    if (parsedOption)
    {
        return Option(parsedOption->m_spec->m_name, parsedOption->m_values);
    }
    else
    {
        return Option();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ProgramOptions::firstValue(const String& optionName) const
{
    const ParsedOption* parsedOption = findParsedOption(optionName);
    if (parsedOption && parsedOption->m_values.size() > 0)
    {
        return parsedOption->m_values[0];
    }
    else
    {
        return String();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<String> ProgramOptions::values(const String& optionName) const
{
    const ParsedOption* parsedOption = findParsedOption(optionName);
    if (parsedOption)
    {
        return parsedOption->m_values;
    }
    else
    {
        return std::vector<String>();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<String> ProgramOptions::positionalParameters() const
{
    return m_positionalParams;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<String> ProgramOptions::unknownOptions() const
{
    return m_unknownOptions;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<String> ProgramOptions::optionsWithMissingValues() const
{
    return m_optionsWithMissingValues;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ProgramOptions::prefixString() const
{
    switch (m_optionPrefix)
    {
        case DOUBLE_DASH:   return "--";
        case SINGLE_DASH:   return "-";
        case SLASH:         return "/";
        default:            
            CVF_FAIL_MSG("Unhandled option prefix type");
            return "UNKNOWN";
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ProgramOptions::OptionSpec* ProgramOptions::findOptionSpec(const String& optionName) const
{
    const String lcName = optionName.toLower();
    for (size_t i = 0; i < m_optionSpecs.size(); i++)
    {
        if (m_optionSpecs[i]->m_name.toLower() == lcName)
        {
            return m_optionSpecs.at(i);
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ProgramOptions::ParsedOption* ProgramOptions::findParsedOption(const String& optionName) const
{
    const String lcName = optionName.toLower();
    for (size_t i = 0; i < m_parsedOptions.size(); i++)
    {
        if (m_parsedOptions[i]->m_spec->m_name.toLower() == lcName)
        {
            return m_parsedOptions.at(i);
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ProgramOptions::ParsedOption* ProgramOptions::findParsedOption(const String& optionName)
{
    // Utilize const version
    return const_cast<ParsedOption*>( (static_cast<const ProgramOptions*>(this))->findParsedOption(optionName) );
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ProgramOptions::usageText(int maxWidth, int maxOptionWidth) const
{
    if (maxWidth <= 1) maxWidth = 2;
    if (maxOptionWidth <= 0) maxOptionWidth = maxWidth/2;

    const String prefixStr = prefixString();

    std::vector<String> optAndValArr;
    std::vector<String> descrArr;
    int optAndValMaxLen = 0;

    const size_t numOpts = m_optionSpecs.size();
    for (size_t i = 0; i < numOpts; i++)
    {
        const OptionSpec* spec = m_optionSpecs.at(i);
        String optAndVal = prefixStr + spec->m_name + String(" ") + spec->m_valueSyntax;
        optAndValMaxLen = CVF_MAX(optAndValMaxLen, static_cast<int>(optAndVal.size()));

        optAndValArr.push_back(optAndVal);
        descrArr.push_back(spec->m_descr);
    }


    const int firstColWidth = static_cast<int>(CVF_MIN(optAndValMaxLen + 1, maxOptionWidth));
    const String firstColBlanks = String("%1").arg("", firstColWidth);

    String retStr;
    for (size_t iopt = 0; iopt < numOpts; iopt++)
    {
        const String optAndVal = optAndValArr[iopt];

        const int maxDescrWidth = CVF_MAX((maxWidth - firstColWidth), 1);
        std::vector<String> descrLines = breakStringIntoLines(descrArr[iopt], static_cast<size_t>(maxDescrWidth));

        String s = String("%1 ").arg(optAndValArr[iopt], -(firstColWidth - 1));
        if (s.size() > static_cast<size_t>(firstColWidth) && descrLines.size() > 0)
        {   
            s += "\n" + firstColBlanks;
        }

        for (size_t i = 0; i < descrLines.size(); i++)
        {
            if (i > 0)
            {
                s += "\n" + firstColBlanks;
            }
            s += descrLines[i];
        }

        retStr += s + String("\n");
    }

    return retStr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<String> ProgramOptions::breakStringIntoLines(const String& str, size_t maxCharsPerLine)
{
    std::vector<String> words = str.split();
    if (words.size() < 1)
    {
        return std::vector<String>();
    }

    std::vector<String> lines;

    String currLine = words[0];
    const size_t numWords = words.size();
    for (size_t i = 1; i < numWords; i++)
    {
        const String& thisWord = words[i];
        const size_t thisWordLen = thisWord.size();

        if (currLine.size() + 1 + thisWordLen <= maxCharsPerLine)
        {
            currLine += String(" ") + thisWord;
        }
        else
        {
            lines.push_back(currLine);
            currLine = thisWord;
        }
    }

    lines.push_back(currLine);

    return lines;
}


} // namespace cvf

