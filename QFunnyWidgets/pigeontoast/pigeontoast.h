/**
 * PigeonToast: a toast notification overlay delivered by a carrier pigeon. Attach it to a parent widget; it covers
 * the parent (following its size), and for each message a pigeon flies in carrying a swinging banner, hovers
 * while the message is shown, drops a few feathers and flies away. Messages are queued.
 */
#ifndef QFUNNYWIDGETS_PIGEONTOAST_H
#define QFUNNYWIDGETS_PIGEONTOAST_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPair>
#include <QPointF>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Carrier pigeon toast overlay.
 */
class PigeonToast : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int duration READ getDuration WRITE setDuration)
    Q_PROPERTY(QColor bannerColor READ getBannerColor WRITE setBannerColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)
    Q_PROPERTY(QColor pigeonColor READ getPigeonColor WRITE setPigeonColor)
    Q_PROPERTY(QString caption READ getCaption WRITE setCaption)
    Q_PROPERTY(int margin READ getMargin WRITE setMargin)

public:
    static constexpr qreal FLY_IN = 1.1;
    static constexpr qreal FLY_OUT = 1.0;

    /** A falling feather. */
    struct Feather
    {
        Feather(qreal x = 0.0, qreal y = 0.0, qreal life = 2.0, qreal phase = 0.0)
            : x(x), y(y), life(life), maxLife(life), phase(phase) {}
        qreal x;
        qreal y;
        qreal life;
        qreal maxLife;
        qreal phase;
    };

    struct Options
    {
        int duration = 2600;  ///< Default ms a message stays.
        QColor bannerColor = QColor(QStringLiteral("#FFFFFF"));
        QColor textColor = QColor(QStringLiteral("#22222A"));
        QColor pigeonColor = QColor(QStringLiteral("#9AA5B1"));
        QString caption = QStringLiteral("Delivered by pigeon");
        int margin = 24;
        std::function<void(const QString &)> messageShown;
        std::function<void(const QString &)> messageFinished;
    };

    explicit PigeonToast(QWidget *parent = nullptr);
    PigeonToast(QWidget *parent, const Options &options);
    explicit PigeonToast(const Options &options, QWidget *parent = nullptr);

    int getPending() const;
    int getDuration() const;
    QColor getBannerColor() const;
    QColor getTextColor() const;
    QColor getPigeonColor() const;
    QString getCaption() const;
    int getMargin() const;

public slots:
    /** Queue a message; duration < 0 uses the default duration. */
    void showMessage(const QString &text, int duration = -1);
    void dismiss();
    void setDuration(int duration);
    void setBannerColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setPigeonColor(const QColor &color);
    void setCaption(const QString &caption);
    void setMargin(int margin);

signals:
    void messageShown(const QString &text);
    void messageFinished(const QString &text);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    enum Phase { In, Hold, Out, Done };

    qreal now() const;
    void bannerSize(const QString &text, qreal &bw, qreal &bh, qreal &mh) const;
    void next();
    QPointF path(qreal now, Phase &phase) const;
    void drawBanner(QPainter &p, const QPointF &pos, const QString &text);
    void drawPigeon(QPainter &p, const QPointF &pos, qreal s, qreal flap, qreal vx);
    static void drawWing(QPainter &p, qreal s, qreal flap, bool farWing);
    void drawFeather(QPainter &p);

    // Options.
    int m_duration;
    QColor m_bannerColor;
    QColor m_textColor;
    QColor m_pigeonColor;
    QString m_caption;
    int m_margin;
    // State.
    QList<QPair<QString, int>> m_queue;
    bool m_hasCurrent = false;
    QPair<QString, int> m_current;
    qreal m_start = 0.0;
    qreal m_holdEnd = 0.0;
    QPointF m_pos = QPointF(-999, -999);
    qreal m_vx = 0.0;
    qreal m_swing = 0.0;
    qreal m_swingVel = 0.0;
    QList<Feather> m_feathers;
    bool m_dropped = false;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_PIGEONTOAST_H
