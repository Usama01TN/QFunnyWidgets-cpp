/**
 * PresenceAvatarWidget: an initials avatar whose online status is its mood: online (pulsing ring, eyes follow the
 * cursor, waves when clicked), away (asleep with a floating "z"), busy (headphones, frowning at the work) or
 * offline (a translucent ghost). Hovering shows a greeting bubble.
 */
#ifndef QFUNNYWIDGETS_PRESENCEAVATAR_H
#define QFUNNYWIDGETS_PRESENCEAVATAR_H

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

/** Up to two initials of a name ("Ada Lovelace" -> "AL", "Plato" -> "PL", "" -> "?"). */
QFUNNYWIDGETS_EXPORT QString initialsOf(const QString &name);
/** A stable pastel colour derived from a name. */
QFUNNYWIDGETS_EXPORT QColor colorOf(const QString &name);

/**
 * Avatar with a presence mood.
 */
class QFUNNYWIDGETS_EXPORT PresenceAvatarWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(Status status READ getStatus WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(QColor badgeBorderColor READ getBadgeBorderColor WRITE setBadgeBorderColor)

public:
    enum Status
    {
        Online = 0,
        Away = 1,
        Busy = 2,
        Offline = 3
    };
    Q_ENUM(Status)

    static QColor statusColor(int status);
    static QString greeting(int status);

    struct Options
    {
        QString name = QStringLiteral("Ada Lovelace");
        int status = PresenceAvatarWidget::Online;
        QColor color;  ///< Invalid = derived from the name.
        QColor badgeBorderColor = QColor(QStringLiteral("#FFFFFF"));
        std::function<void()> clicked;
        std::function<void(int)> statusChanged;
    };

    explicit PresenceAvatarWidget(QWidget *parent = nullptr);
    explicit PresenceAvatarWidget(const Options &options, QWidget *parent = nullptr);

    QString getInitials() const;
    QString getName() const;
    Status getStatus() const;
    QColor getColor() const;
    QColor getBadgeBorderColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setName(const QString &name);
    void setStatus(int status);
    void setColor(const QColor &color);
    void setBadgeBorderColor(const QColor &color);

signals:
    void clicked();
    void statusChanged(int status);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
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

private:
    qreal now() const;
    void updateToolTip();
    QColor baseColor() const;
    void drawFace(QPainter &p, const QPointF &c, qreal R, int st, qreal now);
    static void drawHeadphones(QPainter &p, const QPointF &c, qreal R);
    void drawBadge(QPainter &p, const QPointF &c, qreal r, int st, const QColor &sc);
    void drawBubble(QPainter &p, const QPointF &anchor, const QString &text);

    // Options.
    QString m_name;
    Status m_status;
    QColor m_color;
    QColor m_badgeBorderColor;
    // State.
    qreal m_hover = 0.0;
    bool m_hovered = false;
    qreal m_clickAt = -10.0;
    qreal m_blinkAt;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    qreal m_phase;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_PRESENCEAVATAR_H
