/**
 * AntBreadcrumbWidget: a breadcrumb bar made of actual bread crumbs, maintained by a colony of ants. Navigating
 * deeper makes ants carry new crumbs in from the right; going back makes them carry the dropped crumbs away. The
 * current crumb has a little "you are here" flag, and a few ants keep marching along the bottom.
 */
#ifndef QFUNNYWIDGETS_ANTBREADCRUMB_H
#define QFUNNYWIDGETS_ANTBREADCRUMB_H

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
 * Breadcrumbs carried by ants.
 */
class AntBreadcrumbWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList path READ getPath WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QColor crumbColor READ getCrumbColor WRITE setCrumbColor)
    Q_PROPERTY(QColor antColor READ getAntColor WRITE setAntColor)
    Q_PROPERTY(bool marchingAnts READ isMarchingAnts WRITE setMarchingAnts)

public:
    static constexpr qreal CARRY_SPEED = 260.0;
    static constexpr qreal GAP = 24.0;

    /** Crumb state (Python: IN, IDLE, OUT; renamed because <windows.h> defines IN and OUT as macros). */
    enum CrumbState { CarriedIn, Idle, CarriedOut };

    /** One crumb of the path. */
    struct Crumb
    {
        explicit Crumb(const QString &text = QString(), qreal x = 0.0, int state = Idle, qreal delay = 0.0);
        QString text;
        qreal x;
        qreal targetX;
        qreal width = 60.0;
        int state;
        qreal delay;
        qreal lift = 0.0;
        quint32 seed;
    };

    static QStringList defaultPath();

    struct Options
    {
        QStringList path = AntBreadcrumbWidget::defaultPath();
        QColor crumbColor = QColor(QStringLiteral("#E9B872"));
        QColor antColor = QColor(QStringLiteral("#2B1A12"));
        bool marchingAnts = true;
        std::function<void(int)> crumbClicked;
        std::function<void(const QStringList &)> pathChanged;
    };

    explicit AntBreadcrumbWidget(QWidget *parent = nullptr);
    explicit AntBreadcrumbWidget(const Options &options, QWidget *parent = nullptr);

    QStringList getPath() const;
    QColor getCrumbColor() const;
    QColor getAntColor() const;
    bool isMarchingAnts() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setPath(const QStringList &path);
    void push(const QString &name);
    void navigateTo(int index);
    void setCrumbColor(const QColor &color);
    void setAntColor(const QColor &color);
    void setMarchingAnts(bool marchingAnts);

signals:
    void crumbClicked(int index);
    void pathChanged(const QStringList &path);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    qreal now() const;
    QFont crumbFont(bool bold = false) const;
    void layoutCrumbs(bool snap = false);
    QRectF crumbRect(const Crumb &c) const;
    void drawCrumb(QPainter &p, const Crumb &c, bool current, bool hovered, qreal now);
    void drawAnt(QPainter &p, const QPointF &c, qreal phase, qreal scale);

    // Options.
    QColor m_crumbColor;
    QColor m_antColor;
    bool m_marching;
    // State.
    QList<Crumb> m_crumbs;
    QList<Crumb> m_leaving;
    int m_hover = -1;
    bool m_layoutDone = false;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_ANTBREADCRUMB_H
