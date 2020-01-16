//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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
#include "cafUiStyleSheet.h"

#include <QStringList>
#include <QStyle>
#include <QVariant>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet::State::State(const QString& stateTag, Type type)
    : m_type(type)
{
    if (type == PseudoState)
    {
        if (!stateTag.isEmpty())
        {
            m_stateTag = ":" + stateTag;
        }
    }
    else
    {
        m_stateTag = QString("[%1=true]").arg(stateTag);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet::State::Type caf::UiStyleSheet::State::type() const
{
    return m_type;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::State::set(const QString& key, const QString& value)
{
    m_content[key] = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::UiStyleSheet::State::get(const QString& key) const
{
    auto it = m_content.find(key);
    if (it != m_content.end())
    {
        return it->second;
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::UiStyleSheet::State::fullText(const QString& className, const QString& objectName, bool applyToSubClasses /*= true*/) const
{
    QString classNameTag = applyToSubClasses ? className : QString(".%1").arg(className);
    QString objectNameTag = !objectName.isEmpty() ? QString("#%1").arg(objectName) : "";

    QStringList content;
    for (auto keyValuePair : m_content)
    {
        content << QString("  %1: %2").arg(keyValuePair.first).arg(keyValuePair.second);
    }

    QString format("%1%2%3\n{\n%4}");
    QString stateStyleSheet = format.arg(classNameTag).arg(objectNameTag).arg(m_stateTag).arg(content.join(";\n"));
    return stateStyleSheet;

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet::UiStyleSheet()
{
    // Add a default state (blank tag)
    m_states.insert(std::make_pair("", State("", State::PseudoState)));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::set(const QString& key, const QString& value)
{
    pseudoState("").set(key, value);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::UiStyleSheet::get(const QString& key) const
{
    auto it = m_states.find("");
    if (it != m_states.end())
    {
        return it->second.get(key);
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet::State& caf::UiStyleSheet::property(QString stateTag)
{
    auto itBoolResult = m_states.insert(std::make_pair(stateTag, State(stateTag, State::PropertyState)));
    return itBoolResult.first->second;

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet::State& caf::UiStyleSheet::pseudoState(QString stateTag)
{
    auto itBoolResult = m_states.insert(std::make_pair(stateTag, State(stateTag, State::PseudoState)));
    return itBoolResult.first->second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::applyToWidget(QWidget* widget, bool applyToSubClasses /*= true*/) const
{
    if (widget->objectName().isEmpty())
    {
        // If widget has no object name we use the pointer as a name.
        widget->setObjectName(QString("%1").arg(reinterpret_cast<std::uintptr_t>(widget)));
    }
    QString completeStyleSheet = fullText(QString(widget->metaObject()->className()), widget->objectName(), applyToSubClasses);
    widget->setStyleSheet(completeStyleSheet);
    refreshWidget(widget);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::applyToWidgetAndChildren(QWidget* widget)
{
    QString completeStyleSheet = fullText("*", "", false);
    widget->setStyleSheet(completeStyleSheet);
    refreshWidget(widget);
}

//--------------------------------------------------------------------------------------------------
/// Clear all existing properties
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::clearWidgetStates(QWidget* widget)
{
    for (QByteArray existingProperty : widget->dynamicPropertyNames())
    {
        widget->setProperty(existingProperty, QVariant());
    }
    refreshWidget(widget);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::setWidgetState(QWidget* widget, QString stateTag, bool on /*= true*/) const
{
    // Set current property state to true
    if (!stateTag.isEmpty())
    {
        auto it = m_states.find(stateTag);
        if (it != m_states.end() && it->second.type() == State::PropertyState)
        {
            widget->setProperty(stateTag.toLatin1(), QVariant(on));
        }
    }

    // Trigger style update
    refreshWidget(widget);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::stashWidgetStates()
{
    m_stashedStates.swap(m_states);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::restoreWidgetStates()
{
    if (!m_stashedStates.empty())
    {
        m_stashedStates.swap(m_states);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::UiStyleSheet::fullText(const QString& className, const QString& objectName, bool applyToSubClasses) const
{
    QStringList stateTexts;
    for (auto it = m_states.begin(); it != m_states.end(); ++it)
    {
        stateTexts << it->second.fullText(className, objectName, applyToSubClasses);
    }
    return stateTexts.join("\n");

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiStyleSheet::refreshWidget(QWidget* widget)
{
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}
