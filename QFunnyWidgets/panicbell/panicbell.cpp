/**
 * PanicBellWidget: a notification bell whose stress grows with the unread count: asleep at 0, calm with a few,
 * sweating and swinging past the panic threshold, and in full meltdown (spiral eyes, alarm glow) when the count
 * overflows the badge. Clearing it gives a sigh of relief.
 */
#include "panicbell.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

const qreal PI = 3.14159265358979323846;

inline QPoint eventPos(const QMouseEvent *e)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position().toPoint();
#else
    return e->pos();
#endif
}

inline qreal clamp(qreal x, qreal a, qreal b)
{
    return x < a ? a : (x > b ? b : x);
}

inline qreal uniform(qreal a, qreal b)
{
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}

inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

} // namespace

PanicBellWidget::PanicBellWidget(QWidget *parent)
    : PanicBellWidget(Options(), parent)
{
}

PanicBellWidget::PanicBellWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_count(std::max(0, options.count)),
      m_color(options.color),
      m_badgeColor(options.badgeColor),
      m_panicThreshold(std::max(1, options.panicThreshold)),
      m_maxDisplay(std::max(1, options.maxDisplay)),
      m_clearOnClick(options.clearOnClick)
{
    if (options.clicked)
        connect(this, &PanicBellWidget::clicked, this, options.clicked);
    if (options.countChanged)
        connect(this, &PanicBellWidget::countChanged, this, options.countChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PanicBellWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal PanicBellWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

int PanicBellWidget::stress() const
{
    if (m_count <= 0)
        return 0;
    if (m_count > m_maxDisplay)
        return 3;
    if (m_count >= m_panicThreshold)
        return 2;
    return 1;
}

void PanicBellWidget::geometry(QPointF &pivot, qreal &s) const
{
    const qreal w = width();
    const qreal h = height();
    s = std::max(10.0, std::min(w * 0.7, h * 0.78));
    pivot = QPointF(w / 2.0, (h - s) / 2.0 + s * 0.06);
}

// ---------- Public API ----------
void PanicBellWidget::ring()
{
    m_angVel += 260.0 * (uniform(0, 1) < 0.5 ? 1 : -1);
    m_dingAt = now();
    update();
}

void PanicBellWidget::increment()
{
    setCount(m_count + 1);
}

void PanicBellWidget::clear()
{
    setCount(0);
}

int PanicBellWidget::getCount() const
{
    return m_count;
}

void PanicBellWidget::setCount(int count)
{
    count = std::max(0, count);
    if (count == m_count)
        return;
    const int old = m_count;
    m_count = count;
    if (count > old) {
        ring();
        m_badgeVel += 8.0;
    } else if (count == 0 && old > 0) {
        m_reliefAt = now();
        QPointF pv;
        qreal s;
        geometry(pv, s);
        for (int k = 0; k < 7; ++k) {
            const qreal a = -PI / 2.0 + uniform(-0.9, 0.9);
            m_puffs.append(Puff(pv.x() + s * 0.2, pv.y() + s * 0.62, std::cos(a) * 25.0 + 30.0, std::sin(a) * 25.0,
                                uniform(0.8, 1.2), 1));
        }
    }
    emit countChanged(count);
    update();
}

QColor PanicBellWidget::getColor() const
{
    return m_color;
}

void PanicBellWidget::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QColor PanicBellWidget::getBadgeColor() const
{
    return m_badgeColor;
}

void PanicBellWidget::setBadgeColor(const QColor &color)
{
    if (m_badgeColor != color) {
        m_badgeColor = color;
        update();
    }
}

int PanicBellWidget::getPanicThreshold() const
{
    return m_panicThreshold;
}

void PanicBellWidget::setPanicThreshold(int panicThreshold)
{
    m_panicThreshold = std::max(1, panicThreshold);
    update();
}

int PanicBellWidget::getMaxDisplay() const
{
    return m_maxDisplay;
}

void PanicBellWidget::setMaxDisplay(int maxDisplay)
{
    m_maxDisplay = std::max(1, maxDisplay);
    update();
}

bool PanicBellWidget::isClearOnClick() const
{
    return m_clearOnClick;
}

void PanicBellWidget::setClearOnClick(bool clearOnClick)
{
    m_clearOnClick = clearOnClick;
}

// ---------- Qt overrides ----------
QSize PanicBellWidget::sizeHint() const
{
    return QSize(180, 180);
}

QSize PanicBellWidget::minimumSizeHint() const
{
    return QSize(48, 48);
}

void PanicBellWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void PanicBellWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void PanicBellWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(eventPos(e))) {
        emit clicked();
        if (m_clearOnClick)
            clear();
    }
    QWidget::mouseReleaseEvent(e);
}

