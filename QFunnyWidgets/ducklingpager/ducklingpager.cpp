/**
 * DucklingPagerWidget: a carousel page indicator where the dots are lily pads and the current page is shown by a
 * mother duck. Her ducklings swim after her in a line whenever the page changes; she quacks when she arrives.
 */
#include "ducklingpager.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace {

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

DucklingPagerWidget::DucklingPagerWidget(QWidget *parent)
    : DucklingPagerWidget(Options(), parent)
{
}

DucklingPagerWidget::DucklingPagerWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_count(clampInt(options.count, 2, 12)),
      m_current(clampInt(options.current, 0, m_count - 1)),
      m_ducklings(clampInt(options.ducklings, 0, 6)),
      m_wrap(options.wrap),
      m_duckColor(options.duckColor),
      m_ducklingColor(options.ducklingColor),
      m_padColor(options.padColor)
{
    if (options.currentChanged)
        connect(this, &DucklingPagerWidget::currentChanged, this, options.currentChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DucklingPagerWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal DucklingPagerWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QList<qreal> DucklingPagerWidget::layoutPads(qreal &y, qreal &padR, qreal &size) const
{
    const int n = m_count;
    const qreal w = width();
    const qreal h = height();
    const qreal gap = std::min((w - 110.0) / std::max(1, n - 1), 70.0); // side room for the ducks
    const qreal x0 = w / 2.0 - gap * (n - 1) / 2.0;
    y = h * 0.66;
    padR = clamp(gap * 0.32, 7.0, 20.0);
    size = clamp(gap * 0.6, 16.0, 40.0);
    QList<qreal> xs;
    for (int i = 0; i < n; ++i)
        xs.append(x0 + i * gap);
    return xs;
}

void DucklingPagerWidget::ensureDucks()
{
    if (m_ducks.size() != m_ducklings + 1) {
        qreal y, padR, size;
        const QList<qreal> xs = layoutPads(y, padR, size);
        const qreal mx = xs.at(m_current);
        m_ducks.clear();
        for (int k = 0; k < m_ducklings + 1; ++k)
            m_ducks.append(Swimmer(mx - m_facing * k * size * 0.75));
    }
}

// ---------- Public API ----------
void DucklingPagerWidget::next()
{
    if (m_current + 1 < m_count)
        setCurrent(m_current + 1);
    else if (m_wrap)
        setCurrent(0);
}

void DucklingPagerWidget::previous()
{
    if (m_current > 0)
        setCurrent(m_current - 1);
    else if (m_wrap)
        setCurrent(m_count - 1);
}

int DucklingPagerWidget::getCurrent() const
{
    return m_current;
}

void DucklingPagerWidget::setCurrent(int current)
{
    current = clampInt(current, 0, m_count - 1);
    if (current != m_current) {
        m_current = current;
        m_moving = true;
        emit currentChanged(current);
        update();
    }
}

int DucklingPagerWidget::getCount() const
{
    return m_count;
}

void DucklingPagerWidget::setCount(int count)
{
    m_count = clampInt(count, 2, 12);
    if (m_current >= m_count)
        setCurrent(m_count - 1);
    update();
}

int DucklingPagerWidget::getDucklings() const
{
    return m_ducklings;
}

void DucklingPagerWidget::setDucklings(int ducklings)
{
    m_ducklings = clampInt(ducklings, 0, 6);
    update();
}

bool DucklingPagerWidget::isWrap() const { return m_wrap; }
QColor DucklingPagerWidget::getDuckColor() const { return m_duckColor; }
QColor DucklingPagerWidget::getDucklingColor() const { return m_ducklingColor; }
QColor DucklingPagerWidget::getPadColor() const { return m_padColor; }

void DucklingPagerWidget::setWrap(bool wrap)
{
    m_wrap = wrap;
}

void DucklingPagerWidget::setDuckColor(const QColor &color)
{
    m_duckColor = color;
    update();
}

void DucklingPagerWidget::setDucklingColor(const QColor &color)
{
    m_ducklingColor = color;
    update();
}

void DucklingPagerWidget::setPadColor(const QColor &color)
{
    m_padColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize DucklingPagerWidget::sizeHint() const
{
    return QSize(360, 100);
}

QSize DucklingPagerWidget::minimumSizeHint() const
{
    return QSize(160, 70);
}

void DucklingPagerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void DucklingPagerWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void DucklingPagerWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_ducks.clear(); // re-line up for the new size
}

void DucklingPagerWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        qreal y, padR, size;
        const QList<qreal> xs = layoutPads(y, padR, size);
        const qreal ex = eventPos(e).x();
        int best = 0;
        for (int i = 1; i < xs.size(); ++i) {
            if (std::abs(xs.at(i) - ex) < std::abs(xs.at(best) - ex))
                best = i;
        }
        if (std::abs(xs.at(best) - ex) < std::max(padR * 1.6, 14.0))
            setCurrent(best);
    }
    QWidget::mouseReleaseEvent(e);
}

