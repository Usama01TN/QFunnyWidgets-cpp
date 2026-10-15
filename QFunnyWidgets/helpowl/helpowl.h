/**
 * HelpOwlWidget: a tiny "?" help icon shaped like a sleeping owl. Hover it (or click/press Space to pin it) and the
 * owl wakes up, swivels its head and shows a floating help bubble (HelpBubble); it dozes off again when you leave.
 */
#ifndef QFUNNYWIDGETS_HELPOWL_H
#define QFUNNYWIDGETS_HELPOWL_H

#include <QColor>
#include <QElapsedTimer>
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

class QTimer;

/**
 * The floating help bubble (a frameless tool-tip window with a tail).
 */
class QFUNNYWIDGETS_EXPORT HelpBubble : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QString text READ getText WRITE setText)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)
    Q_PROPERTY(qreal opacity READ getOpacity WRITE setOpacity)
    Q_PROPERTY(bool tailRight READ isTailRight WRITE setTailRight)

public:
    struct Options
    {
        QString title;
        QString text;
        QColor accentColor = QColor(QStringLiteral("#8D6E63"));
    };

    explicit HelpBubble(QWidget *parent = nullptr);
    HelpBubble(QWidget *parent, const Options &options);
    explicit HelpBubble(const Options &options, QWidget *parent = nullptr);

    QString getTitle() const;
    QString getText() const;
    QColor getAccentColor() const;
    bool isTailRight() const;
    qreal getOpacity() const;

public slots:
    void setTitle(const QString &title);
    void setText(const QString &text);
    void setAccentColor(const QColor &color);
    void setTailRight(bool tailRight);
    void setOpacity(qreal opacity);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void fonts(QFont &tf, QFont &bf) const;
    void resizeToText();

    QString m_title;
    QString m_text;
    QColor m_accentColor;
    qreal m_opacity = 0.0;
    bool m_tailRight = false;
};

/**
 * The owl help icon.
 */
class QFUNNYWIDGETS_EXPORT HelpOwlWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString helpText READ getHelpText WRITE setHelpText)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QColor owlColor READ getOwlColor WRITE setOwlColor)

public:
    static constexpr qreal WAKE_TIME = 0.6;
    static constexpr qreal DOZE_TIME = 0.5;

    struct Options
    {
        QString helpText = QStringLiteral("Hoo! This is where the helpful explanation goes.");
        QString title;
        QColor owlColor = QColor(QStringLiteral("#8D6E63"));
        std::function<void()> shown;
        std::function<void()> hidden;
    };

    explicit HelpOwlWidget(QWidget *parent = nullptr);
    explicit HelpOwlWidget(const Options &options, QWidget *parent = nullptr);

    bool isHelpVisible() const;
    HelpBubble *getBubble() const;
    QString getHelpText() const;
    QString getTitle() const;
    QColor getOwlColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void showHelp();
    void hideHelp();
    void setHelpText(const QString &helpText);
    void setTitle(const QString &title);
    void setOwlColor(const QColor &color);

signals:
    void shown();
    void hidden();

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
    void focusOutEvent(QFocusEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    enum State { SLEEP, WAKING, AWAKE, DOZING };

    qreal now() const;
    void setState(State state);
    bool wantOpen() const;
    void placeBubble();

    // Options.
    QColor m_owlColor;
    // Bubble window.
    HelpBubble *m_bubble;
    // State.
    State m_state = SLEEP;
    qreal m_stateAt = -10.0;
    bool m_hovered = false;
    bool m_pinned = false;
    bool m_visible = false;
    qreal m_alpha = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_HELPOWL_H
