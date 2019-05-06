#include "cafQIconProvider.h"

#include "cafAssert.h"

#include <QApplication>

using namespace caf;


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider()
    : m_active(true)
{    
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QString& iconResourceString)
    : m_active(true)
    , m_iconResourceString(iconResourceString)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QPixmap& pixmap)
    : m_iconPixmap(pixmap)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QIconProvider& rhs)
    : m_icon(rhs.m_icon)
    , m_active(rhs.m_active)
    , m_iconResourceString(rhs.m_iconResourceString)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider& QIconProvider::operator=(const QIconProvider& rhs)
{
    m_icon         = rhs.m_icon;
    m_active       = rhs.m_active;
    m_iconResourceString = rhs.m_iconResourceString;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIcon QIconProvider::icon() const
{
    if (m_icon.isNull())
    {
        m_icon = generateIcon();
    }

    if (!m_active)
    {
        QPixmap disabledPixmap = m_icon.pixmap(16, 16, QIcon::Disabled);
        return QIcon(disabledPixmap);
    }

    return m_icon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QIconProvider::isNull() const
{
    if (!isGuiApplication()) return true;

    if (m_iconPixmap.isNull() && m_iconResourceString.isEmpty()) return true;

    return icon().isNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QIconProvider::setActive(bool active)
{
    m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QIconProvider::setIconResourceString(const QString& iconResourceString)
{
    m_iconResourceString = iconResourceString;
    m_icon = QIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QIconProvider::setPixmap(const QPixmap& pixmap)
{
    m_iconPixmap = pixmap;
    m_icon = QIcon();
}

//--------------------------------------------------------------------------------------------------
/// Generate the actual icon. Will generate a NULL-icon if a QtGuiApplication isn't running.
/// Override in a sub-class if you want to generate a custom icon procedurally
//--------------------------------------------------------------------------------------------------
QIcon QIconProvider::generateIcon() const
{
    if (isGuiApplication())
    {
        if (!m_iconPixmap.isNull())
        {
            return QIcon(m_iconPixmap);
        }
        return QIcon(m_iconResourceString);
    }
    return QIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QIconProvider::isGuiApplication()
{
    return dynamic_cast<QApplication*>(QCoreApplication::instance()) != nullptr;
}
