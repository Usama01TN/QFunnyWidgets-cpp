/**
 * VolumeKnobWidget: a rotary volume dial with a face. Muted it sleeps with a zipped mouth, then it smiles, sings
 * (with floating music notes), squints at loud volumes and screams (shaking, glowing red, sound waves) at the top.
 */
#ifndef QFUNNYWIDGETS_VOLUMEKNOB_H
#define QFUNNYWIDGETS_VOLUMEKNOB_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Rotary dial from zipped mouth to screaming.
 */
class VolumeKnobWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ getValue WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QColor knobColor READ getKnobColor WRITE setKnobColor)
    Q_PROPERTY(QColor trackColor READ getTrackColor WRITE setTrackColor)
    Q_PROPERTY(bool showLabel READ isShowLabel WRITE setShowLabel)

public:
    /** A floating music note. */
    struct Note
    {
        Note(qreal x = 0.0, qreal y = 0.0, qreal vx = 0.0, qreal vy = 0.0, qreal life = 1.5, qreal size = 1.0)
            : x(x), y(y), vx(vx), vy(vy), life(life), maxLife(life), size(size) {}
        qreal x;
        qreal y;
        qreal vx;
        qreal vy;
        qreal life;
        qreal maxLife;
        qreal size;
    };

    static constexpr qreal START_ANGLE = 225.0; ///< degrees, maths convention (0 = right, counter-clockwise)
    static constexpr qreal SWEEP = 270.0;

    /** Level colour for a ratio 0..1 (teal -> yellow -> orange -> red). */
    static QColor levelColor(qreal t);

    struct Options
    {
        int value = 40;  ///< 0..100
        QColor knobColor = QColor(QStringLiteral("#E9EAF0"));
        QColor trackColor = QColor(127, 127, 127, 60);
        bool showLabel = true;
        std::function<void(int)> valueChanged;
    };

    explicit VolumeKnobWidget(QWidget *parent = nullptr);
    explicit VolumeKnobWidget(const Options &options, QWidget *parent = nullptr);

    int getValue() const;
    QColor getKnobColor() const;
    QColor getTrackColor() const;
    bool isShowLabel() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void toggleMute();
    void setValue(int value);
    void setKnobColor(const QColor &color);
    void setTrackColor(const QColor &color);
    void setShowLabel(bool showLabel);

signals:
    void valueChanged(int value);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    void geometry(QPointF &c, qreal &R, qreal &labelTop) const;
    int valueFromPos(const QPointF &pos) const;
    static QString mood(qreal t);
    void drawFace(QPainter &p, const QPointF &c, qreal r, qreal t, qreal now);
    static void drawNote(QPainter &p, const QPointF &pt, qreal s, const QColor &color);

    // Options.
    int m_value;
    QColor m_knobColor;
    QColor m_trackColor;
    bool m_showLabel;
    // State.
    qreal m_shown;
    bool m_dragging = false;
    int m_beforeMute = 40;
    QList<Note> m_notes;
    qreal m_noteAcc = 0.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_VOLUMEKNOB_H
