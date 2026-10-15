/**
 * RocketBackToTop: a "back to top" button for scroll areas. Create it with a QAbstractScrollArea as parent: once
 * the user scrolls past the threshold a little rocket rises into the bottom-right corner of the viewport; clicking
 * it makes it rumble and blast off, scrolling the content back to the top. Next time it floats back down on a
 * parachute.
 */
#ifndef QFUNNYWIDGETS_ROCKETTOP_H
#define QFUNNYWIDGETS_ROCKETTOP_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QPointer>
#include <QWidget>
#include <functional>

// Symbol export for the shared library build (empty for the static library, or when compiling the sources directly).
#ifndef QFUNNYWIDGETS_EXPORT
#  if defined(QFUNNYWIDGETS_BUILD_SHARED)
#    define QFUNNYWIDGETS_EXPORT Q_DECL_EXPORT
#  elif defined(QFUNNYWIDGETS_SHARED)
#    define QFUNNYWIDGETS_EXPORT Q_DECL_IMPORT
#  else
#    define QFUNNYWIDGETS_EXPORT
#  endif
#endif

class QAbstractScrollArea;
class QPainter;
class QTimer;

/**
 * Back-to-top rocket overlay for a scroll area.
 */
class QFUNNYWIDGETS_EXPORT RocketBackToTop : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int threshold READ getThreshold WRITE setThreshold)
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(QColor bodyColor READ getBodyColor WRITE setBodyColor)
    Q_PROPERTY(QString label READ getLabel WRITE setLabel)

public:
    static const int STRIP = 84;
    static constexpr qreal RUMBLE = 0.35;
    static constexpr qreal FLIGHT = 0.9;

    /** One smoke puff. */
    struct Smoke
    {
        Smoke(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 1.0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
    };

    struct Options
    {
        int threshold = 300;  ///< Scroll position (px) after which the rocket appears.
        QColor color = QColor(QStringLiteral("#E63946"));
        QColor bodyColor = QColor(QStringLiteral("#EEF0F5"));
        QString label = QStringLiteral("TOP");
        std::function<void()> launched;
    };

    explicit RocketBackToTop(QWidget *parent = nullptr);
    RocketBackToTop(QWidget *parent, const Options &options);
    explicit RocketBackToTop(const Options &options, QWidget *parent = nullptr);

    int getThreshold() const;
    QColor getColor() const;
    QColor getBodyColor() const;
    QString getLabel() const;

public slots:
    void launch();
    void setThreshold(int threshold);
    void setColor(const QColor &color);
    void setBodyColor(const QColor &color);
    void setLabel(const QString &label);

signals:
    void launched();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *e) override;
#else
    void enterEvent(QEvent *e) override;
#endif
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();
    void onScroll(int value);

private:
    enum Mode { HIDDEN, RISING, IDLE, SINKING, LAUNCH, PARACHUTE };

    qreal now() const;
    qreal home() const;
    void place();
    void setMode(Mode mode);
    void updateMask();
    void drawRocket(QPainter &p, const QPointF &c, Mode mode, qreal since, qreal now);
    void drawParachute(QPainter &p, const QPointF &c, qreal now);

    // Options.
    int m_threshold;
    QColor m_color;
    QColor m_bodyColor;
    QString m_label;
    // State.
    Mode m_mode = HIDDEN;
    qreal m_modeAt = 0.0;
    qreal m_y = 0.0;
    qreal m_vy = 0.0;
    int m_startScroll = 0;
    bool m_flown = false;
    bool m_hovered = false;
    QList<Smoke> m_smoke;
    qreal m_smokeAcc = 0.0;
    qreal m_last = 0.0;
    QPointer<QAbstractScrollArea> m_area;
    QPointer<QWidget> m_viewport;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_ROCKETTOP_H
