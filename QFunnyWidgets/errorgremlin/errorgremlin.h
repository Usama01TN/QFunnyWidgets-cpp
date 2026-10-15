/**
 * ErrorGremlinWidget: an inline alert banner with a severity-matched gremlin: sleeping for info, peeking and waving
 * a wrench for warnings, chewing a sparking cable for errors and bouncing in flames for critical alerts. Retry
 * scares it off; the close button fades the banner out.
 */
#ifndef QFUNNYWIDGETS_ERRORGREMLIN_H
#define QFUNNYWIDGETS_ERRORGREMLIN_H

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
class QTimer;

/**
 * Alert banner with a gremlin.
 */
class QFUNNYWIDGETS_EXPORT ErrorGremlinWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Severity severity READ getSeverity WRITE setSeverity)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QString message READ getMessage WRITE setMessage)
    Q_PROPERTY(QString retryText READ getRetryText WRITE setRetryText)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)
    Q_PROPERTY(QColor panelColor READ getPanelColor WRITE setPanelColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)

public:
    enum Severity
    {
        Info = 0,
        Warning = 1,
        Error = 2,
        Critical = 3
    };
    Q_ENUM(Severity)

    /** Electric spark (kind 0) or flame (kind 1). */
    struct Spark
    {
        Spark(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 0.4, int kind = 0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life), kind(kind) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
        int kind;
    };

    static QColor severityColor(int severity);

    struct Options
    {
        int severity = ErrorGremlinWidget::Error;
        QString title = QStringLiteral("Something went wrong");
        QString message = QStringLiteral("A gremlin got into the server room. Our team has been notified.");
        QString retryText = QStringLiteral("Retry");  ///< Empty = no retry button.
        bool closable = true;
        QColor panelColor = QColor(QStringLiteral("#FFFFFF"));
        QColor textColor = QColor(QStringLiteral("#23232B"));
        std::function<void()> retryClicked;
        std::function<void()> dismissed;
    };

    explicit ErrorGremlinWidget(QWidget *parent = nullptr);
    explicit ErrorGremlinWidget(const Options &options, QWidget *parent = nullptr);

    Severity getSeverity() const;
    QString getTitle() const;
    QString getMessage() const;
    QString getRetryText() const;
    bool isClosable() const;
    QColor getPanelColor() const;
    QColor getTextColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    /** (Re)show the banner; a negative severity or empty texts keep the current ones. */
    void showAlert(int severity = -1, const QString &title = QString(), const QString &message = QString());
    void dismiss();
    void setSeverity(int severity);
    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setRetryText(const QString &retryText);
    void setClosable(bool closable);
    void setPanelColor(const QColor &color);
    void setTextColor(const QColor &color);

signals:
    void retryClicked();
    void dismissed();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QRectF iconRect() const;
    qreal fleeing() const;
    void drawScene(QPainter &p, const QRectF &icon, int sev, qreal now);
    void drawGremlin(QPainter &p, const QPointF &c, qreal g, int sev, qreal flee, qreal now);

    // Options.
    Severity m_severity;
    QString m_title;
    QString m_message;
    QString m_retryText;
    bool m_closable;
    QColor m_panelColor;
    QColor m_textColor;
    // State.
    qreal m_retryAt = -10.0;
    qreal m_closeAt = -1.0;
    qreal m_changedAt = 0.0;
    QRectF m_retryRect;
    QRectF m_closeRect;
    int m_hoverBtn = 0;
    QList<Spark> m_sparks;
    qreal m_sparkAcc = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_ERRORGREMLIN_H
