#include <QWidget>
class QPaintEvent;
class QString;
class QStringList;


class RiuSimpleHistogramWidget : public QWidget
{
public:
    RiuSimpleHistogramWidget( QWidget * parent = 0, Qt::WindowFlags f = 0);

    void                setHistogramData(double min, double max, const std::vector<size_t>& histogram);
    void                setPercentiles(double pmin, double pmax) {m_minPercentile = pmin; m_maxPercentile = pmax;}

protected:
     virtual void       paintEvent(QPaintEvent* event);

private:
    void                draw(QPainter *painter,int x, int y, int width, int height );

    int                 xPosFromColIdx(size_t colIdx)          { return  (int)(m_x + 1 + (m_width - 2 ) * colIdx/m_histogramData.size());}
    int                 yPosFromCount(size_t colHeight)        { return  (int)(m_y + m_height - 1 - (m_height - 3 ) * colHeight/m_maxHistogramCount);}

    int                 xPosFromDomainValue(double value) { double range = m_max - m_min; return  (range == 0.0) ? (m_x + 1) : (int)(m_x + 1 + (m_width - 2 ) * (value - m_min)/(m_max - m_min));}

    std::vector<size_t> m_histogramData;
    double              m_max;
    double              m_min;
    double              m_minPercentile;
    double              m_maxPercentile;
    size_t              m_maxHistogramCount;

    double              m_width;
    double              m_height;
    double              m_x;
    double              m_y;
};
