/**
 * HeartLikeButtonWidget: a like button whose heart has feelings. Liking it pops it with a burst ring and mini
 * hearts, then it beats happily; un-liking cracks it in two (with tears) and leaves a bandage for a while;
 * toggling too fast makes it dizzy. The count rolls like an odometer.
 */
#include "heartlike.h"

#include <QFontMetricsF>
#include <QKeyEvent>
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

QPainterPath HeartLikeButtonWidget::heartPath(const QPointF &c, qreal s)
{
    const qreal x = c.x();
    const qreal y = c.y();
    QPainterPath path;
    path.moveTo(x, y + s * 0.95);
    path.cubicTo(x - s * 1.55, y + s * 0.05, x - s * 0.95, y - s * 1.15, x, y - s * 0.45);
    path.cubicTo(x + s * 0.95, y - s * 1.15, x + s * 1.55, y + s * 0.05, x, y + s * 0.95);
    return path;
}

HeartLikeButtonWidget::HeartLikeButtonWidget(QWidget *parent)
    : HeartLikeButtonWidget(Options(), parent)
{
}

HeartLikeButtonWidget::HeartLikeButtonWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_liked(options.liked),
      m_count(std::max(0, options.count)),
      m_color(options.color),
      m_showCount(options.showCount)
{
    if (options.likedChanged)
        connect(this, &HeartLikeButtonWidget::likedChanged, this, options.likedChanged);
    if (options.countChanged)
        connect(this, &HeartLikeButtonWidget::countChanged, this, options.countChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    // State.
    m_oldCount = m_count;
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HeartLikeButtonWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal HeartLikeButtonWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void HeartLikeButtonWidget::heartGeom(QPointF &c, qreal &s) const
{
    const qreal h = height();
    const qreal w = width();
    s = clamp(std::min(h * 0.3, w * 0.22), 10.0, 60.0);
    const qreal cx = m_showCount ? s * 1.6 + 4.0 : w / 2.0;
    c = QPointF(cx, h / 2.0 + s * 0.05);
}

void HeartLikeButtonWidget::changeCount(int count)
{
    count = std::max(0, count);
    if (count != m_count) {
        m_oldCount = m_count;
        m_count = count;
        m_countAt = now();
        emit countChanged(count);
    }
}

// ---------- Public API ----------
void HeartLikeButtonWidget::toggle()
{
    setLiked(!m_liked);
}

bool HeartLikeButtonWidget::isLiked() const
{
    return m_liked;
}

void HeartLikeButtonWidget::setLiked(bool liked)
{
    if (liked == m_liked)
        return;
    const qreal t = now();
    m_liked = liked;
    QList<qreal> recent;
    for (qreal x : std::as_const(m_toggles)) {
        if (t - x < 2.0)
            recent.append(x);
    }
    recent.append(t);
    m_toggles = recent;
    if (m_toggles.size() >= 4)
        m_dizzyUntil = t + 2.0;
    QPointF c;
    qreal s;
    heartGeom(c, s);
    if (liked) {
        m_popVel += 14.0;
        m_ringAt = t;
        for (int k = 0; k < 9; ++k) {
            const qreal a = -PI / 2.0 + (k - 4) * 0.36 + uniform(-0.1, 0.1);
            const qreal sp = uniform(90, 160);
            const qreal life = uniform(0.6, 0.9);
            const qreal size = s * uniform(0.12, 0.2);
            m_minis.append(MiniHeart(c.x(), c.y() - s * 0.2, std::cos(a) * sp, std::sin(a) * sp, life, size));
        }
        changeCount(m_count + 1);
    } else {
        m_unlikedAt = t;
        m_popVel -= 6.0;
        changeCount(m_count - 1);
    }
    emit likedChanged(liked);
    update();
}

int HeartLikeButtonWidget::getCount() const { return m_count; }
QColor HeartLikeButtonWidget::getColor() const { return m_color; }
bool HeartLikeButtonWidget::isShowCount() const { return m_showCount; }

void HeartLikeButtonWidget::setCount(int count)
{
    changeCount(count);
}

void HeartLikeButtonWidget::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void HeartLikeButtonWidget::setShowCount(bool showCount)
{
    m_showCount = showCount;
    update();
}

// ---------- Qt overrides ----------
QSize HeartLikeButtonWidget::sizeHint() const
{
    return QSize(200, 120);
}

QSize HeartLikeButtonWidget::minimumSizeHint() const
{
    return QSize(60, 50);
}

void HeartLikeButtonWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void HeartLikeButtonWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void HeartLikeButtonWidget::enterEvent(QEnterEvent *e)
#else
void HeartLikeButtonWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void HeartLikeButtonWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void HeartLikeButtonWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(eventPos(e)))
        toggle();
    QWidget::mouseReleaseEvent(e);
}

void HeartLikeButtonWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        toggle();
        return;
    }
    QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void HeartLikeButtonWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    m_popVel += ((1.0 - m_pop) * 340.0 - m_popVel * 11.0) * dt;
    m_pop += m_popVel * dt;
    QList<MiniHeart> alive;
    for (MiniHeart m : std::as_const(m_minis)) {
        m.vy += 220.0 * dt;
        m.vx *= 0.98;
        m.x += m.vx * dt;
        m.y += m.vy * dt;
        m.life -= dt;
        if (m.life > 0)
            alive.append(m);
    }
    m_minis = alive;
    update();
}

// ---------- Painting ----------
void HeartLikeButtonWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF c;
    qreal s;
    heartGeom(c, s);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const bool liked = m_liked;
    const bool dizzy = tNow < m_dizzyUntil;
    const qreal sinceUnlike = tNow - m_unlikedAt;
    const bool cracking = !liked && sinceUnlike < 0.9;
    // Burst ring.
    const qreal rs = tNow - m_ringAt;
    if (rs < 0.45) {
        QColor rc(m_color);
        rc.setAlpha(int(200 * (1.0 - rs / 0.45)));
        p.setPen(QPen(rc, std::max(1.5, s * 0.08 * (1.0 - rs / 0.45))));
        p.setBrush(Qt::NoBrush);
        const qreal rr = s * (0.6 + 1.2 * rs / 0.45);
        p.drawEllipse(c, rr, rr);
    }
    // Heartbeat (lub-dub) when liked.
    qreal beat = 1.0;
    if (liked) {
        const qreal ph = pyMod(tNow, 1.3);
        beat += ph < 0.15 ? 0.07 * std::max(0.0, std::sin(ph / 0.15 * PI)) : 0.0;
        beat += (0.22 <= ph && ph < 0.37) ? 0.05 * std::max(0.0, std::sin((ph - 0.22) / 0.15 * PI)) : 0.0;
    }
    const qreal k = std::max(0.3, m_pop) * beat;
    if (dizzy) {
        p.save();
        p.translate(c);
        p.rotate(std::sin(tNow * 9.0) * 12.0);
        p.translate(-c);
    }
    p.save();
    p.translate(c);
    p.scale(k, k);
    p.translate(-c);
    if (cracking)
        drawCracked(p, c, s, sinceUnlike);
    else
        drawHeart(p, c, s, liked);
    drawFace(p, c, s, liked, dizzy, cracking, tNow);
    p.restore();
    if (dizzy)
        p.restore();
    // Bandage after a break-up (fades away).
    if (!liked && 0.9 <= sinceUnlike && sinceUnlike < 4.5) {
        const qreal a = clamp((4.5 - sinceUnlike) / 1.0, 0.0, 1.0);
        p.save();
        p.setOpacity(a);
        p.translate(c.x() + s * 0.45, c.y() - s * 0.35);
        p.rotate(35);
        p.setPen(QPen(QColor(QStringLiteral("#C9A27E")), 1.0));
        p.setBrush(QColor(QStringLiteral("#F2D2B0")));
        p.drawRoundedRect(QRectF(-s * 0.32, -s * 0.11, s * 0.64, s * 0.22), s * 0.1, s * 0.1);
        p.setBrush(QColor(QStringLiteral("#E8C09A")));
        p.drawRect(QRectF(-s * 0.1, -s * 0.11, s * 0.2, s * 0.22));
        p.restore();
    }
    // Mini hearts.
    p.setPen(Qt::NoPen);
    for (const MiniHeart &m : std::as_const(m_minis)) {
        QColor mc(m_color);
        mc.setAlpha(int(255 * clamp(m.life / m.maxLife, 0.0, 1.0)));
        p.fillPath(heartPath(QPointF(m.x, m.y), m.size), mc);
    }
    // Dizzy bubble.
    if (dizzy) {
        QFont f(font());
        f.setBold(true);
        f.setPointSizeF(8.0);
        const QFontMetricsF fm(f);
        const QString text = tr("Make up your mind!");
        const qreal tw = fm.horizontalAdvance(text);
        const qreal bw = tw + 14.0;
        const qreal bh = fm.height() + 6.0;
        const qreal bx = clamp(c.x() - bw / 2.0, 2.0, width() - bw - 2.0);
        const qreal by = std::max(2.0, c.y() - s * 1.25 - bh);
        p.setPen(QPen(QColor(20, 20, 30, 60), 1.0));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawRoundedRect(QRectF(bx, by, bw, bh), bh / 2.0, bh / 2.0);
        p.setFont(f);
        p.setPen(QColor(QStringLiteral("#22222A")));
        p.drawText(QRectF(bx, by, bw, bh), Qt::AlignCenter, text);
    }
    // Odometer count.
    if (m_showCount)
        drawCount(p, QRectF(c.x() + s * 1.45, c.y() - 16.0, width() - c.x() - s * 1.45, 32.0), tNow);
    p.end();
}

