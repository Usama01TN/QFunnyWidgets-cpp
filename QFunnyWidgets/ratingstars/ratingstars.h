/**
 * StarRatingWidget: a star rating where every star has a face. Stars bounce when hovered, do a wave when the
 * rating changes, cry for a 1-star rating, dance and sparkle for a perfect score, and empty stars look sad.
 */
#ifndef QFUNNYWIDGETS_RATINGSTARS_H
#define QFUNNYWIDGETS_RATINGSTARS_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
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
class QPainterPath;
class QTimer;

/**
 * Star rating with feelings.
 */
class QFUNNYWIDGETS_EXPORT StarRatingWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int rating READ getRating WRITE setRating NOTIFY ratingChanged)
    Q_PROPERTY(int maxRating READ getMaxRating WRITE setMaxRating)
    Q_PROPERTY(QColor starColor READ getStarColor WRITE setStarColor)
    Q_PROPERTY(QColor emptyColor READ getEmptyColor WRITE setEmptyColor)
    Q_PROPERTY(QStringList labels READ getLabels WRITE setLabels)
    Q_PROPERTY(QString placeholder READ getPlaceholder WRITE setPlaceholder)
    Q_PROPERTY(bool showLabel READ isShowLabel WRITE setShowLabel)
    Q_PROPERTY(bool allowClear READ isAllowClear WRITE setAllowClear)

public:
    /** Bounce state of one star. */
    struct StarState
    {
        qreal scale = 1.0;
        qreal vel = 0.0;
        qreal kickAt = -1.0;
    };

    /** Tear (kind 0) or sparkle (kind 1). */
    struct Particle
    {
        Particle(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 1.0, int kind = 0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life), kind(kind) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
        int kind;
    };

    static QStringList defaultLabels();
    /** Star outline with the given number of points and inner radius ratio. */
    static QPainterPath starPath(const QPointF &c, qreal r, qreal inner = 0.5, int points = 5);

    struct Options
    {
        int rating = 0;
        int maxRating = 5;  ///< 3..10
        QColor starColor = QColor(QStringLiteral("#FFC93C"));
        QColor emptyColor = QColor(QStringLiteral("#C9CCD6"));
        QStringList labels = StarRatingWidget::defaultLabels();
        QString placeholder = QStringLiteral("Rate us (we have feelings)");
        bool showLabel = true;
        bool allowClear = true;
        std::function<void(int)> ratingChanged;
        std::function<void(int)> hovered;
    };

    explicit StarRatingWidget(QWidget *parent = nullptr);
    explicit StarRatingWidget(const Options &options, QWidget *parent = nullptr);

    int getRating() const;
    int getMaxRating() const;
    QColor getStarColor() const;
    QColor getEmptyColor() const;
    QStringList getLabels() const;
    QString getPlaceholder() const;
    bool isShowLabel() const;
    bool isAllowClear() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setRating(int rating);
    void setMaxRating(int maxRating);
    void setStarColor(const QColor &color);
    void setEmptyColor(const QColor &color);
    void setLabels(const QStringList &labels);
    void setPlaceholder(const QString &placeholder);
    void setShowLabel(bool showLabel);
    void setAllowClear(bool allowClear);

signals:
    void ratingChanged(int rating);
    void hovered(int index);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    int shown() const;
    QList<QPointF> layoutStars(qreal &r) const;
    int indexAt(const QPointF &pos) const;
    QString labelFor(int value) const;
    void wave();
    void drawStar(QPainter &p, qreal r, int i, bool filled, int shown, qreal now);

    // Options.
    int m_maxRating;
    int m_rating;
    QColor m_starColor;
    QColor m_emptyColor;
    QStringList m_labels;
    QString m_placeholder;
    bool m_showLabel;
    bool m_allowClear;
    // State.
    int m_hover = 0;
    QList<StarState> m_stars;
    QList<Particle> m_particles;
    qreal m_spawn = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_RATINGSTARS_H
