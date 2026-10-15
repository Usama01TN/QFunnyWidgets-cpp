/**
 * DachshundTabsWidget: a segmented control whose selection highlight is a dachshund. When you switch tabs its
 * head end runs ahead while the tail end lags, so the dog stretches across the tabs (tongue out on long runs)
 * before catching up.
 */
#include "dachshundtabs.h"

#include <QFontMetricsF>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace {

const qreal PI = 3.14159265358979323846;

inline QPointF eventPos(const QMouseEvent *e)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position();
#else
    return e->localPos();
#endif
}

inline qreal clamp(qreal x, qreal a, qreal b)
{
    return x < a ? a : (x > b ? b : x);
}

inline int clampInt(int x, int a, int b)
{
    return x < a ? a : (x > b ? b : x);
}

} // namespace

QStringList DachshundTabsWidget::defaultTabs()
{
    return {QStringLiteral("Overview"), QStringLiteral("Activity"), QStringLiteral("Settings"), QStringLiteral("Billing")};
}

DachshundTabsWidget::DachshundTabsWidget(QWidget *parent)
    : DachshundTabsWidget(Options(), parent)
{
}

DachshundTabsWidget::DachshundTabsWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_tabs(options.tabs.isEmpty() ? defaultTabs() : options.tabs),
      m_current(clampInt(options.current, 0, int(m_tabs.size()) - 1)),
      m_dogColor(options.dogColor),
      m_trackColor(options.trackColor)
{
    if (options.currentChanged)
        connect(this, &DachshundTabsWidget::currentChanged, this, options.currentChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DachshundTabsWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal DachshundTabsWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QRectF DachshundTabsWidget::track() const
{
    const qreal h = height();
    const qreal th = clamp(h - 18.0, 30.0, 46.0);
    return QRectF(2.0, h - th - 2.0, width() - 4.0, th);
}

QList<QRectF> DachshundTabsWidget::segments() const
{
    const QRectF t = track();
    const int n = int(m_tabs.size());
    const qreal sw = t.width() / n;
    QList<QRectF> segs;
    for (int i = 0; i < n; ++i)
        segs.append(QRectF(t.left() + i * sw, t.top(), sw, t.height()));
    return segs;
}

// ---------- Public API ----------
int DachshundTabsWidget::getCurrent() const
{
    return m_current;
}

void DachshundTabsWidget::setCurrent(int current)
{
    current = clampInt(current, 0, int(m_tabs.size()) - 1);
    if (current != m_current) {
        m_facing = current > m_current ? 1.0 : -1.0;
        m_current = current;
        m_changedAt = now();
        emit currentChanged(current);
        update();
    }
}

QStringList DachshundTabsWidget::getTabs() const
{
    return m_tabs;
}

void DachshundTabsWidget::setTabs(const QStringList &tabs)
{
    m_tabs = tabs.isEmpty() ? defaultTabs() : tabs;
    m_current = std::min(m_current, int(m_tabs.size()) - 1);
    m_left = -1.0;
    update();
}

QColor DachshundTabsWidget::getDogColor() const { return m_dogColor; }
QColor DachshundTabsWidget::getTrackColor() const { return m_trackColor; }

void DachshundTabsWidget::setDogColor(const QColor &color)
{
    m_dogColor = color;
    update();
}

void DachshundTabsWidget::setTrackColor(const QColor &color)
{
    m_trackColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize DachshundTabsWidget::sizeHint() const
{
    return QSize(120 * int(m_tabs.size()), 64);
}

QSize DachshundTabsWidget::minimumSizeHint() const
{
    return QSize(60 * int(m_tabs.size()), 50);
}

void DachshundTabsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void DachshundTabsWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void DachshundTabsWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_left = -1.0; // snap to the new geometry
}

void DachshundTabsWidget::mouseMoveEvent(QMouseEvent *e)
{
    const QList<QRectF> segs = segments();
    m_hover = -1;
    for (int i = 0; i < segs.size(); ++i) {
        if (segs.at(i).contains(eventPos(e))) {
            m_hover = i;
            break;
        }
    }
    QWidget::mouseMoveEvent(e);
}

void DachshundTabsWidget::leaveEvent(QEvent *e)
{
    m_hover = -1;
    QWidget::leaveEvent(e);
}

void DachshundTabsWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        const QList<QRectF> segs = segments();
        for (int i = 0; i < segs.size(); ++i) {
            if (segs.at(i).contains(eventPos(e))) {
                setCurrent(i);
                break;
            }
        }
    }
    QWidget::mouseReleaseEvent(e);
}

void DachshundTabsWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Right)
        setCurrent(m_current + 1);
    else if (e->key() == Qt::Key_Left)
        setCurrent(m_current - 1);
    else
        QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void DachshundTabsWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const QRectF seg = segments().at(m_current);
    const qreal pad = 4.0;
    const qreal tl = seg.left() + pad;
    const qreal tr = seg.right() - pad;
    if (m_left < 0) {
        m_left = tl;
        m_right = tr;
    }
    // The end in the direction of travel is the head: it leads. The tail end waits, then follows.
    const bool delay = t - m_changedAt < 0.18;
    const qreal kFront = 180.0, cFront = 18.0;
    const qreal kBack = delay ? 0.0 : 110.0;
    const qreal cBack = delay ? 12.0 : 16.0;
    if (m_facing > 0) {
        m_vr += ((tr - m_right) * kFront - m_vr * cFront) * dt;
        m_vl += ((tl - m_left) * kBack - m_vl * cBack) * dt;
    } else {
        m_vl += ((tl - m_left) * kFront - m_vl * cFront) * dt;
        m_vr += ((tr - m_right) * kBack - m_vr * cBack) * dt;
    }
    m_left += m_vl * dt;
    m_right += m_vr * dt;
    if (m_right - m_left < 20.0) {
        const qreal mid = (m_left + m_right) / 2.0;
        m_left = mid - 10.0;
        m_right = mid + 10.0;
    }
    update();
}

