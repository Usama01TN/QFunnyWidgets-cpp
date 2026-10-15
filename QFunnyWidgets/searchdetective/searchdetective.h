/**
 * SearchDetectiveWidget: a debounced search field investigated by a pipe-smoking detective (DetectiveFaceWidget).
 * He waits while idle, studies your typing through his magnifying glass, investigates during the search, beams
 * when something is found and gets a personal rain cloud when nothing is.
 */
#ifndef QFUNNYWIDGETS_SEARCHDETECTIVE_H
#define QFUNNYWIDGETS_SEARCHDETECTIVE_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QWidget>
#include <functional>

class QLineEdit;
class QPainter;
class QTimer;

/**
 * The detective: painted face, magnifying glass, pipe smoke and a speech bubble.
 */
class DetectiveFaceWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int state READ getState WRITE setState)
    Q_PROPERTY(QString comment READ getComment WRITE setComment)
    Q_PROPERTY(QColor hatColor READ getHatColor WRITE setHatColor)
    Q_PROPERTY(QColor skinColor READ getSkinColor WRITE setSkinColor)

public:
    enum State
    {
        IDLE = 0,
        TYPING = 1,
        SEARCHING = 2,
        FOUND = 3,
        NONE = 4
    };
    Q_ENUM(State)

    /** Pipe smoke (kind 0) or rain drop (kind 1). */
    struct Puff
    {
        Puff(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 1.5, int kind = 0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life), kind(kind) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
        int kind;
    };

    /** The detective's lines for a state (FOUND has none: the count is reported instead). */
    static QStringList lines(int state);

    struct Options
    {
        int state = DetectiveFaceWidget::IDLE;
        QString comment = DetectiveFaceWidget::lines(DetectiveFaceWidget::IDLE).first();
        QColor hatColor = QColor(QStringLiteral("#8B6B4A"));
        QColor skinColor = QColor(QStringLiteral("#F3C6A5"));
    };

    explicit DetectiveFaceWidget(QWidget *parent = nullptr);
    explicit DetectiveFaceWidget(const Options &options, QWidget *parent = nullptr);

    int getState() const;
    QString getComment() const;
    QColor getHatColor() const;
    QColor getSkinColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setState(int state);
    void setComment(const QString &comment);
    void setHatColor(const QColor &color);
    void setSkinColor(const QColor &color);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void face(QPointF &c, qreal &R) const;
    void drawDetective(QPainter &p, const QPointF &c, qreal R, int st, qreal now);
    void drawBubble(QPainter &p, const QRectF &area, qreal now);

    // Options.
    int m_state;
    QString m_comment;
    QColor m_hatColor;
    QColor m_skinColor;
    // State.
    qreal m_changedAt = 0.0;
    QList<Puff> m_puffs;
    qreal m_puffAcc = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

/**
 * Search line edit + detective. Emits searchRequested after a debounce; if a search function is set it is called
 * after a short "investigation" and its return value is reported as the result count.
 */
class SearchDetectiveWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString placeholder READ getPlaceholder WRITE setPlaceholder)
    Q_PROPERTY(int debounce READ getDebounce WRITE setDebounce)

public:
    using SearchFunction = std::function<int(const QString &)>;

    struct Options
    {
        QString placeholder = QStringLiteral("Search...");
        int debounce = 450;  ///< ms
        SearchFunction searchFunction;
        std::function<void(const QString &)> searchRequested;
        std::function<void(const QString &)> textChanged;
        std::function<void()> returnPressed;
    };

    explicit SearchDetectiveWidget(QWidget *parent = nullptr);
    explicit SearchDetectiveWidget(const Options &options, QWidget *parent = nullptr);

    DetectiveFaceWidget *getFace() const;
    QLineEdit *getLineEdit() const;
    QString getText() const;
    QString getPlaceholder() const;
    int getDebounce() const;
    SearchFunction getSearchFunction() const;
    void setSearchFunction(const SearchFunction &searchFunction);

public slots:
    void setResultCount(int count);
    void setText(const QString &text);
    void setPlaceholder(const QString &placeholder);
    void setDebounce(int debounce);

signals:
    void searchRequested(const QString &text);
    void textChanged(const QString &text);
    void returnPressed();

private slots:
    void onText(const QString &text);
    void onReturn();
    void fire();
    void autoResult();

private:
    SearchFunction m_searchFunction;
    DetectiveFaceWidget *m_face;
    QLineEdit *m_edit;
    QTimer *m_debounce;
    QTimer *m_investigate;
};

#endif // QFUNNYWIDGETS_SEARCHDETECTIVE_H
