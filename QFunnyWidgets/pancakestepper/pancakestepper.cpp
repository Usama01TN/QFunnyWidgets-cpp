/**
 * PancakeStepperWidget: a quantity spin box served as a stack of pancakes. Incrementing drops a pancake onto the
 * stack (it bounces and squashes), decrementing flips the top one off the plate; a tall stack sways nervously
 * and the stepper refuses to go above the maximum.
 */
#include "pancakestepper.h"

#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <utility>

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

PancakeStepperWidget::PancakeStepperWidget(QWidget *parent)
    : PancakeStepperWidget(Options(), parent)
{
}

PancakeStepperWidget::PancakeStepperWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_minimum(std::max(0, options.minimum)),
      m_maximum(clampInt(options.maximum, m_minimum + 1, 20)),
      m_value(clampInt(options.value, m_minimum, m_maximum)),
      m_unit(options.unit),
      m_accentColor(options.accentColor)
{
    if (options.valueChanged)
        connect(this, &PancakeStepperWidget::valueChanged, this, options.valueChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    // State.
    for (int i = 0; i < m_value; ++i)
        m_drops.append(Drop());
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PancakeStepperWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal PancakeStepperWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void PancakeStepperWidget::geometry(QPointF &plate, qreal &pw, qreal &th) const
{
    const qreal w = width();
    const qreal h = height();
    plate = QPointF(w / 2.0, h - 58.0);
    pw = clamp(std::min(w * 0.55, 170.0), 60.0, 170.0);
    th = clamp((plate.y() - 40.0) / (m_maximum + 1.5), 5.0, 16.0);
}

void PancakeStepperWidget::refuse()
{
    m_refuseAt = now();
    m_swayVel += uniform(0, 1) < 0.5 ? 45.0 : -45.0;
}

// ---------- Public API ----------
void PancakeStepperWidget::increment()
{
    if (m_value >= m_maximum)
        refuse();
    else
        setValue(m_value + 1);
}

void PancakeStepperWidget::decrement()
{
    setValue(m_value - 1);
}

int PancakeStepperWidget::getValue() const
{
    return m_value;
}

void PancakeStepperWidget::setValue(int value)
{
    value = clampInt(value, m_minimum, m_maximum);
    if (value == m_value)
        return;
    QPointF plate;
    qreal pw, th;
    geometry(plate, pw, th);
    while (m_drops.size() < value)
        m_drops.append(Drop(plate.y() * 0.6 + uniform(0, 20)));
    while (m_drops.size() > value) {
        m_drops.removeLast();
        const qreal top = plate.y() - (m_drops.size() + 0.5) * th;
        const qreal vx = uniform(-120, 120);
        const qreal vy = -uniform(380, 480);
        const qreal vrot = uniform(-720, 720);
        m_flips.append(Flip(plate.x(), top, vx, vy, vrot));
    }
    m_value = value;
    emit valueChanged(value);
    update();
}

int PancakeStepperWidget::getMinimum() const
{
    return m_minimum;
}

void PancakeStepperWidget::setMinimum(int minimum)
{
    m_minimum = clampInt(minimum, 0, m_maximum - 1);
    if (m_value < m_minimum)
        setValue(m_minimum);
}

int PancakeStepperWidget::getMaximum() const
{
    return m_maximum;
}

void PancakeStepperWidget::setMaximum(int maximum)
{
    m_maximum = clampInt(maximum, m_minimum + 1, 20);
    if (m_value > m_maximum)
        setValue(m_maximum);
    update();
}

QString PancakeStepperWidget::getUnit() const { return m_unit; }
QColor PancakeStepperWidget::getAccentColor() const { return m_accentColor; }

void PancakeStepperWidget::setUnit(const QString &unit)
{
    m_unit = unit;
    update();
}

void PancakeStepperWidget::setAccentColor(const QColor &color)
{
    m_accentColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize PancakeStepperWidget::sizeHint() const
{
    return QSize(260, 280);
}

QSize PancakeStepperWidget::minimumSizeHint() const
{
    return QSize(160, 180);
}

void PancakeStepperWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void PancakeStepperWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void PancakeStepperWidget::mouseMoveEvent(QMouseEvent *e)
{
    const int hover = m_minusRect.contains(eventPos(e)) ? -1 : (m_plusRect.contains(eventPos(e)) ? 1 : 0);
    if (hover != m_hoverBtn) {
        m_hoverBtn = hover;
        setCursor(hover ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
    QWidget::mouseMoveEvent(e);
}

void PancakeStepperWidget::leaveEvent(QEvent *e)
{
    m_hoverBtn = 0;
    QWidget::leaveEvent(e);
}

void PancakeStepperWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (m_minusRect.contains(eventPos(e)))
            decrement();
        else if (m_plusRect.contains(eventPos(e)))
            increment();
    }
    QWidget::mouseReleaseEvent(e);
}

void PancakeStepperWidget::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() > 0)
        increment();
    else
        decrement();
    e->accept();
}

void PancakeStepperWidget::keyPressEvent(QKeyEvent *e)
{
    const int k = e->key();
    if (k == Qt::Key_Up || k == Qt::Key_Right || k == Qt::Key_Plus)
        increment();
    else if (k == Qt::Key_Down || k == Qt::Key_Left || k == Qt::Key_Minus)
        decrement();
    else
        QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void PancakeStepperWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    for (Drop &d : m_drops) {
        if (d.offset > 0 || d.vel != 0) {
            d.vel -= 2600.0 * dt;
            d.offset += d.vel * dt;
            if (d.offset <= 0) {
                d.offset = 0.0;
                d.squash = clamp(-d.vel / 1400.0, 0.0, 0.5);
                d.vel = std::abs(d.vel) > 160 ? -d.vel * 0.25 : 0.0;
                if (d.vel != 0.0)
                    m_swayVel += uniform(-6, 6);
            }
        }
        d.squash = std::max(0.0, d.squash - dt * 3.0);
    }
    // A tall stack sways on its own; refusal adds a big shove.
    const qreal tall = clamp((m_value - m_maximum * 0.6) / (m_maximum * 0.4), 0.0, 1.0);
    m_swayVel += std::sin(t * 2.3) * 20.0 * tall * dt;
    m_swayVel += (-m_sway * 30.0 - m_swayVel * 2.5) * dt;
    m_sway = clamp(m_sway + m_swayVel * dt, -14.0, 14.0);
    QList<Flip> alive;
    for (Flip f : std::as_const(m_flips)) {
        f.vy += 1500.0 * dt;
        f.x += f.vx * dt;
        f.y += f.vy * dt;
        f.rot += f.vrot * dt;
        f.life -= dt;
        if (f.life > 0 && f.y < height() + 40)
            alive.append(f);
    }
    m_flips = alive;
    update();
}

// ---------- Painting ----------
void PancakeStepperWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF plate;
    qreal pw, th;
    geometry(plate, pw, th);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const QColor tc(palette().text().color());
    // Plate.
    p.setPen(QPen(QColor(QStringLiteral("#C9CCD6")), 1.2));
    p.setBrush(QColor(QStringLiteral("#F4F5F8")));
    p.drawEllipse(QPointF(plate.x(), plate.y() + th * 0.4), pw * 0.68, th * 1.3);
    p.setBrush(QColor(QStringLiteral("#E6E8EE")));
    p.drawEllipse(QPointF(plate.x(), plate.y() + th * 0.4), pw * 0.48, th * 0.8);
    // Stack (rotates around the plate as it sways).
    p.save();
    p.translate(plate);
    p.rotate(m_sway);
    const int n = int(m_drops.size());
    for (int i = 0; i < n; ++i) {
        const Drop &d = m_drops.at(i);
        const qreal y = -(i + 0.5) * th - d.offset;
        const qreal jx = std::abs(m_swayVel) > 30 ? uniform(-0.3, 0.3) : 0.0;
        drawPancake(p, QPointF(jx, y), pw, th, d.squash, i == n - 1, tNow);
    }
    p.restore();
    // Flipped pancakes.
    for (const Flip &f : std::as_const(m_flips)) {
        p.save();
        p.translate(f.x, f.y);
        p.rotate(f.rot);
        p.setOpacity(clamp(f.life / 0.4, 0.0, 1.0));
        drawPancake(p, QPointF(0, 0), pw, th, 0.0, false, tNow);
        p.restore();
    }
    // Controls.
    const qreal h = height();
    const qreal br = 17.0;
    const qreal cy = h - 22.0;
    m_minusRect = QRectF(plate.x() - pw * 0.7 - br, cy - br, br * 2, br * 2);
    m_plusRect = QRectF(plate.x() + pw * 0.7 - br, cy - br, br * 2, br * 2);
    const QColor ac = m_accentColor;
    const struct { QRectF rect; int sign; bool enabled; bool hov; } buttons[2] = {
        {m_minusRect, -1, m_value > m_minimum, m_hoverBtn == -1},
        {m_plusRect, 1, m_value < m_maximum, m_hoverBtn == 1}};
    for (const auto &b : buttons) {
        QColor col(b.hov ? ac.lighter(115) : ac);
        if (!b.enabled)
            col.setAlpha(80);
        p.setPen(Qt::NoPen);
        p.setBrush(col);
        p.drawEllipse(b.rect);
        p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), 2.5, Qt::SolidLine, Qt::RoundCap));
        const QPointF c = b.rect.center();
        p.drawLine(c - QPointF(6, 0), c + QPointF(6, 0));
        if (b.sign > 0)
            p.drawLine(c - QPointF(0, 6), c + QPointF(0, 6));
    }
    // Label.
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(10.5);
    p.setFont(f);
    const bool refusing = tNow - m_refuseAt < 1.6;
    const int v = m_value;
    QString label;
    if (refusing) {
        label = tr("That's enough %1s.").arg(m_unit);
        p.setPen(QColor(QStringLiteral("#E63946")));
    } else {
        label = v ? QStringLiteral("%1 %2%3").arg(v).arg(m_unit, v == 1 ? QString() : QStringLiteral("s"))
                  : tr("No %1s :(").arg(m_unit);
        p.setPen(tc);
    }
    p.drawText(QRectF(m_minusRect.right(), cy - 12.0, m_plusRect.left() - m_minusRect.right(), 24.0), Qt::AlignCenter, label);
    if (v >= m_maximum * 0.8 && !refusing) {
        QFont sf(font());
        sf.setItalic(true);
        sf.setPointSizeF(8.5);
        p.setFont(sf);
        QColor wc(tc);
        wc.setAlpha(150);
        p.setPen(wc);
        p.drawText(QRectF(0, 4, width(), 18), Qt::AlignHCenter | Qt::AlignTop, tr("Whoa... steady..."));
    }
    p.end();
}

