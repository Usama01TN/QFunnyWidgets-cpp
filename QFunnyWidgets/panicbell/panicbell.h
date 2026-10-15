/**
 * PanicBellWidget: a notification bell whose stress grows with the unread count: asleep at 0, calm with a few,
 * sweating and swinging past the panic threshold, and in full meltdown (spiral eyes, alarm glow) when the count
 * overflows the badge. Clearing it gives a sigh of relief.
 */
#ifndef QFUNNYWIDGETS_PANICBELL_H
#define QFUNNYWIDGETS_PANICBELL_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Notification bell that panics.
 */
class PanicBellWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int count READ getCount WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(QColor badgeColor READ getBadgeColor WRITE setBadgeColor)
    Q_PROPERTY(int panicThreshold READ getPanicThreshold WRITE setPanicThreshold)
    Q_PROPERTY(int maxDisplay READ getMaxDisplay WRITE setMaxDisplay)
    Q_PROPERTY(bool clearOnClick READ isClearOnClick WRITE setClearOnClick)

public:
    /** Sweat drop (kind 0) or relief puff (kind 1). */
    struct Puff
    {
        Puff(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 0.8, int kind = 0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life), kind(kind) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
        int kind;
    };

    struct Options
    {
        int count = 0;
        QColor color = QColor(QStringLiteral("#FFB703"));
        QColor badgeColor = QColor(QStringLiteral("#E63946"));
        int panicThreshold = 10;  ///< Count at which the bell starts panicking.
        int maxDisplay = 99;      ///< Above this the badge shows "99+" and the bell melts down.
        bool clearOnClick = true;
        std::function<void()> clicked;
        std::function<void(int)> countChanged;
    };

    explicit PanicBellWidget(QWidget *parent = nullptr);
    explicit PanicBellWidget(const Options &options, QWidget *parent = nullptr);

    int getCount() const;
    QColor getColor() const;
    QColor getBadgeColor() const;
    int getPanicThreshold() const;
    int getMaxDisplay() const;
    bool isClearOnClick() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void ring();
    void increment();
    void clear();
    void setCount(int count);
    void setColor(const QColor &color);
    void setBadgeColor(const QColor &color);
    void setPanicThreshold(int panicThreshold);
    void setMaxDisplay(int maxDisplay);
    void setClearOnClick(bool clearOnClick);

signals:
    void clicked();
    void countChanged(int count);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    int stress() const;
    void geometry(QPointF &pivot, qreal &s) const;
    void drawBell(QPainter &p, qreal s, int stress, qreal now);
    void drawBadge(QPainter &p, const QPointF &pivot, qreal s);

    // Options.
    int m_count;
    QColor m_color;
    QColor m_badgeColor;
    int m_panicThreshold;
    int m_maxDisplay;
    bool m_clearOnClick;
    // State.
    qreal m_angle = 0.0; // swing, degrees
    qreal m_angVel = 0.0;
    qreal m_badgePop = 1.0;
    qreal m_badgeVel = 0.0;
    qreal m_dingAt = -10.0;
    qreal m_reliefAt = -10.0;
    QList<Puff> m_puffs;
    qreal m_sweatAcc = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_PANICBELL_H
