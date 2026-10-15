/**
 * RobotCheckWidget: an "I'm not a robot" checkbox panel with a nervous robot hiding behind it. The robot peeks out
 * now and then, sweats oil and calculates while you are being checked, winks with a thumbs-up when you pass and
 * is delighted ("One of us!") when you fail.
 */
#ifndef QFUNNYWIDGETS_ROBOTCHECK_H
#define QFUNNYWIDGETS_ROBOTCHECK_H

#include <QColor>
#include <QElapsedTimer>
#include <QPointF>
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

/**
 * Human verification administered by a robot.
 */
class QFUNNYWIDGETS_EXPORT RobotCheckWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText)
    Q_PROPERTY(QString verifiedText READ getVerifiedText WRITE setVerifiedText)
    Q_PROPERTY(qreal failChance READ getFailChance WRITE setFailChance)
    Q_PROPERTY(QColor panelColor READ getPanelColor WRITE setPanelColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)
    Q_PROPERTY(QColor robotColor READ getRobotColor WRITE setRobotColor)

public:
    static constexpr qreal CHECK_TIME = 1.8;

    struct Options
    {
        QString text = QStringLiteral("I'm not a robot");
        QString verifiedText = QStringLiteral("Verified human (probably)");
        qreal failChance = 0.0;  ///< 0..1
        QColor panelColor = QColor(QStringLiteral("#F9F9FB"));
        QColor textColor = QColor(QStringLiteral("#23232B"));
        QColor robotColor = QColor(QStringLiteral("#AAB4C3"));
        std::function<void()> verified;
        std::function<void()> failed;
    };

    explicit RobotCheckWidget(QWidget *parent = nullptr);
    explicit RobotCheckWidget(const Options &options, QWidget *parent = nullptr);

    bool isVerified() const;
    QString getText() const;
    QString getVerifiedText() const;
    qreal getFailChance() const;
    QColor getPanelColor() const;
    QColor getTextColor() const;
    QColor getRobotColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void verify();
    void reset();
    void setText(const QString &text);
    void setVerifiedText(const QString &verifiedText);
    void setFailChance(qreal failChance);
    void setPanelColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setRobotColor(const QColor &color);

signals:
    void verified();
    void failed();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    enum Mode { IDLE, CHECKING, VERIFIED, FAILED };

    qreal now() const;
    void setMode(Mode mode);
    QRectF panel() const;
    void drawRobot(QPainter &p, const QPointF &c, qreal s, Mode mode, qreal since, qreal now);

    // Options.
    QString m_text;
    QString m_verifiedText;
    qreal m_failChance;
    QColor m_panelColor;
    QColor m_textColor;
    QColor m_robotColor;
    // State.
    Mode m_mode = IDLE;
    qreal m_modeAt = 0.0;
    qreal m_rise = 0.0;
    qreal m_riseVel = 0.0;
    QRectF m_boxRect;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_ROBOTCHECK_H