// ---------- Animation tick ----------
void PanicBellWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const int st = stress();
    // Swing: damped pendulum, kept going by panic.
    if (st >= 2)
        m_angVel += std::sin(t * (st == 3 ? 14.0 : 9.0)) * (st == 3 ? 900.0 : 260.0) * dt;
    m_angVel += (-m_angle * 90.0 - m_angVel * 3.2) * dt;
    m_angle = clamp(m_angle + m_angVel * dt, -40.0, 40.0);
    m_badgeVel += ((1.0 - m_badgePop) * 300.0 - m_badgeVel * 12.0) * dt;
    m_badgePop += m_badgeVel * dt;
    // Sweat.
    QPointF pv;
    qreal s;
    geometry(pv, s);
    if (st >= 2) {
        m_sweatAcc += dt * (st == 2 ? 5.0 : 12.0);
        while (m_sweatAcc > 1.0) {
            m_sweatAcc -= 1.0;
            const qreal side = uniform(0, 1) < 0.5 ? -1.0 : 1.0;
            const qreal vx = side * uniform(40, 90);
            const qreal vy = -uniform(60, 120);
            m_puffs.append(Puff(pv.x() + side * s * 0.25, pv.y() + s * 0.3, vx, vy, 0.7, 0));
        }
    }
    QList<Puff> alive;
    for (Puff pf : std::as_const(m_puffs)) {
        if (pf.kind == 0)
            pf.vy += 500.0 * dt;
        pf.x += pf.vx * dt;
        pf.y += pf.vy * dt;
        pf.life -= dt;
        if (pf.life > 0)
            alive.append(pf);
    }
    m_puffs = alive;
    update();
}

// ---------- Painting ----------
void PanicBellWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF pivot;
    qreal s;
    geometry(pivot, s);
    const int st = stress();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Meltdown alarm glow.
    if (st == 3) {
        const qreal a = 0.5 + 0.5 * std::sin(tNow * 12.0);
        QRadialGradient glow(QPointF(pivot.x(), pivot.y() + s * 0.45), s * 0.75);
        QColor gc(QStringLiteral("#FF2E3F"));
        gc.setAlpha(int(120 * a));
        glow.setColorAt(0.0, gc);
        gc.setAlpha(0);
        glow.setColorAt(1.0, gc);
        p.setPen(Qt::NoPen);
        p.setBrush(glow);
        p.drawEllipse(QPointF(pivot.x(), pivot.y() + s * 0.45), s * 0.75, s * 0.75);
    }
    // Ding arcs.
    const qreal since = tNow - m_dingAt;
    if (since < 0.6 || st == 3) {
        const qreal t = since < 0.6 ? (since / 0.6) : pyMod(tNow * 2.0, 1.0);
        QColor ac(palette().text().color());
        ac.setAlpha(int(200 * (1.0 - t)));
        const QPen pen(ac, std::max(1.5, s * 0.025), Qt::SolidLine, Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const qreal cy = pivot.y() + s * 0.4;
        for (int k = 0; k < 2; ++k) {
            const qreal rr = s * (0.5 + 0.12 * k + 0.1 * t);
            p.drawArc(QRectF(pivot.x() - rr, cy - rr, rr * 2, rr * 2), -25 * 16, 50 * 16);
            p.drawArc(QRectF(pivot.x() - rr, cy - rr, rr * 2, rr * 2), 155 * 16, 50 * 16);
        }
    }
    p.save();
    p.translate(pivot);
    const qreal shake = st < 3 ? 0.0 : 2.0;
    const qreal jx = uniform(-shake, shake);
    const qreal jy = uniform(-shake, shake);
    p.translate(jx, jy);
    p.rotate(m_angle);
    drawBell(p, s, st, tNow);
    p.restore();
    // Badge (not rotating, so it stays readable).
    if (m_count > 0)
        drawBadge(p, pivot, s);
    // Puffs.
    p.setPen(Qt::NoPen);
    for (const Puff &pf : std::as_const(m_puffs)) {
        const qreal a = clamp(pf.life / pf.maxLife, 0.0, 1.0);
        if (pf.kind == 0) {
            QColor col(QStringLiteral("#7FD6FF"));
            col.setAlpha(int(255 * a));
            QPainterPath drop;
            const qreal ds = std::max(2.0, s * 0.025);
            drop.moveTo(pf.x, pf.y - ds * 1.6);
            drop.quadTo(pf.x + ds, pf.y, pf.x, pf.y + ds);
            drop.quadTo(pf.x - ds, pf.y, pf.x, pf.y - ds * 1.6);
            p.fillPath(drop, col);
        } else {
            QColor col(220, 225, 235);
            col.setAlpha(int(170 * a));
            p.setBrush(col);
            const qreal rr = s * (0.03 + 0.06 * (1.0 - a));
            p.drawEllipse(QPointF(pf.x, pf.y), rr, rr);
        }
    }
    p.end();
}

