/**
 * RubberDuckWidget: a rubber-duck-debugging companion floating on animated water. Click it to squeak (beak opens,
 * eye squints) and receive dubious programming advice; tell it your problem with listen() and it nods while
 * thinking before answering.
 */
#include "rubberduck.h"

#include <QFontMetricsF>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QTimer>
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

QStringList RubberDuckWidget::defaultAdvice()
{
    return {QStringLiteral("Have you tried turning it off and on again?"),
            QStringLiteral("Read the error message. Slowly. Out loud."),
            QStringLiteral("It's always DNS."),
            QStringLiteral("Did you save the file?"),
            QStringLiteral("Works on my machine. Ship my machine."),
            QStringLiteral("Explain it to me line by line."),
            QStringLiteral("Is it plugged in?"),
            QStringLiteral("Check for a missing comma."),
            QStringLiteral("Clear the cache. Then clear it again."),
            QStringLiteral("Take a walk. The bug will miss you."),
            QStringLiteral("Off by one? Or off by two?"),
            QStringLiteral("Delete it and see who complains."),
            QStringLiteral("Add a print statement. Add ten."),
            QStringLiteral("Blame the intern. Wait, you ARE the intern.")};
}

RubberDuckWidget::RubberDuckWidget(QWidget *parent)
    : RubberDuckWidget(Options(), parent)
{
}

RubberDuckWidget::RubberDuckWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_duckColor(options.duckColor),
      m_beakColor(options.beakColor),
      m_waterColor(options.waterColor),
      m_advice(options.advice.isEmpty() ? defaultAdvice() : options.advice),
      m_adviceOnClick(options.adviceOnClick),
      m_adviceDuration(std::max(500, options.adviceDuration)),
      m_squeakText(options.squeakText)
{
    if (options.squeaked)
        connect(this, &RubberDuckWidget::squeaked, this, options.squeaked);
    if (options.adviceGiven)
        connect(this, &RubberDuckWidget::adviceGiven, this, options.adviceGiven);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &RubberDuckWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal RubberDuckWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void RubberDuckWidget::geometry(QPointF &c, qreal &s, qreal &water) const
{
    const qreal w = width();
    const qreal h = height();
    s = std::max(20.0, std::min(w * 0.42, h * 0.42));
    water = h - s * 0.28;
    c = QPointF(w * 0.5, water - s * 0.18);
}

QString RubberDuckWidget::pickAdvice()
{
    QStringList options;
    for (const QString &a : std::as_const(m_advice)) {
        if (a != m_lastAdvice)
            options.append(a);
    }
    if (options.isEmpty())
        options = m_advice;
    m_lastAdvice = options.at(int(QRandomGenerator::global()->bounded(int(options.size()))));
    return m_lastAdvice;
}

void RubberDuckWidget::say(const QString &text, qreal now)
{
    m_bubbleText = text;
    m_bubbleShownAt = now;
    m_bubbleUntil = now + m_adviceDuration / 1000.0;
    emit adviceGiven(text);
}

// ---------- Public API ----------
void RubberDuckWidget::squeak()
{
    const qreal t = now();
    m_squeaks += 1;
    m_squeakAt = t;
    m_squashVel -= 9.0;
    emit squeaked(m_squeaks);
    if (m_adviceOnClick && t >= m_thinkUntil)
        say(pickAdvice(), t);
}

void RubberDuckWidget::listen(const QString &problem)
{
    Q_UNUSED(problem)
    const qreal t = now();
    m_thinkUntil = t + 1.6;
    m_bubbleText.clear();
    m_bubbleShownAt = t;
    m_bubbleUntil = m_thinkUntil;
}

int RubberDuckWidget::getSqueaks() const
{
    return m_squeaks;
}

QColor RubberDuckWidget::getDuckColor() const
{
    return m_duckColor;
}

void RubberDuckWidget::setDuckColor(const QColor &color)
{
    if (m_duckColor != color) {
        m_duckColor = color;
        update();
    }
}

QColor RubberDuckWidget::getBeakColor() const
{
    return m_beakColor;
}

void RubberDuckWidget::setBeakColor(const QColor &color)
{
    if (m_beakColor != color) {
        m_beakColor = color;
        update();
    }
}

QColor RubberDuckWidget::getWaterColor() const
{
    return m_waterColor;
}

void RubberDuckWidget::setWaterColor(const QColor &color)
{
    if (m_waterColor != color) {
        m_waterColor = color;
        update();
    }
}

QStringList RubberDuckWidget::getAdvice() const
{
    return m_advice;
}

void RubberDuckWidget::setAdvice(const QStringList &advice)
{
    m_advice = advice.isEmpty() ? defaultAdvice() : advice;
    update();
}

bool RubberDuckWidget::isAdviceOnClick() const
{
    return m_adviceOnClick;
}

void RubberDuckWidget::setAdviceOnClick(bool adviceOnClick)
{
    m_adviceOnClick = adviceOnClick;
    update();
}

int RubberDuckWidget::getAdviceDuration() const
{
    return m_adviceDuration;
}

void RubberDuckWidget::setAdviceDuration(int adviceDuration)
{
    m_adviceDuration = std::max(500, adviceDuration);
}

QString RubberDuckWidget::getSqueakText() const
{
    return m_squeakText;
}

void RubberDuckWidget::setSqueakText(const QString &squeakText)
{
    m_squeakText = squeakText;
}

// ---------- Qt overrides ----------
QSize RubberDuckWidget::sizeHint() const
{
    return QSize(320, 260);
}

QSize RubberDuckWidget::minimumSizeHint() const
{
    return QSize(160, 140);
}

void RubberDuckWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void RubberDuckWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void RubberDuckWidget::mousePressEvent(QMouseEvent *e)
{
    QPointF c;
    qreal s, water;
    geometry(c, s, water);
    const QPointF pos = eventPos(e);
    if (e->button() == Qt::LeftButton && std::abs(pos.x() - c.x()) < s * 0.7 && std::abs(pos.y() - c.y()) < s * 0.7)
        squeak();
    QWidget::mousePressEvent(e);
}

// ---------- Animation tick ----------
void RubberDuckWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    m_squashVel += (-m_squash * 300.0 - m_squashVel * 9.0) * dt;
    m_squash += m_squashVel * dt;
    if (t >= m_nextLook) {
        static const qreal looks[] = {-1.0, -0.4, 0.0, 0.6, 1.0};
        m_lookTarget = looks[QRandomGenerator::global()->bounded(5)];
        m_nextLook = t + uniform(1.2, 3.5);
    }
    if (t >= m_blinkAt + 0.15)
        m_blinkAt = t + uniform(2.0, 5.0);
    m_lookX += (m_lookTarget - m_lookX) * std::min(1.0, dt * 8.0);
    // Finished thinking: deliver wisdom.
    if (m_thinkUntil > 0 && t >= m_thinkUntil) {
        m_thinkUntil = 0.0;
        say(pickAdvice(), t);
    }
    update();
}

