/**
 * BatteryBuddyWidget: a battery status indicator with moods: bouncing and energized when full, getting sleepy
 * (heavy eyelids, yawning), sweating when low, flickering and dizzy when almost empty, and happily munching with
 * a lightning bolt and sparks while charging.
 */
#ifndef QFUNNYWIDGETS_BATTERYBUDDY_H
#define QFUNNYWIDGETS_BATTERYBUDDY_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Battery status with feelings.
 */
class BatteryBuddyWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
    Q_PROPERTY(bool charging READ isCharging WRITE setCharging NOTIFY chargingChanged)
    Q_PROPERTY(int lowThreshold READ getLowThreshold WRITE setLowThreshold)
    Q_PROPERTY(QColor caseColor READ getCaseColor WRITE setCaseColor)
    Q_PROPERTY(bool showLabel READ isShowLabel WRITE setShowLabel)

public:
    /** A charging spark. */
    struct Spark
    {
        qreal x = 0.0;
        qreal y = 0.0;
        qreal vx = 0.0;
        qreal vy = 0.0;
        qreal life = 0.4;
    };

    struct Options
    {
        int level = 75;          ///< 0..100
        bool charging = false;
        int lowThreshold = 20;   ///< 1..99
        QColor caseColor = QColor(QStringLiteral("#8B8FA3"));
        bool showLabel = true;
        std::function<void(int)> levelChanged;
        std::function<void(bool)> chargingChanged;
        std::function<void()> lowBattery;
    };

    explicit BatteryBuddyWidget(QWidget *parent = nullptr);
    explicit BatteryBuddyWidget(const Options &options, QWidget *parent = nullptr);

    int getLevel() const;
    bool isCharging() const;
    int getLowThreshold() const;
    QColor getCaseColor() const;
    bool isShowLabel() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setLevel(int level);
    void setCharging(bool charging);
    void setLowThreshold(int lowThreshold);
    void setCaseColor(const QColor &color);
    void setShowLabel(bool showLabel);

signals:
    void levelChanged(int level);
    void chargingChanged(bool charging);
    void lowBattery();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void geometry(QRectF &body, qreal &labelTop) const;
    QString status() const;
    void drawFace(QPainter &p, const QPointF &c, qreal r, int lv, bool chg, qreal now);
    static void drawBolt(QPainter &p, const QPointF &c, qreal s);

    // Options.
    int m_level;
    bool m_charging;
    int m_lowThreshold;
    QColor m_caseColor;
    bool m_showLabel;
    // State.
    qreal m_shown;
    QList<Spark> m_sparks;
    qreal m_sparkAcc = 0.0;
    qreal m_blinkAt = 2.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_BATTERYBUDDY_H