void PancakeStepperWidget::drawPancake(QPainter &p, const QPointF &c, qreal pw, qreal th, qreal squash, bool top, qreal now)
{
    const qreal w = pw * (1.0 + squash * 0.3);
    const qreal t = th * (1.0 - squash);
    const QRectF rect(c.x() - w / 2.0, c.y() - t / 2.0, w, t);
    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
    grad.setColorAt(0.0, QColor(QStringLiteral("#F2C27B")));
    grad.setColorAt(0.5, QColor(QStringLiteral("#E0A458")));
    grad.setColorAt(1.0, QColor(QStringLiteral("#B9783A")));
    p.setPen(QPen(QColor(QStringLiteral("#9A5F2A")), 1.0));
    p.setBrush(grad);
    p.drawRoundedRect(rect, t / 2.0, t / 2.0);
    if (!top)
        return;
    // Syrup drips + butter + face.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(170, 90, 20, 200));
    QPainterPath sp;
    sp.addRoundedRect(QRectF(rect.left() + w * 0.12, rect.top() - 1, w * 0.76, t * 0.5), t * 0.25, t * 0.25);
    p.drawPath(sp);
    const qreal fxs[3] = {0.2, 0.55, 0.78};
    for (int k = 0; k < 3; ++k) {
        const qreal drip = t * (0.6 + 0.4 * pyMod(now * 0.4 + k * 0.33, 1.0));
        p.drawRoundedRect(QRectF(rect.left() + w * fxs[k] - 2.5, rect.top(), 5.0, drip), 2.5, 2.5);
    }
    p.setBrush(QColor(QStringLiteral("#FFE680")));
    p.setPen(QPen(QColor(QStringLiteral("#E8C547")), 1.0));
    const qreal bw = std::min(w * 0.2, 26.0);
    p.drawRoundedRect(QRectF(c.x() - bw / 2.0 + w * 0.1, rect.top() - bw * 0.45, bw, bw * 0.5), 3, 3);
    // A smug little face on the side of the top pancake.
    const QColor ink(QStringLiteral("#4A2A10"));
    p.setPen(Qt::NoPen);
    p.setBrush(ink);
    const bool nervous = m_value >= m_maximum * 0.8;
    for (qreal d : {-1.0, 1.0})
        p.drawEllipse(QPointF(c.x() - w * 0.12 + d * t * 0.6, c.y() + t * 0.05), t * 0.13, t * (nervous ? 0.2 : 0.13));
    p.setPen(QPen(ink, std::max(1.0, t * 0.1), Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    if (nervous)
        p.drawLine(QPointF(c.x() - w * 0.12 - t * 0.3, c.y() + t * 0.32), QPointF(c.x() - w * 0.12 + t * 0.3, c.y() + t * 0.28));
    else
        p.drawArc(QRectF(c.x() - w * 0.12 - t * 0.35, c.y() - t * 0.05, t * 0.7, t * 0.45), 200 * 16, 140 * 16);
}