// ---------- Painting ----------
void RubberDuckWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF c;
    qreal s, water;
    geometry(c, s, water);
    const qreal w = width();
    const qreal h = height();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const bool thinking = tNow < m_thinkUntil;
    const qreal bob = std::sin(tNow * 2.2) * s * 0.03;
    qreal rock = std::sin(tNow * 1.7) * 4.0;
    if (thinking)
        rock += std::sin(tNow * 9.0) * 6.0; // nodding
    // Duck.
    p.save();
    p.translate(c.x(), c.y() + bob);
    p.rotate(rock);
    const qreal sq = clamp(m_squash, -0.35, 0.35);
    p.translate(0, s * 0.3);
    p.scale(1.0 - sq * 0.6, 1.0 + sq);
    p.translate(0, -s * 0.3);
    drawDuck(p, s, tNow, thinking);
    p.restore();
    // Water in front of the duck's bottom.
    drawWater(p, w, h, water, s, tNow);
    // Squeak text.
    const qreal since = tNow - m_squeakAt;
    if (since < 0.7) {
        QFont f(font());
        f.setBold(true);
        f.setPointSizeF(std::max(7.0, s * 0.1) * (1.0 + 0.4 * clamp(since * 6.0, 0.0, 1.0)));
        p.setFont(f);
        QColor col(QStringLiteral("#FF8C1A"));
        col.setAlpha(int(255 * (1.0 - since / 0.7)));
        p.setPen(col);
        p.save();
        p.translate(c.x() - s * 0.75, c.y() - s * 0.55 - since * s * 0.3);
        p.rotate(-12.0);
        p.drawText(QPointF(0, 0), m_squeakText);
        p.restore();
    }
    // Thought bubble.
    if (tNow < m_bubbleUntil)
        drawBubble(p, c, s, tNow, thinking);
    p.end();
}

