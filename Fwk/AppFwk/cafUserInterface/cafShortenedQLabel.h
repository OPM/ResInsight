#pragma once

#include <QLabel>

class cafShortenedQLabel : public QLabel
{
    Q_OBJECT
public:
    explicit cafShortenedQLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    QSize minimumSizeHint() const override;
protected:
    void    resizeEvent(QResizeEvent *event) override;
    void    setShortText(const QString& shortText);
    QString fullText() const;

private:
    QString m_fullLengthText;
};


