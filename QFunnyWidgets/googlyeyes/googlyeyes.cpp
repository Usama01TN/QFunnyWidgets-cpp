/**
 * GooglyEyesWidget: googly eyes whose pupils follow the cursor with spring physics and gravity, bounce off the
 * eye wall, blink on their own, get dizzy when poked and angry when poked too often.
 */
#include "googlyeyes.h"

#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <utility>
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

/** Python's random.uniform(a, b). */
inline qreal uniform(qreal a, qreal b)
{
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}

} // namespace

GooglyEyesWidget::GooglyEyesWidget(QWidget *parent)
    : GooglyEyesWidget(Options(), parent)
{
}

GooglyEyesWidget::GooglyEyesWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_eyeCount(clampInt(options.eyeCount, 1, 6)),
      m_eyeColor(options.eyeColor),
      m_pupilColor(options.pupilColor),
      m_lidColor(options.lidColor),
      m_wobble(clamp(options.wobble, 0.0, 1.0)),
      m_gravity(std::max(0.0, qreal(options.gravity))),
      m_blinkInterval(std::max(300, options.blinkInterval)),
      m_autoBlink(options.autoBlink)
{
    if (options.poked)
        connect(this, &GooglyEyesWidget::poked, this, options.poked);
    if (options.gotAngry)
        connect(this, &GooglyEyesWidget::gotAngry, this, options.gotAngry);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    // State.
    for (int i = 0; i < m_eyeCount; ++i)
        m_pupils.append(PupilState());
    // Timing.
    m_clock.start();
    m_nextBlink = scheduleBlink(0.0);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GooglyEyesWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal GooglyEyesWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

qreal GooglyEyesWidget::scheduleBlink(qreal now) const
{
    const qreal base = m_blinkInterval / 1000.0;
    return now + uniform(base * 0.5, base * 1.5);
}

QList<QRectF> GooglyEyesWidget::layoutEyes() const
{
    const int n = m_eyeCount;
    const qreal w = width();
    const qreal h = height();
    const qreal gap = 0.18;
    qreal r = std::min(w / (n * 2.0 + (n - 1) * gap * 2.0 + 0.4), h / 2.5);
    r = std::max(4.0, r);
    const qreal total = n * 2.0 * r + (n - 1) * gap * 2.0 * r;
    const qreal x0 = (w - total) / 2.0;
    const qreal cy = h / 2.0 + r * 0.12;
    QList<QRectF> rects;
    for (int i = 0; i < n; ++i) {
        const qreal cx = x0 + r + i * (2.0 * r + gap * 2.0 * r);
        rects.append(QRectF(cx - r, cy - r, 2.0 * r, 2.0 * r));
    }
    return rects;
}

// ---------- Public API ----------
void GooglyEyesWidget::blink()
{
    m_blinkStart = now();
    update();
}

void GooglyEyesWidget::makeDizzy()
{
    m_dizzyUntil = now() + DIZZY_MS / 1000.0;
    update();
}

int GooglyEyesWidget::getPokeCount() const
{
    return m_pokeCount;
}

int GooglyEyesWidget::getEyeCount() const
{
    return m_eyeCount;
}

void GooglyEyesWidget::setEyeCount(int eyeCount)
{
    eyeCount = clampInt(eyeCount, 1, 6);
    if (eyeCount != m_eyeCount) {
        m_eyeCount = eyeCount;
        m_pupils.clear();
        for (int i = 0; i < eyeCount; ++i)
            m_pupils.append(PupilState());
        update();
    }
}

QColor GooglyEyesWidget::getEyeColor() const
{
    return m_eyeColor;
}

void GooglyEyesWidget::setEyeColor(const QColor &color)
{
    if (m_eyeColor != color) {
        m_eyeColor = color;
        update();
    }
}

QColor GooglyEyesWidget::getPupilColor() const
{
    return m_pupilColor;
}

void GooglyEyesWidget::setPupilColor(const QColor &color)
{
    if (m_pupilColor != color) {
        m_pupilColor = color;
        update();
    }
}

QColor GooglyEyesWidget::getLidColor() const
{
    return m_lidColor;
}

void GooglyEyesWidget::setLidColor(const QColor &color)
{
    if (m_lidColor != color) {
        m_lidColor = color;
        update();
    }
}

qreal GooglyEyesWidget::getWobble() const
{
    return m_wobble;
}

void GooglyEyesWidget::setWobble(qreal wobble)
{
    m_wobble = clamp(wobble, 0.0, 1.0);
}

qreal GooglyEyesWidget::getGravity() const
{
    return m_gravity;
}

void GooglyEyesWidget::setGravity(qreal gravity)
{
    m_gravity = std::max(0.0, gravity);
}

int GooglyEyesWidget::getBlinkInterval() const
{
    return m_blinkInterval;
}

void GooglyEyesWidget::setBlinkInterval(int blinkInterval)
{
    m_blinkInterval = std::max(300, blinkInterval);
    m_nextBlink = scheduleBlink(now());
}

bool GooglyEyesWidget::isAutoBlink() const
{
    return m_autoBlink;
}

void GooglyEyesWidget::setAutoBlink(bool autoBlink)
{
    m_autoBlink = autoBlink;
}

// ---------- Qt overrides ----------
QSize GooglyEyesWidget::sizeHint() const
{
    return QSize(260, 140);
}

QSize GooglyEyesWidget::minimumSizeHint() const
{
    return QSize(60, 30);
}

void GooglyEyesWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void GooglyEyesWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void GooglyEyesWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        const QPointF pos = eventPos(e);
        const bool hit = std::any_of(m_eyeRects.cbegin(), m_eyeRects.cend(),
                                     [&pos](const QRectF &r) { return r.contains(pos); });
        if (hit) {
            const qreal t = now();
            m_pokeCount += 1;
            m_squish = 1.0;
            QList<qreal> recent;
            for (qreal pt : std::as_const(m_pokeTimes)) {
                if (t - pt < 2.0)
                    recent.append(pt);
            }
            recent.append(t);
            m_pokeTimes = recent;
            makeDizzy();
            blink();
            if (m_pokeTimes.size() >= ANGRY_POKES && t > m_angryUntil) {
                m_angryUntil = t + 3.0;
                emit gotAngry();
            }
            emit poked(m_pokeCount);
        }
    }
    QWidget::mousePressEvent(e);
}