// ---------- Painting ----------
void DachshundTabsWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const QRectF trk = track();
    const QList<QRectF> segs = segments();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(m_trackColor);
    p.drawRoundedRect(trk, trk.height() / 2.0, trk.height() / 2.0);
    const QColor tc(palette().text().color());
    if (0 <= m_hover && m_hover != m_current) {
        QColor hc(tc);
        hc.setAlpha(18);
        p.setBrush(hc);
        const QRectF hs = segs.at(m_hover).adjusted(4, 4, -4, -4);
        p.drawRoundedRect(hs, hs.height() / 2.0, hs.height() / 2.0);
    }
    drawDog(p, trk, tNow);
    // Labels on top of the dog.
    QFont f(font());
    f.setPointSizeF(clamp(trk.height() * 0.27, 8.5, 11.0));
    const QFontMetricsF fm(f);
    for (int i = 0; i < segs.size(); ++i) {
        const QRectF &r = segs.at(i);
        const bool onDog = m_left <= r.center().x() && r.center().x() <= m_right;
        f.setBold(i == m_current);
        p.setFont(f);
        if (onDog) {
            p.setPen(QColor(QStringLiteral("#FFF8EE")));
        } else {
            QColor lc(tc);
            lc.setAlpha(170);
            p.setPen(lc);
        }
        p.drawText(r, Qt::AlignCenter, fm.elidedText(m_tabs.at(i), Qt::ElideRight, r.width() - 36.0));
    }
    if (hasFocus()) {
        QColor fc(tc);
        fc.setAlpha(60);
        p.setPen(QPen(fc, 1.0, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(trk.adjusted(-1, -1, 1, 1), trk.height() / 2.0, trk.height() / 2.0);
    }
    p.end();
}

void DachshundTabsWidget::drawDog(QPainter &p, const QRectF &track, qreal now)
{
    const QColor dc = m_dogColor;
    const QColor ink(QStringLiteral("#2B1A10"));
    const qreal L = m_left;
    const qreal R = m_right;
    const qreal bh = track.height() - 10.0;
    const qreal cy = track.center().y();
    const bool moving = std::abs(m_vl) + std::abs(m_vr) > 30.0;
    const qreal face = m_facing;
    const qreal stretch = (R - L) / std::max(1.0, segments().at(0).width() - 8.0);
    // Legs (4 little stumps, scurrying when moving).
    p.setPen(QPen(dc.darker(130), std::max(3.0, bh * 0.16), Qt::SolidLine, Qt::RoundCap));
    const qreal legs[4] = {L + bh * 0.45, L + bh * 0.85, R - bh * 0.85, R - bh * 0.45};
    for (int k = 0; k < 4; ++k) {
        const qreal x = legs[k];
        const qreal swing = moving ? std::sin(now * 22.0 + k * PI / 2.0) * bh * 0.12 : 0.0;
        p.drawLine(QPointF(x, cy + bh * 0.3), QPointF(x + swing, cy + bh * 0.55));
    }
    // Tail at the rear end.
    const qreal rear = face > 0 ? L : R;
    const qreal wag = std::sin(now * (!moving ? 18.0 : 10.0)) * 0.6;
    p.setPen(QPen(dc.darker(115), std::max(2.5, bh * 0.12), Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    QPainterPath tail;
    tail.moveTo(rear, cy - bh * 0.1);
    tail.quadTo(rear - face * bh * 0.45, cy - bh * (0.4 + 0.25 * wag), rear - face * bh * 0.55, cy - bh * (0.75 + 0.3 * wag));
    p.drawPath(tail);
    // Body: the selection pill.
    const QRectF body(L, cy - bh / 2.0, R - L, bh);
    QLinearGradient grad(body.topLeft(), body.bottomLeft());
    grad.setColorAt(0.0, dc.lighter(115));
    grad.setColorAt(1.0, dc.darker(110));
    p.setPen(QPen(dc.darker(140), 1.2));
    p.setBrush(grad);
    p.drawRoundedRect(body, bh / 2.0, bh / 2.0);
    // Belly line gets "thinner" the more it stretches.
    const QColor bl(255, 255, 255, int(clamp(60 - (stretch - 1.0) * 20, 15, 60)));
    p.setPen(QPen(bl, std::max(1.0, bh * 0.08 / std::max(1.0, stretch * 0.6)), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(L + bh * 0.6, cy + bh * 0.28), QPointF(R - bh * 0.6, cy + bh * 0.28));
    // Head at the front end, poking above the track.
    const qreal front = face > 0 ? R : L;
    const QPointF hc(front + face * bh * 0.05, cy - bh * 0.45);
    const qreal hr = bh * 0.48;
    p.setPen(QPen(dc.darker(140), 1.2));
    p.setBrush(dc);
    p.drawEllipse(hc, hr, hr * 0.9);
    // Snout.
    const QRectF snout(hc.x() + (face > 0 ? 0 : -hr * 1.1), hc.y() - hr * 0.05, hr * 1.1, hr * 0.62);
    p.drawRoundedRect(snout, hr * 0.3, hr * 0.3);
    p.setPen(Qt::NoPen);
    p.setBrush(ink);
    p.drawEllipse(QPointF(hc.x() + face * hr * 1.05, hc.y() + hr * 0.1), hr * 0.16, hr * 0.13);
    // Eye.
    p.drawEllipse(QPointF(hc.x() + face * hr * 0.3, hc.y() - hr * 0.25), hr * 0.12,
                  hr * ((moving && stretch > 1.6) ? 0.04 : 0.13));
    // Floppy ear (flaps when running).
    const qreal flap = moving ? std::sin(now * 16.0) * 0.25 : 0.0;
    QPainterPath ear;
    const qreal ex = hc.x() - face * hr * 0.25;
    ear.moveTo(ex, hc.y() - hr * 0.6);
    ear.quadTo(ex - face * hr * (0.7 + flap), hc.y() - hr * 0.1, ex - face * hr * (0.35 + flap), hc.y() + hr * 0.55);
    ear.quadTo(ex + face * hr * 0.1, hc.y() + hr * 0.1, ex, hc.y() - hr * 0.6);
    p.setPen(QPen(dc.darker(150), 1.0));
    p.setBrush(dc.darker(130));
    p.drawPath(ear);
    // Tongue out when it has been running far.
    if (moving && stretch > 1.5) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FF7A90")));
        p.drawEllipse(QPointF(hc.x() + face * hr * 0.75, hc.y() + hr * 0.62), hr * 0.14, hr * 0.2);
    }
}
