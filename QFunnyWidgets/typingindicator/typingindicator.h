/**
 * TypingIndicatorWidget: a chat "someone is typing" bubble whose three dots have faces and show what the other
 * person is really doing: typing (hopping dots), thinking (floating dots and a "?"), deleting (angry dots that
 * vanish right-to-left) or sleeping (squashed snoring dots). Optionally cycles through the states like a human.
 */
#ifndef QFUNNYWIDGETS_TYPINGINDICATOR_H
#define QFUNNYWIDGETS_TYPINGINDICATOR_H

#include <QColor>
#include <QElapsedTimer>
#include <QPointF>
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
 * Honest typing indicator.
 */
class QFUNNYWIDGETS_EXPORT TypingIndicatorWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(States state READ getState WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(QColor bubbleColor READ getBubbleColor WRITE setBubbleColor)
    Q_PROPERTY(QColor dotColor READ getDotColor WRITE setDotColor)
    Q_PROPERTY(bool autoCycle READ isAutoCycle WRITE setAutoCycle)

public:
    enum States
    {
        Typing = 0,
        Thinking = 1,
        Deleting = 2,
        Sleeping = 3
    };
    Q_ENUM(States)

    /** Label suffix per state ("is typing..." etc.). */
    static QString label(int state);

    struct Options
    {
        int state = TypingIndicatorWidget::Typing;
        QString name = QStringLiteral("Alex");
        QColor bubbleColor = QColor(QStringLiteral("#E9EAF0"));
        QColor dotColor = QColor(QStringLiteral("#7B7FFF"));
        bool autoCycle = false;
        std::function<void(int)> stateChanged;
    };

    explicit TypingIndicatorWidget(QWidget *parent = nullptr);
    explicit TypingIndicatorWidget(const Options &options, QWidget *parent = nullptr);

    States getState() const;
    QString getName() const;
    QColor getBubbleColor() const;
    QColor getDotColor() const;
    bool isAutoCycle() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setState(int state);
    void setName(const QString &name);
    void setBubbleColor(const QColor &color);
    void setDotColor(const QColor &color);
    void setAutoCycle(bool autoCycle);

signals:
    void stateChanged(int state);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void drawDot(QPainter &p, const QPointF &c, qreal r, qreal sx, qreal sy, int i, States st, qreal now);

    // Options.
    States m_state;
    QString m_name;
    QColor m_bubbleColor;
    QColor m_dotColor;
    bool m_autoCycle;
    // State.
    qreal m_stateAt = 0.0;
    qreal m_nextCycle = 3.0;
    qreal m_blend = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_TYPINGINDICATOR_H
