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
    int minimumWidth = 0;

    QFontMetrics fontMetrics = QApplication::fontMetrics();
    QString fullLabelText = fullText();
    if (!fullLabelText.isEmpty())
    {
        minimumWidth = 10;

        QStringList words = fullLabelText.split(" ");
        if (!words.empty())
        {
            int textMinimumWidth = std::min(fontMetrics.width(fullLabelText), fontMetrics.width(words.front() + "..."));
            minimumWidth = std::max(minimumWidth, textMinimumWidth);
        }
    }
    QSize minimumSize = QLabel::minimumSizeHint();
    minimumSize.setWidth(minimumWidth);
    return minimumSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize cafShortenedQLabel::sizeHint() const
{
    QFontMetrics fontMetrics = QApplication::fontMetrics();
    QString      labelText   = fullText();
    QSize        size        = QLabel::sizeHint();
    size.setWidth(fontMetrics.width(labelText));
    return size;
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
        setDisplayText(labelText);
    }
    else
    {
        int width = std::max(minimumSizeHint().width(), event->size().width());
        QString elidedText = fontMetrics.elidedText(labelText, Qt::ElideRight, width);
        setDisplayText(elidedText);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafShortenedQLabel::setDisplayText(const QString& shortText)
{
    // Store original text if we haven't already done so.
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
