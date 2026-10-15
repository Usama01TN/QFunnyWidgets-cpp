/**
 * DachshundTabsWidget: a segmented control whose selection highlight is a dachshund. When you switch tabs its
 * head end runs ahead while the tail end lags, so the dog stretches across the tabs (tongue out on long runs)
 * before catching up.
 */
#ifndef QFUNNYWIDGETS_DACHSHUNDTABS_H
#define QFUNNYWIDGETS_DACHSHUNDTABS_H

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
 * Segmented control with a stretchy dachshund.
 */
class QFUNNYWIDGETS_EXPORT DachshundTabsWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int current READ getCurrent WRITE setCurrent NOTIFY currentChanged)
    Q_PROPERTY(QStringList tabs READ getTabs WRITE setTabs)
    Q_PROPERTY(QColor dogColor READ getDogColor WRITE setDogColor)
    Q_PROPERTY(QColor trackColor READ getTrackColor WRITE setTrackColor)

public:
    static QStringList defaultTabs();

    struct Options
    {
        QStringList tabs = DachshundTabsWidget::defaultTabs();
        int current = 0;
        QColor dogColor = QColor(QStringLiteral("#B5651D"));
        QColor trackColor = QColor(127, 127, 127, 40);
        std::function<void(int)> currentChanged;
    };

    explicit DachshundTabsWidget(QWidget *parent = nullptr);
    explicit DachshundTabsWidget(const Options &options, QWidget *parent = nullptr);

    int getCurrent() const;
    QStringList getTabs() const;
    QColor getDogColor() const;
    QColor getTrackColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setCurrent(int current);
    void setTabs(const QStringList &tabs);
    void setDogColor(const QColor &color);
    void setTrackColor(const QColor &color);

signals:
    void currentChanged(int index);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QRectF track() const;
    QList<QRectF> segments() const;
    void drawDog(QPainter &p, const QRectF &track, qreal now);

    // Options.
    QStringList m_tabs;
    int m_current;
    QColor m_dogColor;
    QColor m_trackColor;
    // State: the dog's two ends move independently.
    qreal m_left = -1.0;
    qreal m_right = -1.0;
    qreal m_vl = 0.0;
    qreal m_vr = 0.0;
    qreal m_facing = 1.0;
    qreal m_changedAt = -10.0;
    int m_hover = -1;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_DACHSHUNDTABS_H
