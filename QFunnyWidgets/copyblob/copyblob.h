/**
 * CopyBlobButtonWidget: a copy-to-clipboard chip (prompt + text) with a little blob living at its end. Clicking it
 * copies the text and the blob undergoes mitosis: its clone hops up "into the clipboard" with a sparkle while a
 * "Copied!" bubble appears.
 */
#ifndef QFUNNYWIDGETS_COPYBLOB_H
#define QFUNNYWIDGETS_COPYBLOB_H

#include <QColor>
#include <QElapsedTimer>
#include <QPointF>
#include <QRectF>
#include <QWidget>
#include <functional>

class QPainter;
class QPainterPath;
class QTimer;

/**
 * Copy-to-clipboard chip whose blob clones itself.
 */
class CopyBlobButtonWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText)
    Q_PROPERTY(QString label READ getLabel WRITE setLabel)
    Q_PROPERTY(QString prompt READ getPrompt WRITE setPrompt)
    Q_PROPERTY(QColor blobColor READ getBlobColor WRITE setBlobColor)
    Q_PROPERTY(QColor chipColor READ getChipColor WRITE setChipColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)

public:
    static constexpr qreal SPLIT = 0.35;
    static constexpr qreal HOP = 1.0;
    static constexpr qreal BUBBLE = 1.7;

    struct Options
    {
        QString text = QStringLiteral("pip install funnywidgets");
        QString label;  ///< Shown instead of the text when set (e.g. a masked secret).
        QString prompt = QStringLiteral("$");
        QColor blobColor = QColor(QStringLiteral("#9BDB4D"));
        QColor chipColor = QColor(QStringLiteral("#23252E"));
        QColor textColor = QColor(QStringLiteral("#E6E8EF"));
        std::function<void(const QString &)> copied;
    };

    explicit CopyBlobButtonWidget(QWidget *parent = nullptr);
    explicit CopyBlobButtonWidget(const Options &options, QWidget *parent = nullptr);

    QString getText() const;
    QString getLabel() const;
    QString getPrompt() const;
    QColor getBlobColor() const;
    QColor getChipColor() const;
    QColor getTextColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void copy();
    void setText(const QString &text);
    void setLabel(const QString &label);
    void setPrompt(const QString &prompt);
    void setBlobColor(const QColor &color);
    void setChipColor(const QColor &color);
    void setTextColor(const QColor &color);

signals:
    void copied(const QString &text);

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
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QRectF chip() const;
    void drawBlobs(QPainter &p, const QPointF &c, qreal r, qreal since, qreal now);
    void paintBlobShape(QPainter &p, const QPainterPath &shape);
    void paintBlob(QPainter &p, const QPointF &c, qreal r, qreal now, bool happy);
    void drawBlobFace(QPainter &p, const QPointF &c, qreal r, qreal now, bool happy);

    // Options.
    QString m_text;
    QString m_label;
    QString m_prompt;
    QColor m_blobColor;
    QColor m_chipColor;
    QColor m_textColor;
    // State.
    qreal m_copiedAt = -10.0;
    bool m_pressed = false;
    bool m_hovered = false;
    qreal m_blinkAt = 2.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_COPYBLOB_H
