/**
 * DucklingPagerWidget: a carousel page indicator where the dots are lily pads and the current page is shown by a
 * mother duck. Her ducklings swim after her in a line whenever the page changes; she quacks when she arrives.
 */
#ifndef QFUNNYWIDGETS_DUCKLINGPAGER_H
#define QFUNNYWIDGETS_DUCKLINGPAGER_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Page indicator: a duck family on lily pads.
 */
class DucklingPagerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int current READ getCurrent WRITE setCurrent NOTIFY currentChanged)
    Q_PROPERTY(int count READ getCount WRITE setCount)
    Q_PROPERTY(int ducklings READ getDucklings WRITE setDucklings)
    Q_PROPERTY(bool wrap READ isWrap WRITE setWrap)
    Q_PROPERTY(QColor duckColor READ getDuckColor WRITE setDuckColor)
    Q_PROPERTY(QColor ducklingColor READ getDucklingColor WRITE setDucklingColor)
    Q_PROPERTY(QColor padColor READ getPadColor WRITE setPadColor)

public:
    /** One swimming duck. */
    struct Swimmer
    {
        Swimmer(qreal x = 0.0, qreal vx = 0.0) : x(x), vx(vx) {}
        qreal x;
        qreal vx;
    };

    struct Options
    {
        int count = 5;      ///< 2..12
        int current = 0;
        int ducklings = 3;  ///< 0..6
        bool wrap = true;
        QColor duckColor = QColor(QStringLiteral("#F4F1E6"));
        QColor ducklingColor = QColor(QStringLiteral("#FFD43B"));
        QColor padColor = QColor(QStringLiteral("#5FB37A"));
        std::function<void(int)> currentChanged;
    };

    explicit DucklingPagerWidget(QWidget *parent = nullptr);
    explicit DucklingPagerWidget(const Options &options, QWidget *parent = nullptr);

    int getCurrent() const;
    int getCount() const;
    int getDucklings() const;
    bool isWrap() const;
    QColor getDuckColor() const;
    QColor getDucklingColor() const;
    QColor getPadColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void next();
    void previous();
    void setCurrent(int current);
    void setCount(int count);
    void setDucklings(int ducklings);
    void setWrap(bool wrap);
    void setDuckColor(const QColor &color);
    void setDucklingColor(const QColor &color);
    void setPadColor(const QColor &color);

signals:
    void currentChanged(int index);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QList<qreal> layoutPads(qreal &y, qreal &padR, qreal &size) const;
    void ensureDucks();
    void drawDuck(QPainter &p, const QPointF &c, qreal s, qreal rot, bool mother);

    // Options.
    int m_count;
    int m_current;
    int m_ducklings;
    bool m_wrap;
    QColor m_duckColor;
    QColor m_ducklingColor;
    QColor m_padColor;
    // State.
    QList<Swimmer> m_ducks; // [mother, duckling1, ...]
    qreal m_facing = 1.0;
    qreal m_quackAt = -10.0;
    bool m_moving = false;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_DUCKLINGPAGER_H
