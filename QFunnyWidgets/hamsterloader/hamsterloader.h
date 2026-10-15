/**
 * HamsterLoaderWidget: a progress indicator powered by a hamster running in a wheel. The hamster speeds up as
 * progress grows, sweats near the end, celebrates with confetti when done and shows rotating funny messages.
 * A negative progress means indeterminate.
 */
#ifndef QFUNNYWIDGETS_HAMSTERLOADER_H
#define QFUNNYWIDGETS_HAMSTERLOADER_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QStringList>
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

/**
 * Hamster-wheel progress indicator.
 */
class QFUNNYWIDGETS_EXPORT HamsterLoaderWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal progress READ getProgress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(qreal speed READ getSpeed WRITE setSpeed)
    Q_PROPERTY(QColor hamsterColor READ getHamsterColor WRITE setHamsterColor)
    Q_PROPERTY(QColor wheelColor READ getWheelColor WRITE setWheelColor)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)
    Q_PROPERTY(QStringList messages READ getMessages WRITE setMessages)
    Q_PROPERTY(int messageInterval READ getMessageInterval WRITE setMessageInterval)
    Q_PROPERTY(QString doneText READ getDoneText WRITE setDoneText)
    Q_PROPERTY(bool showPercent READ isShowPercent WRITE setShowPercent)

public:
    /** Confetti (kind 0) or sweat drop (kind 1). */
    struct Particle
    {
        qreal x = 0.0;
        qreal y = 0.0;
        qreal vx = 0.0;
        qreal vy = 0.0;
        qreal rot = 0.0;
        qreal vrot = 0.0;
        QColor color = QColor(QStringLiteral("#7FD6FF"));
        qreal life = 1.0;
        int kind = 0;
    };

    static QStringList defaultMessages();

    struct Options
    {
        qreal progress = 0.0;  ///< 0..1, negative = indeterminate.
        qreal speed = 1.0;
        QColor hamsterColor = QColor(QStringLiteral("#F4A261"));
        QColor wheelColor = QColor(QStringLiteral("#8D99AE"));
        QColor accentColor = QColor(QStringLiteral("#5E60CE"));
        QStringList messages = HamsterLoaderWidget::defaultMessages();
        int messageInterval = 2200;
        QString doneText = QStringLiteral("Done! The hamster deserves a raise.");
        bool showPercent = true;
        std::function<void()> finished;
    };

    explicit HamsterLoaderWidget(QWidget *parent = nullptr);
    explicit HamsterLoaderWidget(const Options &options, QWidget *parent = nullptr);

    qreal getProgress() const;
    qreal getSpeed() const;
    QColor getHamsterColor() const;
    QColor getWheelColor() const;
    QColor getAccentColor() const;
    QStringList getMessages() const;
    int getMessageInterval() const;
    QString getDoneText() const;
    bool isShowPercent() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void reset();
    void setProgress(qreal progress);
    void setSpeed(qreal speed);
    void setHamsterColor(const QColor &color);
    void setWheelColor(const QColor &color);
    void setAccentColor(const QColor &color);
    void setMessages(const QStringList &messages);
    void setMessageInterval(int messageInterval);
    void setDoneText(const QString &doneText);
    void setShowPercent(bool showPercent);

signals:
    void finished();
    void progressChanged(qreal progress);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    bool isDone() const;
    void geometry(QPointF &c, qreal &R, QRectF &bar) const;
    void drawHamster(QPainter &p, const QPointF &c, qreal R, qreal now);

    // Options.
    qreal m_progress;
    qreal m_speed;
    QColor m_hamsterColor;
    QColor m_wheelColor;
    QColor m_accentColor;
    QStringList m_messages;
    int m_messageInterval;
    QString m_doneText;
    bool m_showPercent;
    // State.
    qreal m_wheelAngle = 0.0;
    qreal m_runPhase = 0.0;
    qreal m_runSpeed = 0.0;
    qreal m_shownProgress;
    qreal m_doneAt = -1.0;
    QList<Particle> m_particles;
    qreal m_sweatAcc = 0.0;
    qreal m_last = 0.0;
    QPointF m_hamsterPos = QPointF(0, 0);
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_HAMSTERLOADER_H