void PanicBellWidget::drawBell(QPainter &p, qreal s, int stress, qreal now)
{
    const QColor bc = m_color;
    // Clapper swings a bit behind.
    p.setPen(Qt::NoPen);
    p.setBrush(bc.darker(150));
    p.drawEllipse(QPointF(-m_angle * s * 0.004, s * 0.86), s * 0.09, s * 0.09);
    // Body.
    QPainterPath body;
    const qreal top = s * 0.1;
    const qreal bottom = s * 0.78;
    body.moveTo(-s * 0.08, top);
    body.cubicTo(-s * 0.36, top + s * 0.02, -s * 0.34, s * 0.45, -s * 0.38, s * 0.62);
    body.quadTo(-s * 0.4, bottom - s * 0.04, -s * 0.47, bottom);
    body.lineTo(s * 0.47, bottom);
    body.quadTo(s * 0.4, bottom - s * 0.04, s * 0.38, s * 0.62);
    body.cubicTo(s * 0.34, s * 0.45, s * 0.36, top + s * 0.02, s * 0.08, top);
    body.closeSubpath();
    QRadialGradient grad(QPointF(-s * 0.15, s * 0.3), s * 0.7);
    grad.setColorAt(0.0, bc.lighter(130));
    grad.setColorAt(1.0, bc.darker(115));
    p.setBrush(grad);
    p.setPen(QPen(bc.darker(150), std::max(1.5, s * 0.02)));
    p.drawPath(body);
    p.drawRoundedRect(QRectF(-s * 0.5, bottom - s * 0.01, s, s * 0.07), s * 0.035, s * 0.035);
    p.setBrush(bc.darker(110));
    p.drawEllipse(QPointF(0, top - s * 0.02), s * 0.06, s * 0.06);
    // Face.
    const QColor ink(QStringLiteral("#3A2A10"));
    const qreal fy = s * 0.42;
    const qreal ex = s * 0.11;
    const QPen pen(ink, std::max(1.2, s * 0.022), Qt::SolidLine, Qt::RoundCap);
    const bool relieved = now - m_reliefAt < 1.5;
    if (stress == 0) {
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        for (qreal d : {-1.0, 1.0}) {
            if (relieved)
                p.drawArc(QRectF(d * ex - s * 0.045, fy - s * 0.04, s * 0.09, s * 0.06), 0, 180 * 16);
            else
                p.drawArc(QRectF(d * ex - s * 0.045, fy - s * 0.03, s * 0.09, s * 0.06), 180 * 16, 180 * 16);
        }
        const qreal breathe = 1.0 + 0.2 * std::sin(now * 2.5);
        if (relieved)
            p.drawArc(QRectF(-s * 0.06, fy + s * 0.06, s * 0.12, s * 0.07), 200 * 16, 140 * 16);
        else
            p.drawEllipse(QPointF(0, fy + s * 0.11), s * 0.025 * breathe, s * 0.03 * breathe);
        // Sleeping "z".
        if (!relieved) {
            QFont f(font());
            f.setBold(true);
            for (int k = 0; k < 2; ++k) {
                const qreal ph = pyMod(now * 0.5 + k * 0.5, 1.0);
                f.setPointSizeF(std::max(5.0, s * (0.06 + 0.05 * ph)));
                p.setFont(f);
                QColor zc(palette().text().color());
                zc.setAlpha(int(220 * std::sin(ph * PI)));
                p.setPen(zc);
                p.drawText(QPointF(s * 0.28 + ph * s * 0.12, fy - s * 0.15 - ph * s * 0.25), QStringLiteral("z"));
            }
        }
        return;
    }
    if (stress == 3) {
        // Spiral eyes.
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(d * ex, fy), s * 0.075, s * 0.075);
        p.setPen(QPen(ink, std::max(1.0, s * 0.012), Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        for (qreal d : {-1.0, 1.0}) {
            QPainterPath spiral;
            for (int k = 0; k < 36; ++k) {
                const qreal a = k * 0.5 + now * 12.0 * d;
                const qreal rr = s * 0.0019 * k;
                const QPointF pt(d * ex + std::cos(a) * rr, fy + std::sin(a) * rr);
                if (k == 0)
                    spiral.moveTo(pt);
                else
                    spiral.lineTo(pt);
            }
            p.drawPath(spiral);
        }
    } else {
        const qreal er = s * (stress == 1 ? 0.04 : 0.055);
        for (qreal d : {-1.0, 1.0}) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(d * ex, fy), er, er * 1.1);
            p.setBrush(ink);
            const qreal look = stress == 2 ? std::sin(now * 7.0) * er * 0.4 : 0.0;
            p.drawEllipse(QPointF(d * ex + look, fy + er * 0.1), er * 0.55, er * 0.6);
        }
        if (stress == 2) {
            p.setPen(pen);
            for (qreal d : {-1.0, 1.0})
                p.drawLine(QPointF(d * (ex + s * 0.06), fy - s * 0.07), QPointF(d * (ex - s * 0.04), fy - s * 0.1));
        }
    }
    // Mouth.
    p.setPen(pen);
    if (stress == 1) {
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-s * 0.06, fy + s * 0.04, s * 0.12, s * 0.08), 200 * 16, 140 * 16);
    } else {
        p.setBrush(QColor(QStringLiteral("#7A1F1F")));
        const qreal o = 1.0 + 0.25 * std::sin(now * (stress == 3 ? 20.0 : 8.0));
        p.drawEllipse(QPointF(0, fy + s * 0.12), s * 0.045 * o, s * (stress == 2 ? 0.04 : 0.06) * o);
    }
}

void PanicBellWidget::drawBadge(QPainter &p, const QPointF &pivot, qreal s)
{
    const QString text = m_count <= m_maxDisplay ? QString::number(m_count)
                                                 : QStringLiteral("%1+").arg(m_maxDisplay);
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(std::max(6.0, s * 0.075));
    p.setFont(f);
    const qreal tw = p.fontMetrics().horizontalAdvance(text);
    const qreal bh = s * 0.2;
    const qreal bw = std::max(bh, tw + bh * 0.6);
    const QPointF c(pivot.x() + s * 0.3, pivot.y() + s * 0.2);
    p.save();
    p.translate(c);
    const qreal k = std::max(0.2, m_badgePop);
    p.scale(k, k);
    const QRectF rect(-bw / 2.0, -bh / 2.0, bw, bh);
    p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), std::max(1.5, s * 0.015)));
    p.setBrush(m_badgeColor);
    p.drawRoundedRect(rect, bh / 2.0, bh / 2.0);
    p.setPen(QColor(QStringLiteral("#FFFFFF")));
    p.drawText(rect, Qt::AlignCenter, text);
    p.restore();
}
