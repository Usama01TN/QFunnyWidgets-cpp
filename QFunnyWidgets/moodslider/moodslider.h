/**
 * MoodSliderWidget: a slider whose thumb is a face going from furious (with steam) to ecstatic (with floating
 * hearts). The thumb springs to its position, tilts while moving and shows the current mood in a bubble.
 */
#ifndef QFUNNYWIDGETS_MOODSLIDER_H
#define QFUNNYWIDGETS_MOODSLIDER_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
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
 * A slider with feelings.
 */
class QFUNNYWIDGETS_EXPORT MoodSliderWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ getValue WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int minimum READ getMinimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ getMaximum WRITE setMaximum)
    Q_PROPERTY(QStringList labels READ getLabels WRITE setLabels)
    Q_PROPERTY(bool showLabel READ isShowLabel WRITE setShowLabel)
    Q_PROPERTY(QColor trackColor READ getTrackColor WRITE setTrackColor)

public:
    /** Floating heart (kind 0) or steam puff (kind 1). */
    struct FloatItem
    {
        FloatItem(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 1.0, int kind = 0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life), kind(kind) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
        int kind;
    };

    static QStringList defaultLabels();
    /** Mood colour for a ratio 0..1 (red -> orange -> yellow -> green -> teal). */
    static QColor moodColor(qreal t);

    struct Options
    {
        int value = 50;
        int minimum = 0;
        int maximum = 100;
        QStringList labels = MoodSliderWidget::defaultLabels();
        bool showLabel = true;
        QColor trackColor = QColor(127, 127, 127, 70);
        std::function<void(int)> valueChanged;
        std::function<void(const QString &)> moodChanged;
    };

    explicit MoodSliderWidget(QWidget *parent = nullptr);
    explicit MoodSliderWidget(const Options &options, QWidget *parent = nullptr);

    QString getMood() const;
    int getValue() const;
    int getMinimum() const;
    int getMaximum() const;
    QStringList getLabels() const;
    bool isShowLabel() const;
    QColor getTrackColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setValue(int value);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setLabels(const QStringList &labels);
    void setShowLabel(bool showLabel);
    void setTrackColor(const QColor &color);

signals:
    void valueChanged(int value);
    void moodChanged(const QString &mood);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *e) override;
#else
    void enterEvent(QEvent *e) override;
#endif
    void leaveEvent(QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    qreal ratio() const;
    void geometry(qreal &r, qreal &left, qreal &right, qreal &y) const;
    int valueFromX(qreal x) const;
    void drawFace(QPainter &p, qreal r, qreal t, const QColor &mc, qreal now);
    void drawBubble(QPainter &p, const QString &text, qreal x, qreal bottom, const QColor &color);

    // Options.
    int m_minimum;
    int m_maximum;
    int m_value;
    QStringList m_labels;
    bool m_showLabel;
    QColor m_trackColor;
    // State.
    qreal m_shown;       // animated thumb position 0..1
    qreal m_vel = 0.0;
    bool m_dragging = false;
    bool m_hovered = false;
    qreal m_bubble = 0.0;
    qreal m_press = 0.0;
    QList<FloatItem> m_items;
    qreal m_spawn = 0.0;
    QString m_lastMood;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_MOODSLIDER_H
