/**
 * JellySwitchWidget: a toggle switch whose knob is a jelly blob with a face. It sleeps (with floating
 * "z") when off, wakes up happy when on, squashes when pressed and wobbles like jelly when it slides.
 */
#ifndef QFUNNYWIDGETS_JELLYSWITCH_H
#define QFUNNYWIDGETS_JELLYSWITCH_H

#include <QColor>
#include <QElapsedTimer>
#include <QRectF>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * A wobbly jelly toggle switch with a face.
 */
class JellySwitchWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY toggled)
    Q_PROPERTY(QColor onColor READ getOnColor WRITE setOnColor)
    Q_PROPERTY(QColor offColor READ getOffColor WRITE setOffColor)
    Q_PROPERTY(QColor knobColor READ getKnobColor WRITE setKnobColor)
    Q_PROPERTY(qreal wobbliness READ getWobbliness WRITE setWobbliness)
    Q_PROPERTY(bool showFace READ isShowFace WRITE setShowFace)

public:
    /** Construction options (the Python keyword arguments, same names and defaults). */
    struct Options
    {
        bool checked = false;
        QColor onColor = QColor(QStringLiteral("#34C759"));
        QColor offColor = QColor(QStringLiteral("#B8BCC8"));
        QColor knobColor = QColor(QStringLiteral("#FFFFFF"));
        qreal wobbliness = 0.7; ///< 0 = firm, 1 = very jiggly.
        bool showFace = true;
        std::function<void(bool)> toggled; ///< Connected to the toggled signal when set.
    };

    explicit JellySwitchWidget(QWidget *parent = nullptr);
    explicit JellySwitchWidget(const Options &options, QWidget *parent = nullptr);

    bool isChecked() const;
    QColor getOnColor() const;
    QColor getOffColor() const;
    QColor getKnobColor() const;
    qreal getWobbliness() const;
    bool isShowFace() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void toggle();
    void setChecked(bool checked);
    void setOnColor(const QColor &color);
    void setOffColor(const QColor &color);
    void setKnobColor(const QColor &color);
    void setWobbliness(qreal wobbliness);
    void setShowFace(bool showFace);

signals:
    void toggled(bool checked);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *e) override;
#else
    void enterEvent(QEvent *e) override;
#endif
    void leaveEvent(QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QRectF trackRect() const;
    void drawFace(QPainter &p, qreal r, qreal sx, qreal sy, qreal t, qreal now);

    // Options.
    bool m_checked;
    QColor m_onColor;
    QColor m_offColor;
    QColor m_knobColor;
    qreal m_wobbliness;
    bool m_showFace;
    // State.
    qreal m_pos;        // knob position 0..1
    qreal m_vel = 0.0;
    qreal m_squash = 0.0; // jelly deformation (signed)
    qreal m_squashVel = 0.0;
    qreal m_press = 0.0;
    bool m_pressed = false;
    qreal m_hover = 0.0;
    bool m_hovered = false;
    qreal m_last = 0.0;
    qreal m_toggleTime = -10.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_JELLYSWITCH_H