void DucklingPagerWidget::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() < 0)
        next();
    else
        previous();
    e->accept();
}

void DucklingPagerWidget::keyPressEvent(QKeyEvent *e)
{
    const int k = e->key();
    if (k == Qt::Key_Right || k == Qt::Key_Down)
        next();
    else if (k == Qt::Key_Left || k == Qt::Key_Up)
        previous();
    else if (k == Qt::Key_Home)
        setCurrent(0);
    else if (k == Qt::Key_End)
        setCurrent(m_count - 1);
    else
        QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void DucklingPagerWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    ensureDucks();
    qreal y, padR, size;
    const QList<qreal> xs = layoutPads(y, padR, size);
    Swimmer &mother = m_ducks[0];
    const qreal target = xs.at(m_current);
    // Mother swims with a capped speed (ducks do not teleport).
    const qreal maxV = size * 6.0;
    const qreal desired = clamp((target - mother.x) * 4.0, -maxV, maxV);
    mother.vx += (desired - mother.vx) * std::min(1.0, dt * 6.0);
    mother.x += mother.vx * dt;
    if (std::abs(mother.vx) > size * 0.3)
        m_facing = mother.vx > 0 ? 1.0 : -1.0;
    if (m_moving && std::abs(target - mother.x) < 1.5 && std::abs(mother.vx) < size * 0.5) {
        m_moving = false;
        m_quackAt = t;
    }
    // Ducklings follow the one in front of them.
    for (int k = 1; k < m_ducks.size(); ++k) {
        const Swimmer &front = m_ducks.at(k - 1);
        qreal tgt = front.x - m_facing * size * (k == 1 ? 0.85 : 0.6);
        tgt = clamp(tgt, size * 0.4, width() - size * 0.4); // huddle at the edges
        Swimmer &d = m_ducks[k];
        d.vx += ((tgt - d.x) * 30.0 - d.vx * 8.0) * dt;
        d.x += d.vx * dt;
    }
    update();
}

// ---------- Painting ----------
void DucklingPagerWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    ensureDucks();
    qreal y, padR, size;
    const QList<qreal> xs = layoutPads(y, padR, size);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Water ripples line.
    QColor wc(QStringLiteral("#4CC9F0"));
    wc.setAlpha(70);
    p.setPen(QPen(wc, 1.5));
    p.setBrush(Qt::NoBrush);
    QPainterPath wave;
    const qreal left = xs.first() - padR * 2.5;
    const qreal right = xs.last() + padR * 2.5;
    const int steps = 60;
    for (int k = 0; k <= steps; ++k) {
        const qreal x = left + (right - left) * k / qreal(steps);
        const qreal yy = y + padR * 0.55 + std::sin(x * 0.08 + tNow * 2.0) * 2.0;
        if (k == 0)
            wave.moveTo(x, yy);
        else
            wave.lineTo(x, yy);
    }
    p.drawPath(wave);
    // Lily pads.
    for (int i = 0; i < xs.size(); ++i) {
        const qreal x = xs.at(i);
        const bool cur = i == m_current;
        const qreal r = padR * (cur ? 1.15 : 1.0);
        const QColor pc = cur ? m_padColor.lighter(115) : m_padColor;
        QPainterPath pad;
        pad.moveTo(x, y);
        pad.arcTo(QRectF(x - r, y - r * 0.45, r * 2, r * 0.9), 20 + i * 37 % 60, 320);
        pad.closeSubpath();
        p.setPen(QPen(pc.darker(130), 1.0));
        p.setBrush(pc);
        p.drawPath(pad);
        if (cur) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(QStringLiteral("#FF8FB1")));
            p.drawEllipse(QPointF(x + r * 0.55, y - r * 0.1), r * 0.18, r * 0.12);
        }
    }
    // Ducklings first (behind mother), then mother.
    for (int k = int(m_ducks.size()) - 1; k >= 0; --k) {
        const Swimmer &d = m_ducks.at(k);
        const qreal s = k == 0 ? size : size * 0.55;
        const bool moving = std::abs(d.vx) > size * 0.25;
        const qreal ph = tNow * 14.0 + k * 1.3;
        const qreal bob = moving ? -std::abs(std::sin(ph)) * s * 0.08 : std::sin(tNow * 2.0 + k) * s * 0.02;
        const qreal rot = moving ? std::sin(ph) * 9.0 : 0.0;
        drawDuck(p, QPointF(d.x, y - s * 0.22 + bob), s, rot, k == 0);
    }
    // Quack.
    const qreal since = tNow - m_quackAt;
    if (since < 0.9) {
        QFont f(font());
        f.setBold(true);
        f.setPointSizeF(std::max(7.0, size * 0.25));
        p.setFont(f);
        QColor qc(palette().text().color());
        qc.setAlpha(int(255 * (1.0 - since / 0.9)));
        p.setPen(qc);
        const Swimmer &m = m_ducks.first();
        p.drawText(QRectF(m.x - 40, y - size * 1.25 - since * 10.0, 80, 18), Qt::AlignCenter, tr("Quack!"));
    }
    if (hasFocus()) {
        QColor fc(palette().text().color());
        fc.setAlpha(50);
        p.setPen(QPen(fc, 1.0, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(rect()).adjusted(2, 2, -2, -2), 10, 10);
    }
    p.end();
}