// ---------- Animation tick ----------
void GooglyEyesWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    if (m_eyeRects.isEmpty())
        m_eyeRects = layoutEyes();
    const QPoint cursor = mapFromGlobal(QCursor::pos());
    const bool dizzy = t < m_dizzyUntil;
    // Lower stiffness and damping = wobblier pupils.
    const qreal k = 260.0 - 190.0 * m_wobble;
    const qreal c = 18.0 - 14.0 * m_wobble;
    for (int i = 0; i < m_eyeRects.size(); ++i) {
        if (i >= m_pupils.size())
            break;
        const QRectF &rect = m_eyeRects.at(i);
        PupilState &ps = m_pupils[i];
        const qreal r = rect.width() / 2.0;
        const qreal maxR = r * 0.52;
        const qreal cx = rect.center().x();
        const qreal cy = rect.center().y();
        qreal tx, ty, kk;
        if (dizzy) {
            // Opposite spin directions per eye = maximum silliness.
            const qreal a = t * 14.0 * (i % 2 == 0 ? 1 : -1);
            tx = std::cos(a) * maxR * 0.85;
            ty = std::sin(a) * maxR * 0.85;
            kk = k * 3.0;
        } else {
            const qreal dx = cursor.x() - cx;
            const qreal dy = cursor.y() - cy;
            const qreal d = std::hypot(dx, dy);
            const qreal s = d > 1e-3 ? std::min(1.0, d / (r * 4.0)) * maxR / d : 0.0;
            tx = dx * s;
            ty = dy * s;
            kk = k;
        }
        const qreal ax = (tx - ps.x) * kk - ps.vx * c;
        const qreal ay = (ty - ps.y) * kk - ps.vy * c + m_gravity * 0.25;
        ps.vx += ax * dt;
        ps.vy += ay * dt;
        ps.x += ps.vx * dt;
        ps.y += ps.vy * dt;
        // Bounce off the eye wall like a real googly eye.
        const qreal dist = std::hypot(ps.x, ps.y);
        if (dist > maxR) {
            const qreal nx = ps.x / dist;
            const qreal ny = ps.y / dist;
            ps.x = nx * maxR;
            ps.y = ny * maxR;
            const qreal vn = ps.vx * nx + ps.vy * ny;
            if (vn > 0) {
                ps.vx -= 1.6 * vn * nx;
                ps.vy -= 1.6 * vn * ny;
            }
        }
    }
    // Blink scheduling.
    if (m_autoBlink && t >= m_nextBlink) {
        blink();
        m_nextBlink = scheduleBlink(t);
    }
    m_squish = std::max(0.0, m_squish - dt * 3.0);
    update();
}

// ---------- Painting ----------
qreal GooglyEyesWidget::blinkAmount(qreal now)
{
    if (m_blinkStart < 0)
        return 0.0;
    const qreal t = (now - m_blinkStart) / (BLINK_MS / 1000.0);
    if (t >= 2.0) {
        m_blinkStart = -1.0;
        return 0.0;
    }
    return t < 1.0 ? t : 2.0 - t;
}

void GooglyEyesWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_eyeRects = layoutEyes();
}

