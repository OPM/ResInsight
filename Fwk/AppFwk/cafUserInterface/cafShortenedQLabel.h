#pragma once

#include <QLabel>

class cafShortenedQLabel : public QLabel
{
    Q_OBJECT
public:
    explicit cafShortenedQLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
protected:
    void    resizeEvent(QResizeEvent *event) override;
    void    setDisplayText(const QString& shortText);
    QString fullText() const;
    QString firstWord() const;
private:
    QString m_fullLengthText;
};


