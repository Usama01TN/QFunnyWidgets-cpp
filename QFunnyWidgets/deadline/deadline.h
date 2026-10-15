/**
 * DeadlineCountdownWidget: a countdown timer whose desk calendar copes progressively worse: relaxed with sunglasses,
 * focused, nervous and sweating, panicking with trembling digits, and finally on fire saying "this is fine".
 * Supports pause/resume, extensions (with a sigh of relief) and a time scale for demos.
 */
#ifndef QFUNNYWIDGETS_DEADLINE_H
#define QFUNNYWIDGETS_DEADLINE_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QRectF>
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

class QPainter;
class QTimer;

/** "M:SS"-style formatting: "05:07", or "1:02:03" with hours. */
QFUNNYWIDGETS_EXPORT QString formatTime(qreal seconds);

/**
 * Countdown with a calendar that ends up "fine" in flames.
 */
class QFUNNYWIDGETS_EXPORT DeadlineCountdownWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int duration READ getDuration WRITE setDuration)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(qreal timeScale READ getTimeScale WRITE setTimeScale)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)

public:
    /** One flame particle. */
    struct Flame
    {
        Flame(qreal x = 0.0, qreal y = 0.0, qreal vy = 0.0, qreal size = 6.0, qreal life = 0.8)
            : x(x), y(y), vy(vy), size(size), life(life), maxLife(life) {}
        qreal x;
        qreal y;
        qreal vy;
        qreal size;
        qreal life;
        qreal maxLife;
    };

    struct Options
    {
        int duration = 60;  ///< seconds
        QString title = QStringLiteral("Quarterly report due");
        qreal timeScale = 1.0;
        bool autoStart = true;
        QColor accentColor = QColor(QStringLiteral("#E63946"));
        std::function<void()> expired;
        std::function<void(int)> remainingChanged;
    };

    explicit DeadlineCountdownWidget(QWidget *parent = nullptr);
    explicit DeadlineCountdownWidget(const Options &options, QWidget *parent = nullptr);

    qreal getRemaining() const;
    bool isRunning() const;
    int getDuration() const;
    QString getTitle() const;
    qreal getTimeScale() const;
    QColor getAccentColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    /** (Re)start; seconds >= 0 also sets a new duration. */
    void start(int seconds = -1);
    void pause();
    void resume();
    void extend(int seconds);
    void setDuration(int duration);
    void setTitle(const QString &title);
    void setTimeScale(qreal timeScale);
    void setAccentColor(const QColor &color);

signals:
    void expired();
    void remainingChanged(int seconds);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QRectF calRect() const;
    void drawFlames(QPainter &p, bool back);
    void drawCalendar(QPainter &p, const QRectF &cal, qreal frac, qreal rem, bool relieved, qreal now);

    // Options.
    int m_duration;
    QString m_title;
    qreal m_timeScale;
    QColor m_accentColor;
    // State.
    QElapsedTimer m_clock;
    qreal m_base;              // remaining at last (re)start
    qreal m_runFrom = 0.0;
    bool m_running = false;
    bool m_expiredSent = false;
    int m_lastWhole = -999999;
    qreal m_reliefAt = -10.0;
    QList<Flame> m_flames;
    qreal m_flameAcc = 0.0;
    qreal m_last = 0.0;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_DEADLINE_H
