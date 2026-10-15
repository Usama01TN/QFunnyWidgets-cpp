/**
 * PancakeStepperWidget: a quantity spin box served as a stack of pancakes. Incrementing drops a pancake onto the
 * stack (it bounces and squashes), decrementing flips the top one off the plate; a tall stack sways nervously
 * and the stepper refuses to go above the maximum.
 */
#ifndef QFUNNYWIDGETS_PANCAKESTEPPER_H
#define QFUNNYWIDGETS_PANCAKESTEPPER_H

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
 * Spin box made of pancakes.
 */
class PancakeStepperWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ getValue WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int minimum READ getMinimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ getMaximum WRITE setMaximum)
    Q_PROPERTY(QString unit READ getUnit WRITE setUnit)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)

public:
    /** A pancake on the stack (offset = height above its resting place while falling). */
    struct Drop
    {
        explicit Drop(qreal offset = 0.0, qreal vel = 0.0) : offset(offset), vel(vel) {}
        qreal offset;
        qreal vel;
        qreal squash = 0.0;
    };

    /** A pancake flipped off the stack. */
    struct Flip
    {
        Flip(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal vrot = 0.0)
            : x(x), y(y), vx(vx), vy(vy), vrot(vrot) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal rot = 0.0;
        qreal vrot;
        qreal life = 1.2;
    };

    struct Options
    {
        int value = 2;
        int minimum = 0;
        int maximum = 8;  ///< up to 20
        QString unit = QStringLiteral("pancake");
        QColor accentColor = QColor(QStringLiteral("#C47F3A"));
        std::function<void(int)> valueChanged;
    };

    explicit PancakeStepperWidget(QWidget *parent = nullptr);
    explicit PancakeStepperWidget(const Options &options, QWidget *parent = nullptr);

    int getValue() const;
    int getMinimum() const;
    int getMaximum() const;
    QString getUnit() const;
    QColor getAccentColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void increment();
    void decrement();
    void setValue(int value);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setUnit(const QString &unit);
    void setAccentColor(const QColor &color);

signals:
    void valueChanged(int value);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void geometry(QPointF &plate, qreal &pw, qreal &th) const;
    void refuse();
    void drawPancake(QPainter &p, const QPointF &c, qreal pw, qreal th, qreal squash, bool top, qreal now);

    // Options.
    int m_minimum;
    int m_maximum;
    int m_value;
    QString m_unit;
    QColor m_accentColor;
    // State.
    QList<Drop> m_drops;
    QList<Flip> m_flips;
    qreal m_sway = 0.0;
    qreal m_swayVel = 0.0;
    qreal m_refuseAt = -10.0;
    QRectF m_minusRect;
    QRectF m_plusRect;
    int m_hoverBtn = 0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_PANCAKESTEPPER_H
