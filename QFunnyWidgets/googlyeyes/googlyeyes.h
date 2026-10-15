/**
 * GooglyEyesWidget: googly eyes whose pupils follow the cursor with spring physics and gravity, bounce off the
 * eye wall, blink on their own, get dizzy when poked and angry when poked too often.
 */
#ifndef QFUNNYWIDGETS_GOOGLYEYES_H
#define QFUNNYWIDGETS_GOOGLYEYES_H

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
 * Googly eyes with real(ish) physics.
 */
class GooglyEyesWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int eyeCount READ getEyeCount WRITE setEyeCount)
    Q_PROPERTY(QColor eyeColor READ getEyeColor WRITE setEyeColor)
    Q_PROPERTY(QColor pupilColor READ getPupilColor WRITE setPupilColor)
    Q_PROPERTY(QColor lidColor READ getLidColor WRITE setLidColor)
    Q_PROPERTY(qreal wobble READ getWobble WRITE setWobble)
    Q_PROPERTY(qreal gravity READ getGravity WRITE setGravity)
    Q_PROPERTY(int blinkInterval READ getBlinkInterval WRITE setBlinkInterval)
    Q_PROPERTY(bool autoBlink READ isAutoBlink WRITE setAutoBlink)

public:
    /** Physics state of one pupil (offset from the eye centre and velocity). */
    struct PupilState
    {
        PupilState(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0) : x(x), y(y), vx(vx), vy(vy) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
    };

    static const int DIZZY_MS = 1600;
    static const int BLINK_MS = 160;
    static const int ANGRY_POKES = 3;

    struct Options
    {
        int eyeCount = 2;                                  ///< 1..6
        QColor eyeColor = QColor(QStringLiteral("#FFFFFF"));
        QColor pupilColor = QColor(QStringLiteral("#17171C"));
        QColor lidColor = QColor(QStringLiteral("#F2B8A0"));
        qreal wobble = 0.6;                                ///< 0 = stiff, 1 = very wobbly.
        qreal gravity = 900.0;                             ///< Pull on the pupils (px/s^2).
        int blinkInterval = 3500;                          ///< Mean ms between automatic blinks.
        bool autoBlink = true;
        std::function<void(int)> poked;
        std::function<void()> gotAngry;
    };

    explicit GooglyEyesWidget(QWidget *parent = nullptr);
    explicit GooglyEyesWidget(const Options &options, QWidget *parent = nullptr);

    int getPokeCount() const;
    int getEyeCount() const;
    QColor getEyeColor() const;
    QColor getPupilColor() const;
    QColor getLidColor() const;
    qreal getWobble() const;
    qreal getGravity() const;
    int getBlinkInterval() const;
    bool isAutoBlink() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void blink();
    void makeDizzy();
    void setEyeCount(int eyeCount);
    void setEyeColor(const QColor &color);
    void setPupilColor(const QColor &color);
    void setLidColor(const QColor &color);
    void setWobble(qreal wobble);
    void setGravity(qreal gravity);
    void setBlinkInterval(int blinkInterval);
    void setAutoBlink(bool autoBlink);

signals:
    void poked(int count);
    void gotAngry();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    qreal scheduleBlink(qreal now) const;
    QList<QRectF> layoutEyes() const;
    qreal blinkAmount(qreal now);
    static void drawStar(QPainter &p, const QPointF &c, qreal r, const QColor &color);

    // Options.
    int m_eyeCount;
    QColor m_eyeColor;
    QColor m_pupilColor;
    QColor m_lidColor;
    qreal m_wobble;
    qreal m_gravity;
    int m_blinkInterval;
    bool m_autoBlink;
    // State.
    QList<PupilState> m_pupils;
    int m_pokeCount = 0;
    QList<qreal> m_pokeTimes;
    qreal m_angryUntil = 0.0;
    qreal m_dizzyUntil = 0.0;
    qreal m_blinkStart = -1.0;
    qreal m_nextBlink = 0.0;
    qreal m_squish = 0.0;
    qreal m_last = 0.0;
    QList<QRectF> m_eyeRects;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_GOOGLYEYES_H
