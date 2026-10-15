/**
 * SnailStepperWidget: a wizard step indicator where a snail crawls (slowly, in pulses) from step to step, leaving
 * a shiny slime trail and complaining when you rush it. Steps are clickable.
 */
#ifndef QFUNNYWIDGETS_SNAILSTEPPER_H
#define QFUNNYWIDGETS_SNAILSTEPPER_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QStringList>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Step indicator carried by a slow snail.
 */
class SnailStepperWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int current READ getCurrent WRITE setCurrent NOTIFY currentChanged)
    Q_PROPERTY(QStringList steps READ getSteps WRITE setSteps)
    Q_PROPERTY(qreal speed READ getSpeed WRITE setSpeed)
    Q_PROPERTY(bool clickable READ isClickable WRITE setClickable)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)
    Q_PROPERTY(QColor shellColor READ getShellColor WRITE setShellColor)

public:
    static QStringList defaultSteps();

    struct Options
    {
        QStringList steps = SnailStepperWidget::defaultSteps();
        int current = 0;
        qreal speed = 70.0;  ///< px/s (on average)
        bool clickable = true;
        QColor accentColor = QColor(QStringLiteral("#3BCEAC"));
        QColor shellColor = QColor(QStringLiteral("#E07A5F"));
        std::function<void(int)> currentChanged;
        std::function<void(int)> arrived;
    };

    explicit SnailStepperWidget(QWidget *parent = nullptr);
    explicit SnailStepperWidget(const Options &options, QWidget *parent = nullptr);

    int getCurrent() const;
    QStringList getSteps() const;
    qreal getSpeed() const;
    bool isClickable() const;
    QColor getAccentColor() const;
    QColor getShellColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void next();
    void previous();
    void setCurrent(int current);
    void setSteps(const QStringList &steps);
    void setSpeed(qreal speed);
    void setClickable(bool clickable);
    void setAccentColor(const QColor &color);
    void setShellColor(const QColor &color);

signals:
    void currentChanged(int index);
    void arrived(int index);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QList<qreal> layoutSteps(qreal &y, qreal &r, qreal &size) const;
    void drawSnail(QPainter &p, const QPointF &base, qreal s, qreal now);

    // Options.
    QStringList m_steps;
    int m_current;
    qreal m_speed;
    bool m_clickable;
    QColor m_accentColor;
    QColor m_shellColor;
    // State.
    qreal m_x = -1.0;     // snail position, px
    qreal m_trail = -1.0; // furthest slime point
    qreal m_facing = 1.0;
    bool m_moving = false;
    qreal m_crawl = 0.0;
    qreal m_arrivedAt = -10.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_SNAILSTEPPER_H
