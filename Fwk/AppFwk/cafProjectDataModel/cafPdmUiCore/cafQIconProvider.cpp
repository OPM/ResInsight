#include "cafQIconProvider.h"

#include "cafAssert.h"

#include <QApplication>
#include <QPainter>

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
    : m_active(true)
    , m_iconPixmap(new QPixmap(pixmap))
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QIconProvider& rhs)
    : m_icon(rhs.m_icon)
    , m_active(rhs.m_active)
    , m_iconResourceString(rhs.m_iconResourceString)
    , m_backgoundColor(rhs.m_backgoundColor)
{
    copyPixmaps(rhs);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider& QIconProvider::operator=(const QIconProvider& rhs)
{
    m_icon         = rhs.m_icon;
    m_active       = rhs.m_active;
    m_iconResourceString = rhs.m_iconResourceString;
    m_backgoundColor = rhs.m_backgoundColor;

    copyPixmaps(rhs);

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

    if (!m_active && isGuiApplication())
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
    return !isGuiApplication();
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
    m_iconPixmap.reset(new QPixmap(pixmap));
    m_icon = QIcon();
}

//--------------------------------------------------------------------------------------------------
/// Generate the actual icon. Will generate a NULL-icon if a QtGuiApplication isn't running.
/// Override in a sub-class if you want to generate a custom icon procedurally
//--------------------------------------------------------------------------------------------------
QIcon QIconProvider::generateIcon() const
{
    if (!isGuiApplication())
    {
        return QIcon();
    }

    QIcon generatedIcon;

    if (m_backgoundColor.isValid())
    {
        QPixmap pixmap(16,16);
        pixmap.fill(m_backgoundColor);
            
        generatedIcon = QIcon(pixmap);
    }
    else if (hasValidPixmap())
    {
        generatedIcon = QIcon(*m_iconPixmap);
    }
    else
    {
        generatedIcon = QIcon(m_iconResourceString);
    }

    if (m_overlayPixmap && !m_overlayPixmap->isNull())
    {
        QPixmap pixmap;
        pixmap = generatedIcon.pixmap(16, 16, QIcon::Normal);

        QPainter painter(&pixmap);
        painter.drawPixmap(0, 0, *m_overlayPixmap);

        generatedIcon = QIcon(pixmap);
    }

    return generatedIcon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QIconProvider::isGuiApplication()
{
    return dynamic_cast<QApplication*>(QCoreApplication::instance()) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QIconProvider::hasValidPixmap() const
{
    return m_iconPixmap && !m_iconPixmap->isNull();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::QIconProvider::setOverlayPixmap(const QPixmap& pixmap)
{
    m_overlayPixmap.reset(new QPixmap(pixmap));
    m_icon = QIcon();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::QIconProvider::setBackgroundColor(const QColor& color)
{
    if (m_backgoundColor != color)
    {
        m_backgoundColor = color;
        m_icon = QIcon();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::QIconProvider::copyPixmaps(const QIconProvider& other)
{
    if (other.m_iconPixmap)
    {
        m_iconPixmap.reset(new QPixmap(*other.m_iconPixmap));
    }

    if (other.m_overlayPixmap)
    {
        m_overlayPixmap.reset(new QPixmap(*other.m_overlayPixmap));
    }
}