void GooglyEyesWidget::paintEvent(QPaintEvent *)
{
    if (m_eyeRects.isEmpty())
        m_eyeRects = layoutEyes();
    const qreal tNow = now();
    const qreal blinkAmt = blinkAmount(tNow);
    const bool angry = tNow < m_angryUntil;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const qreal squish = m_squish;
    for (int i = 0; i < m_eyeRects.size(); ++i) {
        const QRectF &rect = m_eyeRects.at(i);
        const qreal r = rect.width() / 2.0;
        const QPointF c = rect.center();
        p.save();
        // Squash & stretch when poked.
        p.translate(c);
        const qreal sx = 1.0 + 0.12 * squish * std::sin(squish * PI * 3.0);
        p.scale(sx, 2.0 - sx);
        if (angry)
            p.translate(uniform(-1.5, 1.5), uniform(-1.5, 1.5));
        // Drop shadow.
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 45));
        p.drawEllipse(QPointF(0, r * 0.08), r * 1.02, r * 1.02);
        // Eyeball with soft shading.
        QRadialGradient grad(QPointF(-r * 0.3, -r * 0.35), r * 1.6);
        grad.setColorAt(0.0, m_eyeColor);
        grad.setColorAt(0.75, m_eyeColor.darker(108));
        grad.setColorAt(1.0, m_eyeColor.darker(135));
        p.setBrush(QBrush(grad));
        QPen pen(QColor(30, 30, 40, 200));
        pen.setWidthF(std::max(1.5, r * 0.06));
        p.setPen(pen);
        p.drawEllipse(QPointF(0, 0), r, r);
        // Pupil.
        const PupilState ps = i < m_pupils.size() ? m_pupils.at(i) : PupilState();
        const qreal pr = r * 0.46;
        p.setPen(Qt::NoPen);
        p.setBrush(m_pupilColor);
        p.drawEllipse(QPointF(ps.x, ps.y), pr, pr);
        p.setBrush(QColor(255, 255, 255, 220));
        p.drawEllipse(QPointF(ps.x - pr * 0.35, ps.y - pr * 0.38), pr * 0.26, pr * 0.26);
        p.drawEllipse(QPointF(ps.x + pr * 0.3, ps.y + pr * 0.32), pr * 0.1, pr * 0.1);
        // Eyelid (blink + angry squint).
        const qreal lid = std::max(blinkAmt, angry ? 0.32 : 0.0);
        if (lid > 0.0) {
            QPainterPath clip;
            clip.addEllipse(QPointF(0, 0), r, r);
            p.setClipPath(clip);
            QPainterPath lidPath;
            const qreal edge = -r + 2.0 * r * lid;
            const qreal tilt = (angry && blinkAmt < 0.3) ? (i % 2 == 0 ? r * 0.35 : -r * 0.35) : 0.0;
            lidPath.moveTo(-r, -r - 1);
            lidPath.lineTo(r, -r - 1);
            lidPath.lineTo(r, edge - tilt);
            lidPath.quadTo(0, edge + r * 0.25, -r, edge + tilt);
            lidPath.closeSubpath();
            p.setBrush(m_lidColor);
            QPen lp(m_lidColor.darker(140));
            lp.setWidthF(std::max(1.0, r * 0.05));
            p.setPen(lp);
            p.drawPath(lidPath);
            p.setClipping(false);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPointF(0, 0), r, r);
        }
        // Angry eyebrows.
        if (angry) {
            QPen bp(QColor(40, 30, 30));
            bp.setWidthF(std::max(2.0, r * 0.16));
            bp.setCapStyle(Qt::RoundCap);
            p.setPen(bp);
            const int d = i % 2 == 0 ? 1 : -1;
            p.drawLine(QPointF(-r * 0.7 * d, -r * 1.35), QPointF(r * 0.6 * d, -r * 1.05));
        }
        p.restore();
    }
    // Dizzy stars orbiting above.
    if (tNow < m_dizzyUntil && !m_eyeRects.isEmpty()) {
        qreal top = m_eyeRects.first().top();
        for (const QRectF &rc : std::as_const(m_eyeRects))
            top = std::min(top, rc.top());
        const qreal cx = width() / 2.0;
        const qreal spread = (m_eyeRects.last().right() - m_eyeRects.first().left()) * 0.35;
        p.setPen(Qt::NoPen);
        for (int k = 0; k < 3; ++k) {
            const qreal a = tNow * 5.0 + k * 2.0 * PI / 3.0;
            const qreal sx = cx + std::cos(a) * spread;
            const qreal sy = top - 6.0 + std::sin(a) * 6.0;
            drawStar(p, QPointF(sx, sy), 7.0 + 2.0 * std::sin(tNow * 9.0 + k), QColor(QStringLiteral("#FFD23F")));
        }
    }
    p.end();
}

void GooglyEyesWidget::drawStar(QPainter &p, const QPointF &c, qreal r, const QColor &color)
{
    QPainterPath path;
    for (int j = 0; j < 10; ++j) {
        const qreal rr = j % 2 == 0 ? r : r * 0.45;
        const qreal a = -PI / 2.0 + j * PI / 5.0;
        const QPointF pt(c.x() + std::cos(a) * rr, c.y() + std::sin(a) * rr);
        if (j == 0)
            path.moveTo(pt);
        else
            path.lineTo(pt);
    }
    path.closeSubpath();
    p.setBrush(color);
    p.drawPath(path);
}
