/**
 * EmptyStateWidget: a "nothing here yet" placeholder: a tumbleweed rolls through a desert past a bored, chirping
 * cactus, above a title, a subtitle and an optional call-to-action button.
 */
#ifndef QFUNNYWIDGETS_EMPTYSTATE_H
#define QFUNNYWIDGETS_EMPTYSTATE_H

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

/**
 * Empty-state placeholder with a rolling tumbleweed.
 */
class QFUNNYWIDGETS_EXPORT EmptyStateWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QString subtitle READ getSubtitle WRITE setSubtitle)
    Q_PROPERTY(QString actionText READ getActionText WRITE setActionText)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)
    Q_PROPERTY(QColor sandColor READ getSandColor WRITE setSandColor)
    Q_PROPERTY(qreal speed READ getSpeed WRITE setSpeed)

public:
    /** One loop of the tumbleweed: offset x/y, radii x/y (all relative), angle. */
    struct Loop
    {
        qreal ox;
        qreal oy;
        qreal rx;
        qreal ry;
        qreal ang;
    };

    struct Options
    {
        QString title = QStringLiteral("Nothing here yet");
        QString subtitle = QStringLiteral("Just us tumbleweeds. Add something to get started.");
        QString actionText = QStringLiteral("Add item");  ///< Empty = no button.
        QColor accentColor = QColor(QStringLiteral("#5E60CE"));
        QColor sandColor = QColor(QStringLiteral("#E9C46A"));
        qreal speed = 1.0;
        std::function<void()> actionClicked;
    };

    explicit EmptyStateWidget(QWidget *parent = nullptr);
    explicit EmptyStateWidget(const Options &options, QWidget *parent = nullptr);

    QString getTitle() const;
    QString getSubtitle() const;
    QString getActionText() const;
    QColor getAccentColor() const;
    QColor getSandColor() const;
    qreal getSpeed() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setTitle(const QString &title);
    void setSubtitle(const QString &subtitle);
    void setActionText(const QString &actionText);
    void setAccentColor(const QColor &color);
    void setSandColor(const QColor &color);
    void setSpeed(qreal speed);

signals:
    void actionClicked();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private:
    qreal now() const;
    void drawScene(QPainter &p, qreal w, qreal h, qreal ground, qreal unit, qreal now);
    void drawText(QPainter &p, qreal w, qreal ground, qreal unit);

    // Options.
    QString m_title;
    QString m_subtitle;
    QString m_actionText;
    QColor m_accentColor;
    QColor m_sandColor;
    qreal m_speed;
    // State.
    QList<Loop> m_loops;
    QRectF m_btnRect;
    bool m_btnHover = false;
    bool m_btnPress = false;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_EMPTYSTATE_H
