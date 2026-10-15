/**
 * PasswordJudgeWidget: a password field reviewed live by a powdered-wig judge (JudgeFaceWidget) with a monocle.
 * The judge's expression, comment and strength meter follow the password strength; he drops his monocle at
 * terrible passwords, follows the text cursor with his eyes and covers them when the password is revealed.
 */
#ifndef QFUNNYWIDGETS_PASSWORDJUDGE_H
#define QFUNNYWIDGETS_PASSWORDJUDGE_H

#include <QColor>
#include <QElapsedTimer>
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

class QLineEdit;
class QPainter;
class QPushButton;
class QTimer;

/**
 * Password strength: -1 = empty, 0 = terrible, 1 = weak, 2 = fair, 3 = good, 4 = strong.
 */
QFUNNYWIDGETS_EXPORT int passwordStrength(const QString &text, int minLength = 8);

/**
 * The judge: a painted face with a speech bubble and a strength meter.
 */
class QFUNNYWIDGETS_EXPORT JudgeFaceWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int strength READ getStrength WRITE setStrength)
    Q_PROPERTY(QString comment READ getComment WRITE setComment)
    Q_PROPERTY(bool covering READ isCovering WRITE setCovering)
    Q_PROPERTY(qreal look READ getLook WRITE setLook)
    Q_PROPERTY(QColor skinColor READ getSkinColor WRITE setSkinColor)
    Q_PROPERTY(QColor wigColor READ getWigColor WRITE setWigColor)

public:
    /** The comment shown before anything was typed. */
    static QString defaultComment();

    struct Options
    {
        int strength = -1;  ///< -1..4
        QString comment = JudgeFaceWidget::defaultComment();
        QColor skinColor = QColor(QStringLiteral("#F6C9A8"));
        QColor wigColor = QColor(QStringLiteral("#F1EFE9"));
    };

    explicit JudgeFaceWidget(QWidget *parent = nullptr);
    explicit JudgeFaceWidget(const Options &options, QWidget *parent = nullptr);

    int getStrength() const;
    QString getComment() const;
    bool isCovering() const;
    qreal getLook() const;
    QColor getSkinColor() const;
    QColor getWigColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setStrength(int strength);
    void setComment(const QString &comment);
    void setCovering(bool covering);
    void setLook(qreal look);
    void setSkinColor(const QColor &color);
    void setWigColor(const QColor &color);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    void drawJudge(QPainter &p, const QPointF &c, qreal R, qreal now);
    void drawVerdict(QPainter &p, const QRectF &area, qreal now);

    // Options.
    int m_strength;
    QString m_comment;
    QColor m_skinColor;
    QColor m_wigColor;
    // State.
    qreal m_cover = 0.0;
    qreal m_coverTarget = 0.0;
    qreal m_lookX = 0.0;
    qreal m_lookTarget = 0.0;
    qreal m_mono = 0.0; // 0 = in place, 1 = dropped
    qreal m_monoVel = 0.0;
    qreal m_meter = 0.0;
    qreal m_changedAt = -10.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

/**
 * Password line edit + Show/Hide toggle + the judge.
 */
class QFUNNYWIDGETS_EXPORT PasswordJudgeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString placeholder READ getPlaceholder WRITE setPlaceholder)
    Q_PROPERTY(int minLength READ getMinLength WRITE setMinLength)
    Q_PROPERTY(bool showToggle READ isShowToggle WRITE setShowToggle)
    Q_PROPERTY(bool revealed READ isRevealed WRITE setRevealed)

public:
    struct Options
    {
        QString text;
        QString placeholder = QStringLiteral("Enter a password");
        int minLength = 8;
        bool showToggle = true;
        bool revealed = false;
        std::function<void(int)> strengthChanged;
        std::function<void(const QString &)> textChanged;
        std::function<void()> returnPressed;
    };

    explicit PasswordJudgeWidget(QWidget *parent = nullptr);
    explicit PasswordJudgeWidget(const Options &options, QWidget *parent = nullptr);

    int getStrength() const;
    JudgeFaceWidget *getFace() const;
    QLineEdit *getLineEdit() const;
    QString getText() const;
    QString getPlaceholder() const;
    int getMinLength() const;
    bool isShowToggle() const;
    bool isRevealed() const;

public slots:
    void setText(const QString &text);
    void setPlaceholder(const QString &placeholder);
    void setMinLength(int minLength);
    void setShowToggle(bool showToggle);
    void setRevealed(bool revealed);

signals:
    void strengthChanged(int strength);
    void textChanged(const QString &text);
    void returnPressed();

private slots:
    void onText(const QString &text);
    void onCursor(int oldPos, int newPos);

private:
    QString comment() const;

    int m_minLength;
    int m_strength = -1;
    JudgeFaceWidget *m_face;
    QLineEdit *m_edit;
    QPushButton *m_toggle;
};

#endif // QFUNNYWIDGETS_PASSWORDJUDGE_H
