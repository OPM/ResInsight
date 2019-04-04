#include "cafShortenedQLabel.h"

#include <QApplication>
#include <QFontMetrics>
#include <QResizeEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cafShortenedQLabel::cafShortenedQLabel(QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
    : QLabel(parent, f)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize cafShortenedQLabel::minimumSizeHint() const
{
    int minimumWidth = 100;

    QFontMetrics fontMetrics = QApplication::fontMetrics();
    QString labelText = fullText();
    if (!labelText.isEmpty())
    {
        QStringList words = labelText.split(" ");
        if (!words.empty())
        {
            minimumWidth = std::min(fontMetrics.width(labelText), fontMetrics.width(words.front() + "..."));
        }
    }
    QSize minimumSize = QLabel::minimumSizeHint();
    minimumSize.setWidth(std::min(minimumSize.width(), minimumWidth));
    return minimumSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafShortenedQLabel::resizeEvent(QResizeEvent* event)
{
    QString labelText = fullText();
    QFontMetrics fontMetrics = QApplication::fontMetrics();

    if (fontMetrics.width(labelText) < event->size().width())
    {
        setShortText(labelText);
    }
    else
    {
        QString elidedText = fontMetrics.elidedText(labelText, Qt::ElideRight, event->size().width());
        setShortText(elidedText);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafShortenedQLabel::setShortText(const QString& shortText)
{
    if (m_fullLengthText.isEmpty())
    {
        m_fullLengthText = text();
    }
    setText(shortText);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString cafShortenedQLabel::fullText() const
{
    if (!m_fullLengthText.isEmpty())
    {
        return m_fullLengthText;
    }
    return text();
}
