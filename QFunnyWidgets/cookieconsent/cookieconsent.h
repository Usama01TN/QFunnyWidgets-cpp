/**
 * CookieConsentWidget: a cookie banner starring an emotionally invested cookie. It looks at you with puppy eyes,
 * gets excited when you hover Accept, tears up when you hover Decline (which shyly shrinks), happily gets eaten
 * bite by bite when accepted and sobs when declined.
 */
#ifndef QFUNNYWIDGETS_COOKIECONSENT_H
#define QFUNNYWIDGETS_COOKIECONSENT_H

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
 * Cookie banner with feelings.
 */
class QFUNNYWIDGETS_EXPORT CookieConsentWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QString message READ getMessage WRITE setMessage)
    Q_PROPERTY(QString acceptText READ getAcceptText WRITE setAcceptText)
    Q_PROPERTY(QString declineText READ getDeclineText WRITE setDeclineText)
    Q_PROPERTY(QString acceptedMessage READ getAcceptedMessage WRITE setAcceptedMessage)
    Q_PROPERTY(QString declinedMessage READ getDeclinedMessage WRITE setDeclinedMessage)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)
    Q_PROPERTY(QColor panelColor READ getPanelColor WRITE setPanelColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)
    Q_PROPERTY(QColor cookieColor READ getCookieColor WRITE setCookieColor)
    Q_PROPERTY(int autoReset READ getAutoReset WRITE setAutoReset)

public:
    /** One cookie crumb. */
    struct Crumb
    {
        qreal x = 0.0;
        qreal y = 0.0;
        qreal vx = 0.0;
        qreal vy = 0.0;
        qreal size = 2.0;
        qreal life = 1.0;
    };

    struct Options
    {
        QString title = QStringLiteral("We use cookies");
        QString message = QStringLiteral("This cookie is hoping you'll accept it. No pressure. (Some pressure.)");
        QString acceptText = QStringLiteral("Accept");
        QString declineText = QStringLiteral("Decline");
        QString acceptedMessage = QStringLiteral("Nom. Excellent choice.");
        QString declinedMessage = QStringLiteral("Fine. The cookie will remember this.");
        QColor accentColor = QColor(QStringLiteral("#5E60CE"));
        QColor panelColor = QColor(QStringLiteral("#FFFFFF"));
        QColor textColor = QColor(QStringLiteral("#23232B"));
        QColor cookieColor = QColor(QStringLiteral("#D9A066"));
        int autoReset = 0;  ///< ms after a choice before asking again (0 = never).
        std::function<void()> accepted;
        std::function<void()> declined;
    };

    explicit CookieConsentWidget(QWidget *parent = nullptr);
    explicit CookieConsentWidget(const Options &options, QWidget *parent = nullptr);

    int getChoice() const; ///< 0 = asking, 1 = accepted, -1 = declined
    QString getTitle() const;
    QString getMessage() const;
    QString getAcceptText() const;
    QString getDeclineText() const;
    QString getAcceptedMessage() const;
    QString getDeclinedMessage() const;
    QColor getAccentColor() const;
    QColor getPanelColor() const;
    QColor getTextColor() const;
    QColor getCookieColor() const;
    int getAutoReset() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void accept();
    void decline();
    void reset();
    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setAcceptText(const QString &acceptText);
    void setDeclineText(const QString &declineText);
    void setAcceptedMessage(const QString &acceptedMessage);
    void setDeclinedMessage(const QString &declinedMessage);
    void setAccentColor(const QColor &color);
    void setPanelColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setCookieColor(const QColor &color);
    void setAutoReset(int autoReset);

signals:
    void accepted();
    void declined();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void choose(int choice);
    void drawButtons(QPainter &p, qreal x0, qreal y, qreal h, qreal now);
    void drawCookie(QPainter &p, const QPointF &c, qreal R, qreal now);

    // Options.
    QString m_title;
    QString m_message;
    QString m_acceptText;
    QString m_declineText;
    QString m_acceptedMessage;
    QString m_declinedMessage;
    QColor m_accentColor;
    QColor m_panelColor;
    QColor m_textColor;
    QColor m_cookieColor;
    int m_autoReset;
    // State.
    int m_choice = 0; // 0 = asking, 1 = accepted, -1 = declined
    qreal m_choiceAt = -10.0;
    int m_hoverBtn = 0; // 1 = accept, -1 = decline
    QRectF m_acceptRect;
    QRectF m_declineRect;
    QPointF m_cookieCentre;
    qreal m_cookieR = 10.0;
    int m_bitesDone = 0;
    QList<Crumb> m_crumbs;
    qreal m_blinkAt = 2.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_COOKIECONSENT_H
