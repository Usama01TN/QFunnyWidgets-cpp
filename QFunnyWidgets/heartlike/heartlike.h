/**
 * HeartLikeButtonWidget: a like button whose heart has feelings. Liking it pops it with a burst ring and mini
 * hearts, then it beats happily; un-liking cracks it in two (with tears) and leaves a bandage for a while;
 * toggling too fast makes it dizzy. The count rolls like an odometer.
 */
#ifndef QFUNNYWIDGETS_HEARTLIKE_H
#define QFUNNYWIDGETS_HEARTLIKE_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
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
class QPainterPath;
class QTimer;

/**
 * Like button that pops when liked and cracks when un-liked.
 */
class QFUNNYWIDGETS_EXPORT HeartLikeButtonWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool liked READ isLiked WRITE setLiked NOTIFY likedChanged)
    Q_PROPERTY(int count READ getCount WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(bool showCount READ isShowCount WRITE setShowCount)

public:
    /** A small flying heart. */
    struct MiniHeart
    {
        MiniHeart(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 0.8, qreal size = 5.0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life), size(size) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
        qreal size;
    };

    /** Heart outline centred at c with size s. */
    static QPainterPath heartPath(const QPointF &c, qreal s);

    struct Options
    {
        bool liked = false;
        int count = 41;
        QColor color = QColor(QStringLiteral("#FF4D6D"));
        bool showCount = true;
        std::function<void(bool)> likedChanged;
        std::function<void(int)> countChanged;
    };

    explicit HeartLikeButtonWidget(QWidget *parent = nullptr);
    explicit HeartLikeButtonWidget(const Options &options, QWidget *parent = nullptr);

    bool isLiked() const;
    int getCount() const;
    QColor getColor() const;
    bool isShowCount() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void toggle();
    void setLiked(bool liked);
    void setCount(int count);
    void setColor(const QColor &color);
    void setShowCount(bool showCount);

signals:
    void likedChanged(bool liked);
    void countChanged(int count);

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
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void heartGeom(QPointF &c, qreal &s) const;
    void changeCount(int count);
    void drawHeart(QPainter &p, const QPointF &c, qreal s, bool liked);
    void drawCracked(QPainter &p, const QPointF &c, qreal s, qreal since);
    void drawFace(QPainter &p, const QPointF &c, qreal s, bool liked, bool dizzy, bool cracking, qreal now);
    void drawCount(QPainter &p, const QRectF &area, qreal now);

    // Options.
    bool m_liked;
    int m_count;
    QColor m_color;
    bool m_showCount;
    // State.
    qreal m_pop = 1.0;
    qreal m_popVel = 0.0;
    qreal m_unlikedAt = -10.0;
    QList<qreal> m_toggles;
    qreal m_dizzyUntil = 0.0;
    QList<MiniHeart> m_minis;
    qreal m_ringAt = -10.0;
    int m_oldCount;
    qreal m_countAt = -10.0;
    bool m_hovered = false;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_HEARTLIKE_H
