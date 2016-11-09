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


#pragma once

#include "cvfFlags.h"
#include "cvfString.h"
#include "cvfCollection.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class Option
{
public:
    Option();
    Option(const String& name, const std::vector<String>& values);

    String              name() const;
    size_t              valueCount() const;
    String              value(size_t valueIndex) const;
    String              safeValue(size_t valueIndex) const;
    std::vector<String> values() const;
    String              combinedValues() const;

    bool                isValid() const;

    // Safe bool idiom, internally calls isValid()
    typedef void (Option::*bool_type)() const;
    operator bool_type() const;

private:
    const String                m_name;
    const std::vector<String>   m_values;

private:
    void this_type_does_not_support_comparisons() const {}
};


//==================================================================================================
//
// 
//
//==================================================================================================
class ProgramOptions
{
public:
    enum OptionPrefix
    {
        DOUBLE_DASH,        
        SINGLE_DASH,
        SLASH       
    };

    enum ValueReq
    {
        NO_VALUE,               // A flag option that does not accept any values
        SINGLE_VALUE,           // Option requires exactly one single value, no more, no less
        MULTI_VALUE,            // Option requires one or more values
        OPTIONAL_MULTI_VALUE    // Option can have none, one or more values
    };

    enum OptionFlag
    {
        COMBINE_REPEATED = 0x01,    // When this flag is specified and an option occurs multiple times, the values will be combined. Default is for the last occurrence to win
        UNDOCUMENTED = 0x02         // Will not be output in help text
    };
    typedef cvf::Flags<OptionFlag> OptionFlags;

public:
    ProgramOptions();
    ~ProgramOptions();

    void                setOptionPrefix(OptionPrefix prefix);
    bool                registerOption(const String& optionName, ValueReq valueReq = NO_VALUE, OptionFlags optionFlags = OptionFlags());
    bool                registerOption(const String& optionName, const String& valueSyntax, const String& description, ValueReq valueReq = NO_VALUE, OptionFlags optionFlags = OptionFlags());

    bool                parse(const std::vector<String>& commandLineArguments);

    bool                hasOption(const String& optionName) const;
    Option              option(const String& optionName) const;
    std::vector<String> values(const String& optionName) const;
    String              firstValue(const String& optionName) const;

    std::vector<String> positionalParameters() const;

    std::vector<String> unknownOptions() const;
    std::vector<String> optionsWithMissingValues() const;

    String              usageText(int maxWidth, int maxOptionWidth = -1) const;

private:
    class OptionSpec;
    class ParsedOption;

    String                      prefixString() const;
    const OptionSpec*           findOptionSpec(const String& optionName) const;
    const ParsedOption*         findParsedOption(const String& optionName) const;
    ParsedOption*               findParsedOption(const String& optionName);
    void                        addNewParsedOption(ParsedOption* parsedOption);
    static std::vector<String>  breakStringIntoLines(const String& str, size_t maxCharsPerLine);

private:
    OptionPrefix                m_optionPrefix;             // The prefix to use to identify options
    Collection<OptionSpec>      m_optionSpecs;              // Collection of legal registered options
    Collection<ParsedOption>    m_parsedOptions;            // The options we have successfully parsed
    std::vector<String>         m_positionalParams;         // Array of positional parameters
    std::vector<String>         m_unknownOptions;           // Unrecognized options
    std::vector<String>         m_optionsWithMissingValues; // Options that failed during parsing due to missing values
};

}