void RubberDuckWidget::drawDuck(QPainter &p, qreal s, qreal now, bool thinking)
{
    const QColor dc = m_duckColor;
    const QPen outline(dc.darker(150), std::max(1.2, s * 0.012));
    // Body with a perky tail.
    QPainterPath body;
    body.moveTo(-s * 0.55, -s * 0.12);
    body.quadTo(-s * 0.5, s * 0.02, -s * 0.38, s * 0.04);
    body.cubicTo(-s * 0.5, s * 0.38, s * 0.42, s * 0.42, s * 0.48, s * 0.06);
    body.cubicTo(s * 0.5, -s * 0.1, s * 0.2, -s * 0.12, -s * 0.05, -s * 0.06);
    body.cubicTo(-s * 0.25, -s * 0.02, -s * 0.38, -s * 0.06, -s * 0.55, -s * 0.12);
    QRadialGradient grad(QPointF(-s * 0.05, -s * 0.05), s * 0.7);
    grad.setColorAt(0.0, dc.lighter(115));
    grad.setColorAt(1.0, dc.darker(112));
    p.setPen(outline);
    p.setBrush(grad);
    p.drawPath(body);
    // Wing.
    QPainterPath wing;
    const qreal flap = now - m_squeakAt < 0.5 ? std::sin(now * 30.0) * s * 0.03 : 0.0;
    wing.moveTo(-s * 0.22, s * 0.06);
    wing.cubicTo(-s * 0.12, -s * 0.06 - flap, s * 0.2, -s * 0.02 - flap, s * 0.16, s * 0.12);
    wing.quadTo(-s * 0.02, s * 0.2, -s * 0.22, s * 0.06);
    p.setBrush(dc.darker(106));
    p.drawPath(wing);
    // Head.
    const qreal hx = s * 0.22;
    const qreal hy = -s * 0.32;
    const qreal hr = s * 0.24;
    QRadialGradient hg(QPointF(hx - hr * 0.3, hy - hr * 0.35), hr * 1.6);
    hg.setColorAt(0.0, dc.lighter(118));
    hg.setColorAt(1.0, dc.darker(110));
    p.setBrush(hg);
    p.drawEllipse(QPointF(hx, hy), hr, hr);
    // Beak (opens on squeak).
    const qreal open = clamp(1.0 - (now - m_squeakAt) / 0.4, 0.0, 1.0);
    const QColor bc = m_beakColor;
    p.setPen(QPen(bc.darker(140), std::max(1.0, s * 0.01)));
    p.setBrush(bc);
    const qreal bx = hx + hr * 0.75;
    const qreal by = hy + hr * 0.25;
    QPainterPath upper;
    upper.moveTo(bx, by - s * 0.05);
    upper.quadTo(bx + s * 0.2, by - s * 0.05 - open * s * 0.05, bx + s * 0.22, by - open * s * 0.05);
    upper.quadTo(bx + s * 0.1, by + s * 0.01 - open * s * 0.04, bx, by + s * 0.01);
    upper.closeSubpath();
    QPainterPath lower;
    lower.moveTo(bx, by + s * 0.01);
    lower.quadTo(bx + s * 0.1, by + s * 0.02 + open * s * 0.04, bx + s * 0.18, by + s * 0.03 + open * s * 0.06);
    lower.quadTo(bx + s * 0.08, by + s * 0.07 + open * s * 0.05, bx, by + s * 0.05);
    lower.closeSubpath();
    p.drawPath(lower);
    p.drawPath(upper);
    // Eye: blinks, looks around, squints when squeezed, looks up while thinking.
    const qreal ex = hx + hr * 0.3;
    const qreal ey = hy - hr * 0.15;
    const qreal er = hr * 0.24;
    const QColor ink(QStringLiteral("#1E1E24"));
    const bool blinking = m_blinkAt <= now && now < m_blinkAt + 0.15;
    if (blinking || open > 0.3) {
        p.setPen(QPen(ink, std::max(1.2, s * 0.015), Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        if (open > 0.3) {
            p.drawLine(QPointF(ex - er, ey - er * 0.5), QPointF(ex + er * 0.6, ey));
            p.drawLine(QPointF(ex + er * 0.6, ey), QPointF(ex - er, ey + er * 0.5));
        } else {
            p.drawArc(QRectF(ex - er, ey - er * 0.5, er * 2.0, er), 180 * 16, 180 * 16);
        }
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(ex, ey), er, er * 1.1);
        const qreal lx = thinking ? 0.0 : m_lookX * er * 0.35;
        const qreal ly = thinking ? -er * 0.4 : 0.0;
        p.setBrush(ink);
        p.drawEllipse(QPointF(ex + lx, ey + ly), er * 0.6, er * 0.65);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(ex + lx - er * 0.2, ey + ly - er * 0.25), er * 0.2, er * 0.2);
    }
    // Blush.
    QColor blush(QStringLiteral("#FF8FA3"));
    blush.setAlpha(120);
    p.setPen(Qt::NoPen);
    p.setBrush(blush);
    p.drawEllipse(QPointF(hx + hr * 0.25, hy + hr * 0.45), hr * 0.2, hr * 0.12);
}