void DucklingPagerWidget::drawDuck(QPainter &p, const QPointF &c, qreal s, qreal rot, bool mother)
{
    const QColor col = mother ? m_duckColor : m_ducklingColor;
    p.save();
    p.translate(c);
    p.scale(m_facing, 1.0);
    p.rotate(rot);
    const QPen outline(col.darker(150), std::max(1.0, s * 0.03));
    p.setPen(outline);
    p.setBrush(col);
    // Body with tail.
    QPainterPath body;
    body.moveTo(-s * 0.55, -s * 0.28);
    body.quadTo(-s * 0.45, -s * 0.05, -s * 0.38, -s * 0.02);
    body.cubicTo(-s * 0.5, s * 0.3, s * 0.4, s * 0.32, s * 0.45, s * 0.02);
    body.cubicTo(s * 0.45, -s * 0.18, -s * 0.1, -s * 0.12, -s * 0.55, -s * 0.28);
    p.drawPath(body);
    p.setBrush(col.darker(108));
    p.drawEllipse(QPointF(-s * 0.04, s * 0.04), s * 0.2, s * 0.1);
    // Head.
    p.setBrush(col);
    const QPointF hc(s * 0.3, -s * 0.3);
    p.drawEllipse(hc, s * 0.2, s * 0.19);
    // Beak.
    p.setPen(QPen(QColor(QStringLiteral("#C76A12")), std::max(0.8, s * 0.02)));
    p.setBrush(QColor(QStringLiteral("#FF9F1C")));
    QPainterPath beak;
    beak.moveTo(hc.x() + s * 0.15, hc.y() - s * 0.02);
    beak.quadTo(hc.x() + s * 0.36, hc.y() + s * 0.0, hc.x() + s * 0.33, hc.y() + s * 0.06);
    beak.quadTo(hc.x() + s * 0.22, hc.y() + s * 0.09, hc.x() + s * 0.14, hc.y() + s * 0.06);
    beak.closeSubpath();
    p.drawPath(beak);
    // Eye.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#1E1E24")));
    p.drawEllipse(QPointF(hc.x() + s * 0.06, hc.y() - s * 0.05), s * 0.04, s * 0.045);
    if (mother) {
        // A little blue speculum and a bonnet-ish tuft for character.
        p.setBrush(QColor(QStringLiteral("#4C6FE0")));
        p.drawEllipse(QPointF(-s * 0.05, s * 0.0), s * 0.08, s * 0.035);
        p.setBrush(col.darker(115));
        p.drawEllipse(QPointF(hc.x() - s * 0.05, hc.y() - s * 0.19), s * 0.06, s * 0.035);
    }
    p.restore();
}