void HeartLikeButtonWidget::drawHeart(QPainter &p, const QPointF &c, qreal s, bool liked)
{
    const QPainterPath path = heartPath(c, s);
    const QColor hc = m_color;
    if (liked) {
        QRadialGradient grad(QPointF(c.x() - s * 0.4, c.y() - s * 0.4), s * 1.8);
        grad.setColorAt(0.0, hc.lighter(125));
        grad.setColorAt(1.0, hc.darker(115));
        p.setPen(QPen(hc.darker(140), std::max(1.2, s * 0.05)));
        p.setBrush(grad);
        p.drawPath(path);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 90));
        p.drawEllipse(QPointF(c.x() - s * 0.5, c.y() - s * 0.35), s * 0.18, s * 0.11);
    } else {
        const QColor tc(palette().text().color());
        QColor line = m_hovered ? QColor(hc) : QColor(tc);
        line.setAlpha(m_hovered ? 220 : 130);
        QColor fill(hc);
        fill.setAlpha(m_hovered ? 35 : 12);
        p.setPen(QPen(line, std::max(1.8, s * 0.08)));
        p.setBrush(fill);
        p.drawPath(path);
    }
}

void HeartLikeButtonWidget::drawCracked(QPainter &p, const QPointF &c, qreal s, qreal since)
{
    const qreal gap = std::sin(clamp(since / 0.9, 0.0, 1.0) * PI) * s * 0.18;
    QPainterPath crack;
    crack.moveTo(c.x(), c.y() - s * 0.45);
    static const qreal pts[5][2] = {{-0.12, -0.2}, {0.1, 0.05}, {-0.1, 0.3}, {0.06, 0.55}, {0.0, 0.95}};
    for (const auto &pt : pts)
        crack.lineTo(c.x() + pt[0] * s, c.y() + pt[1] * s);
    const QPainterPath heart = heartPath(c, s);
    const QColor hc(m_color);
    for (qreal d : {-1.0, 1.0}) {
        QPainterPath half;
        half.moveTo(c.x() + d * s * 2.0, c.y() - s * 2.0);
        half.lineTo(c.x(), c.y() - s * 2.0);
        half.connectPath(crack);
        half.lineTo(c.x(), c.y() + s * 2.0);
        half.lineTo(c.x() + d * s * 2.0, c.y() + s * 2.0);
        half.closeSubpath();
        const QPainterPath piece = heart.intersected(half);
        p.save();
        p.translate(d * gap, 0);
        p.rotate(d * gap * 0.4);
        p.setPen(QPen(hc.darker(140), std::max(1.2, s * 0.05)));
        p.setBrush(hc.darker(105));
        p.drawPath(piece);
        p.restore();
    }
}