void RubberDuckWidget::drawWater(QPainter &p, qreal w, qreal h, qreal water, qreal s, qreal now)
{
    const QColor wc(m_waterColor);
    QPainterPath path;
    path.moveTo(0, h);
    const int steps = 40;
    for (int k = 0; k <= steps; ++k) {
        const qreal x = w * k / qreal(steps);
        const qreal y = water + std::sin(x * 0.04 + now * 2.5) * s * 0.025 + std::sin(x * 0.09 - now * 1.7) * s * 0.012;
        path.lineTo(x, y);
    }
    path.lineTo(w, h);
    path.closeSubpath();
    QLinearGradient grad(QPointF(0, water), QPointF(0, h));
    QColor top(wc);
    top.setAlpha(200);
    QColor bottom(wc.darker(140));
    bottom.setAlpha(230);
    grad.setColorAt(0.0, top);
    grad.setColorAt(1.0, bottom);
    p.setPen(QPen(wc.lighter(130), std::max(1.5, s * 0.015)));
    p.setBrush(grad);
    p.drawPath(path);
    // Ripples around the duck.
    QPointF c;
    qreal s2, wl;
    geometry(c, s2, wl);
    p.setBrush(Qt::NoBrush);
    for (int k = 0; k < 2; ++k) {
        const qreal t = pyMod(now * 0.5 + k * 0.5, 1.0);
        QColor rc(QStringLiteral("#FFFFFF"));
        rc.setAlpha(int(110 * (1.0 - t)));
        p.setPen(QPen(rc, std::max(1.0, s * 0.01)));
        const qreal rw = s * (0.55 + 0.45 * t);
        p.drawEllipse(QPointF(c.x(), water + s * 0.02), rw, rw * 0.12);
    }
}

void RubberDuckWidget::drawBubble(QPainter &p, const QPointF &c, qreal s, qreal now, bool thinking)
{
    QFont f(font());
    f.setPointSizeF(clamp(s * 0.075, 8.0, 11.0));
    const QFontMetricsF fm(f);
    const qreal maxW = std::min(width() - 16.0, std::max(140.0, s * 1.9));
    QString text = thinking ? QStringLiteral(". . .") : m_bubbleText;
    const QRectF br = fm.boundingRect(QRectF(0, 0, maxW - 24.0, 1000.0), Qt::TextWordWrap | Qt::AlignCenter, text);
    const qreal bw = br.width() + 26.0;
    const qreal bh = br.height() + 18.0;
    const qreal headTop = c.y() - s * 0.6;
    const qreal bx = clamp(c.x() - bw * 0.35, 6.0, width() - bw - 6.0);
    const qreal by = std::max(4.0, headTop - bh - s * 0.18);
    const qreal since = now - m_bubbleShownAt;
    const qreal left = m_bubbleUntil - now;
    const qreal a = clamp(std::min(since * 6.0, left * 4.0), 0.0, 1.0);
    p.save();
    p.setOpacity(a);
    const QColor ink(QStringLiteral("#22222A"));
    p.setPen(QPen(QColor(30, 30, 40, 60), 1.2));
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    // Trail of little thought circles to the head.
    const qreal hx = c.x() + s * 0.15;
    const qreal radii[2] = {s * 0.025, s * 0.04};
    for (int k = 0; k < 2; ++k) {
        const qreal t = (k + 1) / 3.0;
        p.drawEllipse(QPointF(hx + (bx + bw * 0.4 - hx) * t, headTop + (by + bh - headTop) * t), radii[k], radii[k]);
    }
    const qreal corner = bh < 50 ? bh * 0.35 : 16.0;
    p.drawRoundedRect(QRectF(bx, by, bw, bh), corner, corner);
    p.setFont(f);
    p.setPen(ink);
    if (thinking) {
        const int dots = int(now * 4.0) % 4;
        text = QString(dots, QLatin1Char('.'));
        f.setBold(true);
        p.setFont(f);
    }
    p.drawText(QRectF(bx + 13.0, by + 9.0, bw - 26.0, bh - 18.0), Qt::TextWordWrap | Qt::AlignCenter, text);
    p.restore();
}
