/**
 * NervousHoldButtonWidget: a hold-to-confirm button for destructive actions. While held it fills up, its face
 * panics more and more (wider eyes, shaking, sweat, screaming) and its label escalates; at 100 % it faints.
 * Releasing early shows relief.
 */
#ifndef QFUNNYWIDGETS_HOLDBUTTON_H
#define QFUNNYWIDGETS_HOLDBUTTON_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QRectF>
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
class QTimer;

/**
 * Hold-to-confirm button that panics, then faints.
 */
class QFUNNYWIDGETS_EXPORT NervousHoldButtonWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList stages READ getStages WRITE setStages)
    Q_PROPERTY(QString doneText READ getDoneText WRITE setDoneText)
    Q_PROPERTY(QString reliefText READ getReliefText WRITE setReliefText)
    Q_PROPERTY(int holdDuration READ getHoldDuration WRITE setHoldDuration)
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)
    Q_PROPERTY(int autoReset READ getAutoReset WRITE setAutoReset)

public:
    /** One sweat drop. */
    struct Drop
    {
        qreal x = 0.0;
        qreal y = 0.0;
        qreal vx = 0.0;
        qreal vy = 0.0;
        qreal life = 0.6;
    };

    static QStringList defaultStages();

    struct Options
    {
        QStringList stages = NervousHoldButtonWidget::defaultStages();
        QString doneText = QStringLiteral("Deleted. RIP.");
        QString reliefText = QStringLiteral("Phew!");
        int holdDuration = 1800;  ///< ms to hold.
        QColor color = QColor(QStringLiteral("#E5383B"));
        QColor textColor = QColor(QStringLiteral("#FFFFFF"));
        int autoReset = 2500;     ///< ms after fainting before it resets (0 = never).
        std::function<void()> confirmed;
        std::function<void(qreal)> cancelled;
    };

    explicit NervousHoldButtonWidget(QWidget *parent = nullptr);
    explicit NervousHoldButtonWidget(const Options &options, QWidget *parent = nullptr);

    qreal getProgress() const;
    QStringList getStages() const;
    QString getDoneText() const;
    QString getReliefText() const;
    int getHoldDuration() const;
    QColor getColor() const;
    QColor getTextColor() const;
    int getAutoReset() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void reset();
    void setStages(const QStringList &stages);
    void setDoneText(const QString &doneText);
    void setReliefText(const QString &reliefText);
    void setHoldDuration(int holdDuration);
    void setColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setAutoReset(int autoReset);

signals:
    void confirmed();
    void cancelled(qreal progress);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *e) override;
#else
    void enterEvent(QEvent *e) override;
#endif
    void leaveEvent(QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    bool isFainted() const;
    QRectF buttonRect() const;
    QString label(qreal now) const;
    void press();
    void release();
    void drawFace(QPainter &p, const QRectF &rect, qreal prog, bool fainted, qreal now);

    // Options.
    QStringList m_stages;
    QString m_doneText;
    QString m_reliefText;
    int m_holdDuration;
    QColor m_color;
    QColor m_textColor;
    int m_autoReset;
    // State.
    bool m_holding = false;
    qreal m_progress = 0.0;
    qreal m_faintAt = -1.0;
    qreal m_reliefUntil = 0.0;
    qreal m_hover = 0.0;
    bool m_hovered = false;
    qreal m_squash = 0.0;
    qreal m_squashVel = 0.0;
    QList<Drop> m_drops;
    qreal m_dropAcc = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_HOLDBUTTON_H