void HeartLikeButtonWidget::drawFace(QPainter &p, const QPointF &c, qreal s, bool liked, bool dizzy, bool cracking,
                                     qreal now)
{
    QColor ink = (liked || cracking) ? QColor(QStringLiteral("#3A1020")) : QColor(palette().text().color());
    if (!(liked || cracking))
        ink.setAlpha(170);
    const QPen pen(ink, std::max(1.2, s * 0.07), Qt::SolidLine, Qt::RoundCap);
    const qreal ex = s * 0.32;
    const qreal ey = c.y() - s * 0.08;
    if (dizzy) {
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        for (qreal d : {-1.0, 1.0}) {
            QPainterPath spiral;
            for (int k = 0; k < 24; ++k) {
                const qreal a = k * 0.55 + now * 10.0 * d;
                const qreal r = s * 0.006 * k;
                const QPointF pt(c.x() + d * ex + std::cos(a) * r, ey + std::sin(a) * r);
                if (k == 0)
                    spiral.moveTo(pt);
                else
                    spiral.lineTo(pt);
            }
            p.drawPath(spiral);
        }
        p.drawEllipse(QPointF(c.x(), c.y() + s * 0.3), s * 0.07, s * 0.08);
        return;
    }
    if (cracking) {
        const qreal gap = std::sin(clamp((now - m_unlikedAt) / 0.9, 0.0, 1.0) * PI) * s * 0.18;
        p.setPen(pen);
        for (qreal d : {-1.0, 1.0}) {
            const qreal x = c.x() + d * ex + d * gap;
            p.drawLine(QPointF(x - s * 0.1, ey), QPointF(x + s * 0.1, ey));
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(QStringLiteral("#5AB9FF")));
            p.drawEllipse(QPointF(x + d * s * 0.04, ey + s * 0.12 + pyMod(now, 0.6) * s * 0.4), s * 0.05, s * 0.07);
            p.setPen(pen);
        }
        return;
    }
    if (liked) {
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(c.x() + d * ex - s * 0.11, ey - s * 0.05, s * 0.22, s * 0.16), 20 * 16, 140 * 16);
        QColor bl(QStringLiteral("#FFFFFF"));
        bl.setAlpha(70);
        p.setPen(Qt::NoPen);
        p.setBrush(bl);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(c.x() + d * s * 0.5, c.y() + s * 0.12), s * 0.11, s * 0.07);
        p.setPen(pen);
        p.setBrush(QColor(QStringLiteral("#7A1F35")));
        QPainterPath m;
        m.moveTo(c.x() - s * 0.18, c.y() + s * 0.15);
        m.quadTo(c.x(), c.y() + s * 0.45, c.x() + s * 0.18, c.y() + s * 0.15);
        m.closeSubpath();
        p.drawPath(m);
    } else {
        // Hopeful: looks up at you when hovered.
        const qreal look = m_hovered ? -s * 0.04 : 0.0;
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(c.x() + d * ex, ey + look), s * 0.06, s * (m_hovered ? 0.09 : 0.07));
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        if (m_hovered)
            p.drawArc(QRectF(c.x() - s * 0.1, c.y() + s * 0.12, s * 0.2, s * 0.14), 200 * 16, 140 * 16);
        else
            p.drawLine(QPointF(c.x() - s * 0.08, c.y() + s * 0.22), QPointF(c.x() + s * 0.08, c.y() + s * 0.22));
    }
}

void HeartLikeButtonWidget::drawCount(QPainter &p, const QRectF &area, qreal now)
{
    if (area.width() < 20)
        return;
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(13.0);
    p.setFont(f);
    const QColor tc(palette().text().color());
    const qreal t = clamp((now - m_countAt) / 0.3, 0.0, 1.0);
    const bool up = m_count > m_oldCount;
    p.save();
    p.setClipRect(area);
    if (t < 1.0) {
        const qreal dy = area.height() * t;
        QColor old(tc);
        old.setAlpha(int(255 * (1.0 - t)));
        p.setPen(old);
        p.drawText(area.translated(0, up ? -dy : dy), Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_oldCount));
        p.setPen(up ? m_color : tc);
        p.drawText(area.translated(0, up ? area.height() - dy : dy - area.height()), Qt::AlignLeft | Qt::AlignVCenter,
                   QString::number(m_count));
    } else {
        p.setPen(m_liked ? m_color : tc);
        p.drawText(area, Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_count));
    }
    p.restore();
}
