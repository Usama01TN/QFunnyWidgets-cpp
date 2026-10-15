/**
 * RunawayButtonWidget: a pill button that dodges the cursor with cartoon spring physics, taunts the user in a
 * speech bubble, gets tired after a number of escapes and celebrates with confetti when it is finally caught.
 */
#ifndef QFUNNYWIDGETS_RUNAWAYBUTTON_H
#define QFUNNYWIDGETS_RUNAWAYBUTTON_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * A button that runs away from the cursor.
 */
class RunawayButtonWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText)
    Q_PROPERTY(QString tiredText READ getTiredText WRITE setTiredText)
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)
    Q_PROPERTY(QStringList taunts READ getTaunts WRITE setTaunts)
    Q_PROPERTY(qreal dodgeRadius READ getDodgeRadius WRITE setDodgeRadius)
    Q_PROPERTY(int maxEscapes READ getMaxEscapes WRITE setMaxEscapes)
    Q_PROPERTY(int autoReset READ getAutoReset WRITE setAutoReset)

public:
    /** One confetti particle. */
    struct Particle
    {
        qreal x = 0.0;
        qreal y = 0.0;
        qreal vx = 0.0;
        qreal vy = 0.0;
        qreal rot = 0.0;
        qreal vrot = 0.0;
        QColor color = QColor(QStringLiteral("#FFD23F"));
        qreal life = 1.0;
    };

    static constexpr qreal BTN_W = 150.0;
    static constexpr qreal BTN_H = 50.0;

    /** The default taunts shown in the speech bubble. */
    static QStringList defaultTaunts();

    struct Options
    {
        QString text = QStringLiteral("Click me!");
        QString tiredText = QStringLiteral("Fine... click");
        QColor color = QColor(QStringLiteral("#FF4D6D"));
        QColor textColor = QColor(QStringLiteral("#FFFFFF"));
        qreal dodgeRadius = 32.0;   ///< Cursor distance (px) that triggers an escape.
        int maxEscapes = 7;         ///< Escapes before it gets tired (0 = never).
        QStringList taunts = RunawayButtonWidget::defaultTaunts();
        int autoReset = 3000;       ///< ms after being caught before it resets (0 = never).
        std::function<void()> clicked;
        std::function<void(int)> escaped;
        std::function<void(int)> caught;
    };

    explicit RunawayButtonWidget(QWidget *parent = nullptr);
    explicit RunawayButtonWidget(const Options &options, QWidget *parent = nullptr);

    int getEscapes() const;
    QString getText() const;
    QString getTiredText() const;
    QColor getColor() const;
    QColor getTextColor() const;
    qreal getDodgeRadius() const;
    int getMaxEscapes() const;
    QStringList getTaunts() const;
    int getAutoReset() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void reset();
    void setText(const QString &text);
    void setTiredText(const QString &tiredText);
    void setColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setDodgeRadius(qreal dodgeRadius);
    void setMaxEscapes(int maxEscapes);
    void setTaunts(const QStringList &taunts);
    void setAutoReset(int autoReset);

signals:
    void clicked();
    void escaped(int count);
    void caught(int escapes);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    bool isTired() const;
    QRectF buttonRect() const;
    void ensurePos();
    QPointF clampCentre(const QPointF &pt) const;
    void escape(const QPointF &cursor);
    void burst(const QPointF &c);
    void drawBubble(QPainter &p, const QString &text, qreal now);

    // Options.
    QString m_text;
    QString m_tiredText;
    QColor m_color;
    QColor m_textColor;
    qreal m_dodgeRadius;
    int m_maxEscapes;
    QStringList m_taunts;
    int m_autoReset;
    // State.
    QPointF m_pos = QPointF(-1, -1); // button center
    QPointF m_target = QPointF(-1, -1);
    QPointF m_vel = QPointF(0, 0);
    int m_escapes = 0;
    QString m_taunt;
    qreal m_tauntUntil = 0.0;
    qreal m_caughtAt = -1.0;
    bool m_pressed = false;
    bool m_hover = false;
    QPointF m_mouse = QPointF(-999, -999);
    QList<Particle> m_particles;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_RUNAWAYBUTTON_H
