/**
 * SleepyCatSectionWidget: a collapsible section whose header bar (CatHeaderWidget) has a cat napping on it. The
 * cat sleeps while the section is collapsed (peeking when you hover), jumps up startled when it is expanded and
 * then sits watching the content, and yawns before going back to sleep when it collapses.
 */
#ifndef QFUNNYWIDGETS_CATSECTION_H
#define QFUNNYWIDGETS_CATSECTION_H

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
class QVBoxLayout;

/**
 * The header bar with the cat.
 */
class QFUNNYWIDGETS_EXPORT CatHeaderWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QColor catColor READ getCatColor WRITE setCatColor)
    Q_PROPERTY(QColor barColor READ getBarColor WRITE setBarColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)

public:
    static constexpr qreal STARTLE_TIME = 0.6;
    static constexpr qreal YAWN_TIME = 1.0;
    static constexpr qreal CAT_ROOM = 42.0; ///< space above the bar for the cat
    static constexpr qreal BAR_HEIGHT = 40.0;

    struct Options
    {
        QString title = QStringLiteral("Section");
        bool expanded = false;
        QColor catColor = QColor(QStringLiteral("#F4A261"));
        QColor barColor = QColor(QStringLiteral("#ECEEF3"));
        QColor textColor = QColor(QStringLiteral("#23232B"));
        std::function<void()> clicked;
    };

    explicit CatHeaderWidget(QWidget *parent = nullptr);
    explicit CatHeaderWidget(const Options &options, QWidget *parent = nullptr);

    bool isExpanded() const;
    QString getTitle() const;
    QColor getCatColor() const;
    QColor getBarColor() const;
    QColor getTextColor() const;

    QSize sizeHint() const override;

public slots:
    void setExpanded(bool expanded);
    void setTitle(const QString &title);
    void setCatColor(const QColor &color);
    void setBarColor(const QColor &color);
    void setTextColor(const QColor &color);

signals:
    void clicked();

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
    enum CatState { SLEEPING, STARTLED, SITTING, YAWNING };

    qreal now() const;
    void drawCat(QPainter &p, const QPointF &base, qreal now);

    // Options.
    QString m_title;
    bool m_expanded;
    QColor m_catColor;
    QColor m_barColor;
    QColor m_textColor;
    // State.
    CatState m_cat;
    qreal m_catAt = -10.0;
    qreal m_chevron;
    bool m_hovered = false;
    qreal m_blinkAt = 2.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

/**
 * Collapsible section: a CatHeaderWidget plus an animated body holding the content widget.
 */
class QFUNNYWIDGETS_EXPORT SleepyCatSectionWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY toggled)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QWidget *contentWidget READ getContentWidget WRITE setContentWidget)

public:
    struct Options
    {
        QString title = QStringLiteral("Advanced settings");
        bool expanded = false;
        QWidget *content = nullptr;
        QColor catColor = QColor(QStringLiteral("#F4A261"));
        QColor barColor = QColor(QStringLiteral("#ECEEF3"));
        QColor textColor = QColor(QStringLiteral("#23232B"));
        std::function<void(bool)> toggled;
    };

    explicit SleepyCatSectionWidget(QWidget *parent = nullptr);
    explicit SleepyCatSectionWidget(const Options &options, QWidget *parent = nullptr);

    bool isExpanded() const;
    QWidget *getContentWidget() const;
    CatHeaderWidget *getHeader() const;
    QString getTitle() const;

public slots:
    void toggle();
    void setExpanded(bool expanded);
    /** Replace the content; the previous content widget is detached (the caller then owns it). */
    void setContentWidget(QWidget *content);
    void setTitle(const QString &title);

signals:
    void toggled(bool expanded);

private slots:
    void tick();

private:
    qreal targetHeight() const;
    void snap();

    CatHeaderWidget *m_header;
    QWidget *m_body;
    QVBoxLayout *m_bodyLayout;
    QWidget *m_content = nullptr;
    bool m_expanded;
    qreal m_height = 0.0;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_CATSECTION_H
