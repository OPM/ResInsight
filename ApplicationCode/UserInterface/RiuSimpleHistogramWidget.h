#include <QWidget>
class QPaintEvent;
class QString;
class QStringList;


class RiuSimpleHistogramWidget : public QWidget
{
public:
    RiuSimpleHistogramWidget( QWidget * parent = 0, Qt::WindowFlags f = 0);

    void                setHistogramData(double min, double max, const std::vector<size_t>& histogram);
    void                setPercentiles(double pmin, double pmax);
    void                setMean(double mean) {m_mean = mean;}

protected:
     virtual void       paintEvent(QPaintEvent* event);

private:
    void                draw(QPainter *painter,int x, int y, int width, int height );

    int                 xPosFromColIdx(size_t colIdx);
    int                 yPosFromCount(size_t colHeight);

    int                 xPosFromDomainValue(double value);

    std::vector<size_t> m_histogramData;
    double              m_max;
    double              m_min;
    double              m_minPercentile;
    double              m_maxPercentile;
    double              m_mean;
    size_t              m_maxHistogramCount;

    double              m_width;
    double              m_height;
    double              m_x;
    double              m_y;
};
