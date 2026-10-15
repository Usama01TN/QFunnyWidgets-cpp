/**
 * RubberDuckWidget: a rubber-duck-debugging companion floating on animated water. Click it to squeak (beak opens,
 * eye squints) and receive dubious programming advice; tell it your problem with listen() and it nods while
 * thinking before answering.
 */
#ifndef QFUNNYWIDGETS_RUBBERDUCK_H
#define QFUNNYWIDGETS_RUBBERDUCK_H

#include <QColor>
#include <QElapsedTimer>
#include <QPointF>
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
 * Rubber duck debugging companion.
 */
class QFUNNYWIDGETS_EXPORT RubberDuckWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor duckColor READ getDuckColor WRITE setDuckColor)
    Q_PROPERTY(QColor beakColor READ getBeakColor WRITE setBeakColor)
    Q_PROPERTY(QColor waterColor READ getWaterColor WRITE setWaterColor)
    Q_PROPERTY(QStringList advice READ getAdvice WRITE setAdvice)
    Q_PROPERTY(bool adviceOnClick READ isAdviceOnClick WRITE setAdviceOnClick)
    Q_PROPERTY(int adviceDuration READ getAdviceDuration WRITE setAdviceDuration)
    Q_PROPERTY(QString squeakText READ getSqueakText WRITE setSqueakText)

public:
    static QStringList defaultAdvice();

    struct Options
    {
        QColor duckColor = QColor(QStringLiteral("#FFD43B"));
        QColor beakColor = QColor(QStringLiteral("#FF8C1A"));
        QColor waterColor = QColor(QStringLiteral("#4CC9F0"));
        QStringList advice = RubberDuckWidget::defaultAdvice();
        bool adviceOnClick = true;
        int adviceDuration = 3500;
        QString squeakText = QStringLiteral("SQUEAK!");
        std::function<void(int)> squeaked;
        std::function<void(const QString &)> adviceGiven;
    };

    explicit RubberDuckWidget(QWidget *parent = nullptr);
    explicit RubberDuckWidget(const Options &options, QWidget *parent = nullptr);

    int getSqueaks() const;
    QColor getDuckColor() const;
    QColor getBeakColor() const;
    QColor getWaterColor() const;
    QStringList getAdvice() const;
    bool isAdviceOnClick() const;
    int getAdviceDuration() const;
    QString getSqueakText() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void squeak();
    void listen(const QString &problem = QString());
    void setDuckColor(const QColor &color);
    void setBeakColor(const QColor &color);
    void setWaterColor(const QColor &color);
    void setAdvice(const QStringList &advice);
    void setAdviceOnClick(bool adviceOnClick);
    void setAdviceDuration(int adviceDuration);
    void setSqueakText(const QString &squeakText);

signals:
    void squeaked(int count);
    void adviceGiven(const QString &advice);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void geometry(QPointF &c, qreal &s, qreal &water) const;
    QString pickAdvice();
    void say(const QString &text, qreal now);
    void drawDuck(QPainter &p, qreal s, qreal now, bool thinking);
    void drawWater(QPainter &p, qreal w, qreal h, qreal water, qreal s, qreal now);
    void drawBubble(QPainter &p, const QPointF &c, qreal s, qreal now, bool thinking);

    // Options.
    QColor m_duckColor;
    QColor m_beakColor;
    QColor m_waterColor;
    QStringList m_advice;
    bool m_adviceOnClick;
    int m_adviceDuration;
    QString m_squeakText;
    // State.
    int m_squeaks = 0;
    qreal m_squash = 0.0;
    qreal m_squashVel = 0.0;
    qreal m_squeakAt = -10.0;
    qreal m_thinkUntil = 0.0;
    QString m_bubbleText;
    qreal m_bubbleUntil = 0.0;
    qreal m_bubbleShownAt = -10.0;
    QString m_lastAdvice;
    qreal m_blinkAt = 2.0;
    qreal m_lookX = 0.0;
    qreal m_lookTarget = 0.0;
    qreal m_nextLook = 1.5;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_RUBBERDUCK_H
