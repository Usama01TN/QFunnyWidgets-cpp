/**
 * PufferCounterWidget: a plain-text area with a character limit, watched by a pufferfish in its footer
 * (PufferFooterWidget). The fish inflates as you type, grows spikes near the limit, turns red and furious when
 * you exceed it, and sighs with relief ("pfff") when you get back under it.
 */
#ifndef QFUNNYWIDGETS_PUFFERCOUNTER_H
#define QFUNNYWIDGETS_PUFFERCOUNTER_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QWidget>
#include <functional>

class QPainter;
class QPlainTextEdit;
class QTimer;

/**
 * The counter footer with the pufferfish.
 */
class PufferFooterWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int count READ getCount WRITE setCount)
    Q_PROPERTY(int maxLength READ getMaxLength WRITE setMaxLength)
    Q_PROPERTY(QColor fishColor READ getFishColor WRITE setFishColor)

public:
    /** One rising air bubble. */
    struct Bubble
    {
        Bubble(qreal x = 0.0, qreal y = 0.0, qreal r = 2.0, qreal life = 1.2) : x(x), y(y), r(r), life(life), maxLife(life) {}
        qreal x;
        qreal y;
        qreal r;
        qreal life;
        qreal maxLife;
    };

    struct Options
    {
        int count = 0;
        int maxLength = 280;
        QColor fishColor = QColor(QStringLiteral("#FFD166"));
    };

    explicit PufferFooterWidget(QWidget *parent = nullptr);
    explicit PufferFooterWidget(const Options &options, QWidget *parent = nullptr);

    int getCount() const;
    int getMaxLength() const;
    QColor getFishColor() const;

    QSize sizeHint() const override;

public slots:
    void setCount(int count);
    void setMaxLength(int maxLength);
    void setFishColor(const QColor &color);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    qreal ratio() const;
    void drawFish(QPainter &p, const QPointF &c, qreal now);

    // Options.
    int m_count;
    int m_maxLength;
    QColor m_fishColor;
    // State.
    qreal m_puff; // animated inflation
    qreal m_puffVel = 0.0;
    qreal m_deflateAt = -10.0;
    QList<Bubble> m_bubbles;
    qreal m_bubbleAcc = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

/**
 * Text area + pufferfish footer.
 */
class PufferCounterWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString placeholder READ getPlaceholder WRITE setPlaceholder)
    Q_PROPERTY(int maxLength READ getMaxLength WRITE setMaxLength)
    Q_PROPERTY(bool enforceLimit READ isEnforceLimit WRITE setEnforceLimit)

public:
    struct Options
    {
        QString text;
        QString placeholder = QStringLiteral("What's happening?");
        int maxLength = 280;
        bool enforceLimit = false;  ///< Truncate instead of letting the fish get angry.
        QColor fishColor = QColor(QStringLiteral("#FFD166"));
        std::function<void(const QString &)> textChanged;
        std::function<void(bool)> limitExceeded;
    };

    explicit PufferCounterWidget(QWidget *parent = nullptr);
    explicit PufferCounterWidget(const Options &options, QWidget *parent = nullptr);

    QPlainTextEdit *getEditor() const;
    PufferFooterWidget *getFooter() const;
    bool isOverLimit() const;
    QString getText() const;
    QString getPlaceholder() const;
    int getMaxLength() const;
    bool isEnforceLimit() const;

public slots:
    void setText(const QString &text);
    void setPlaceholder(const QString &placeholder);
    void setMaxLength(int maxLength);
    void setEnforceLimit(bool enforceLimit);

signals:
    void textChanged(const QString &text);
    void limitExceeded(bool over);

private slots:
    void onText();

private:
    bool m_enforce;
    bool m_over = false;
    QPlainTextEdit *m_edit;
    PufferFooterWidget *m_footer;
};

#endif // QFUNNYWIDGETS_PUFFERCOUNTER_H
