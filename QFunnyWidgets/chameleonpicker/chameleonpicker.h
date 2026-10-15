/**
 * ChameleonColorPickerWidget: a swatch colour picker with a chameleon on a branch. Its turret eyes track the hovered
 * swatch (the far eye wanders independently); picking a colour makes it shoot its tongue at the swatch, "eat" the
 * colour and change colour from snout to tail.
 */
#ifndef QFUNNYWIDGETS_CHAMELEONPICKER_H
#define QFUNNYWIDGETS_CHAMELEONPICKER_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Colour picker whose chameleon eats the colour you pick.
 */
class ChameleonColorPickerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QStringList colors READ getColors WRITE setColors)
    Q_PROPERTY(int columns READ getColumns WRITE setColumns)
    Q_PROPERTY(bool showHex READ isShowHex WRITE setShowHex)
    Q_PROPERTY(QColor branchColor READ getBranchColor WRITE setBranchColor)

public:
    static constexpr qreal TONGUE_OUT = 0.16;
    static constexpr qreal TONGUE_HOLD = 0.3;
    static constexpr qreal TONGUE_IN = 0.5;
    static constexpr qreal WIPE_START = 0.42;
    static constexpr qreal WIPE_END = 1.1;

    static QList<QColor> defaultColors();

    struct Options
    {
        QList<QColor> colors = ChameleonColorPickerWidget::defaultColors();
        QColor color = QColor(QStringLiteral("#3BCEAC"));
        int columns = 4;  ///< 1..8
        bool showHex = true;
        QColor branchColor = QColor(QStringLiteral("#8B5E3C"));
        std::function<void(const QColor &)> colorChanged;
    };

    explicit ChameleonColorPickerWidget(QWidget *parent = nullptr);
    explicit ChameleonColorPickerWidget(const Options &options, QWidget *parent = nullptr);

    QColor getColor() const;
    QStringList getColors() const;
    int getColumns() const;
    bool isShowHex() const;
    QColor getBranchColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setColor(const QColor &color);
    void setColors(const QStringList &colors);
    void setColorList(const QList<QColor> &colors);
    void setColumns(int columns);
    void setShowHex(bool showHex);
    void setBranchColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QList<QRectF> layoutSwatches(QPointF &c, qreal &s) const;
    QPointF mouth(const QPointF &c, qreal s) const;
    QPointF eye(const QPointF &c, qreal s) const;
    void pick(int index);
    qreal wipe() const;
    QColor bodyColorAt(qreal pos) const;
    void drawChameleon(QPainter &p, const QPointF &c, qreal s, qreal now);

    // Options.
    QList<QColor> m_colors;
    QColor m_color;
    int m_columns;
    bool m_showHex;
    QColor m_branchColor;
    // State.
    QColor m_prevColor;
    qreal m_pickAt = -10.0;
    int m_target = -1;
    int m_hover = -1;
    qreal m_lookAng = 0.0;
    qreal m_wanderAng = 0.0;
    qreal m_wanderTarget = 0.0;
    qreal m_nextWander = 1.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_CHAMELEONPICKER_H
