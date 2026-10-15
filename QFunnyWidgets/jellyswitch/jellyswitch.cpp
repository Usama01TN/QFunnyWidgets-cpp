/**
 * JellySwitchWidget: a toggle switch whose knob is a jelly blob with a face. It sleeps (with floating
 * "z") when off, wakes up happy when on, squashes when pressed and wobbles like jelly when it slides.
 */
#include "jellyswitch.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QTimer>
#include <cmath>

namespace {

inline qreal clamp(qreal x, qreal a, qreal b)
{
    return x < a ? a : (x > b ? b : x);
}

inline qreal lerp(qreal a, qreal b, qreal t)
{
    return a + (b - a) * t;
}

inline QColor lerpColor(const QColor &a, const QColor &b, qreal t)
{
    t = clamp(t, 0.0, 1.0);
    return QColor(int(lerp(a.red(), b.red(), t)), int(lerp(a.green(), b.green(), t)),
                  int(lerp(a.blue(), b.blue(), t)), int(lerp(a.alpha(), b.alpha(), t)));
}

/** Python-style modulo (result has the sign of the divisor). */
inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

inline QPoint eventPos(const QMouseEvent *e)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position().toPoint();
#else
    return e->pos();
#endif
}

const qreal PI = 3.14159265358979323846;

} // namespace

JellySwitchWidget::JellySwitchWidget(QWidget *parent)
    : JellySwitchWidget(Options(), parent)
{
}

JellySwitchWidget::JellySwitchWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_checked(options.checked),
      m_onColor(options.onColor),
      m_offColor(options.offColor),
      m_knobColor(options.knobColor),
      m_wobbliness(clamp(options.wobbliness, 0.0, 1.0)),
      m_showFace(options.showFace)
{
    if (options.toggled)
        connect(this, &JellySwitchWidget::toggled, this, options.toggled);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    // State.
    m_pos = m_checked ? 1.0 : 0.0;
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &JellySwitchWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal JellySwitchWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QRectF JellySwitchWidget::trackRect() const
{
    const qreal w = width();
    const qreal h = height();
    const qreal th = std::min(h * 0.8, w / 2.0) - 4.0;
    const qreal tw = th * 2.0;
    return QRectF((w - tw) / 2.0, (h - th) / 2.0, tw, th);
}

// ---------- Public API ----------
void JellySwitchWidget::toggle()
{
    setChecked(!m_checked);
}

bool JellySwitchWidget::isChecked() const
{
    return m_checked;
}

void JellySwitchWidget::setChecked(bool checked)
{
    if (checked != m_checked) {
        m_checked = checked;
        m_toggleTime = now();
        m_squashVel += checked ? 10.0 : -10.0;
        if (!m_timer->isActive())
            m_timer->start(16);
        emit toggled(checked);
        update();
    }
}

QColor JellySwitchWidget::getOnColor() const
{
    return m_onColor;
}

void JellySwitchWidget::setOnColor(const QColor &color)
{
    if (m_onColor != color) {
        m_onColor = color;
        update();
    }
}

QColor JellySwitchWidget::getOffColor() const
{
    return m_offColor;
}

void JellySwitchWidget::setOffColor(const QColor &color)
{
    if (m_offColor != color) {
        m_offColor = color;
        update();
    }
}

QColor JellySwitchWidget::getKnobColor() const
{
    return m_knobColor;
}

void JellySwitchWidget::setKnobColor(const QColor &color)
{
    if (m_knobColor != color) {
        m_knobColor = color;
        update();
    }
}

qreal JellySwitchWidget::getWobbliness() const
{
    return m_wobbliness;
}

void JellySwitchWidget::setWobbliness(qreal wobbliness)
{
    m_wobbliness = clamp(wobbliness, 0.0, 1.0);
}

bool JellySwitchWidget::isShowFace() const
{
    return m_showFace;
}

void JellySwitchWidget::setShowFace(bool showFace)
{
    m_showFace = showFace;
    update();
}

// ---------- Qt overrides ----------
QSize JellySwitchWidget::sizeHint() const
{
    return QSize(130, 70);
}

QSize JellySwitchWidget::minimumSizeHint() const
{
    return QSize(50, 28);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void JellySwitchWidget::enterEvent(QEnterEvent *e)
#else
void JellySwitchWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void JellySwitchWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void JellySwitchWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        m_pressed = true;
    QWidget::mousePressEvent(e);
}

void JellySwitchWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressed && e->button() == Qt::LeftButton) {
        m_pressed = false;
        if (rect().contains(eventPos(e)))
            toggle();
    }
    QWidget::mouseReleaseEvent(e);
}

void JellySwitchWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        toggle();
        return;
    }
    QWidget::keyPressEvent(e);
}

void JellySwitchWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void JellySwitchWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void JellySwitchWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal target = m_checked ? 1.0 : 0.0;
    const qreal wob = m_wobbliness;
    // Knob slide spring (slight overshoot).
    const qreal k = 220.0;
    const qreal c = 22.0 - 8.0 * wob;
    m_vel += ((target - m_pos) * k - m_vel * c) * dt;
    m_pos += m_vel * dt;
    // Jelly deformation spring, kicked by knob velocity.
    const qreal ks = 380.0 - 200.0 * wob;
    const qreal cs = 12.0 - 9.0 * wob;
    const qreal drive = m_vel * 0.08;
    m_squashVel += ((drive - m_squash) * ks - m_squashVel * cs) * dt;
    m_squash += m_squashVel * dt;
    m_press += ((m_pressed ? 1.0 : 0.0) - m_press) * std::min(1.0, dt * 18.0);
    m_hover += ((m_hovered ? 1.0 : 0.0) - m_hover) * std::min(1.0, dt * 10.0);
    update();
}

// ---------- Painting ----------
void JellySwitchWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const QRectF track = trackRect();
    const qreal th = track.height();
    const qreal t = clamp(m_pos, 0.0, 1.0);
    // Track.
    const QColor tc = lerpColor(m_offColor, m_onColor, t);
    QPainterPath path;
    path.addRoundedRect(track, th / 2.0, th / 2.0);
    p.setPen(Qt::NoPen);
    p.fillPath(path, tc);
    const QColor inner(0, 0, 0, 30);
    p.setPen(QPen(inner, 1.5));
    p.drawPath(path);
    if (hasFocus()) {
        QColor fc(tc);
        fc.setAlpha(110);
        p.setPen(QPen(fc, 2.0));
        p.drawRoundedRect(track.adjusted(-3, -3, 3, 3), th / 2.0 + 3, th / 2.0 + 3);
    }
    // Knob geometry.
    const qreal pad = th * 0.1;
    const qreal r = th / 2.0 - pad;
    const qreal x0 = track.left() + pad + r;
    const qreal x1 = track.right() - pad - r;
    const qreal cx = lerp(x0, x1, m_pos);
    const qreal cy = track.center().y();
    const qreal sq = clamp(m_squash, -0.45, 0.45);
    const qreal sx = 1.0 + std::abs(sq) + 0.12 * m_press + 0.05 * m_hover;
    const qreal sy = 1.0 - std::abs(sq) * 0.6 - 0.18 * m_press + 0.05 * m_hover;
    p.save();
    p.translate(cx, cy + r * 0.18 * m_press);
    // Shadow.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 50));
    p.drawEllipse(QPointF(0, r * 0.12), r * sx, r * sy);
    // Jelly body with glossy gradient.
    const QColor kc = m_knobColor;
    QRadialGradient grad(QPointF(-r * 0.35, -r * 0.45), r * 1.8);
    grad.setColorAt(0.0, kc.lighter(105));
    grad.setColorAt(0.6, kc);
    grad.setColorAt(1.0, kc.darker(118));
    p.setBrush(grad);
    p.drawEllipse(QPointF(0, 0), r * sx, r * sy);
    p.setBrush(QColor(255, 255, 255, 150));
    p.drawEllipse(QPointF(-r * 0.38 * sx, -r * 0.48 * sy), r * 0.22, r * 0.12);
    if (m_showFace)
        drawFace(p, r, sx, sy, t, tNow);
    p.restore();
    // Floating "z" when asleep.
    if (t < 0.15 && m_showFace) {
        QFont f(font());
        f.setBold(true);
        for (int i = 0; i < 3; ++i) {
            const qreal ph = pyMod(tNow * 0.6 + i / 3.0, 1.0);
            f.setPointSizeF(std::max(5.0, th * (0.16 + 0.14 * ph)));
            p.setFont(f);
            QColor c(palette().text().color());
            c.setAlpha(int(200 * std::sin(ph * PI) * (1.0 - t / 0.15)));
            p.setPen(c);
            p.drawText(QPointF(cx + r * 0.7 + ph * r * 0.9 + std::sin(ph * 6.0) * 3.0,
                               cy - r * 0.6 - ph * r * 1.4),
                       QStringLiteral("z"));
        }
    }
    p.end();
}

void JellySwitchWidget::drawFace(QPainter &p, qreal r, qreal sx, qreal sy, qreal t, qreal now)
{
    const QColor ink(QStringLiteral("#2B2B33"));
    const qreal look = clamp(m_vel * 0.06, -0.25, 0.25) * r; // eyes lead the motion
    const qreal ex = r * 0.34 * sx;
    const qreal ey = -r * 0.1 * sy;
    const QPen pen(ink, std::max(1.2, r * 0.09), Qt::SolidLine, Qt::RoundCap);
    const qreal excited = clamp(1.0 - (now - m_toggleTime) / 0.8, 0.0, 1.0) * t;
    for (qreal d : {-1.0, 1.0}) {
        const qreal x = d * ex + look;
        if (t < 0.5) {
            // Sleepy closed eyes: little arcs.
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawArc(QRectF(x - r * 0.15, ey - r * 0.1, r * 0.3, r * 0.2), 180 * 16, 180 * 16);
        } else {
            const qreal open = clamp((t - 0.5) * 2.0, 0.0, 1.0);
            const qreal er = r * (0.12 + 0.05 * excited);
            p.setPen(Qt::NoPen);
            p.setBrush(ink);
            p.drawEllipse(QPointF(x, ey), er, er * open);
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(x - er * 0.35, ey - er * 0.35 * open), er * 0.35, er * 0.35 * open);
        }
    }
    // Blush.
    if (t > 0.5) {
        QColor bc(QStringLiteral("#FF7A90"));
        bc.setAlpha(int(150 * (t - 0.5) * 2.0));
        p.setPen(Qt::NoPen);
        p.setBrush(bc);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(d * r * 0.58 * sx + look, r * 0.16), r * 0.15, r * 0.09);
    }
    // Mouth: tiny "o" while asleep, big smile when on.
    p.setPen(pen);
    if (t < 0.5) {
        p.setBrush(Qt::NoBrush);
        const qreal breathe = 1.0 + 0.25 * std::sin(now * 3.0);
        p.drawEllipse(QPointF(look, r * 0.3), r * 0.07 * breathe, r * 0.09 * breathe);
    } else {
        const qreal mw = r * (0.42 + 0.12 * excited);
        QPainterPath mouth;
        mouth.moveTo(-mw / 2.0 + look, r * 0.18);
        mouth.quadTo(look, r * (0.5 + 0.15 * excited), mw / 2.0 + look, r * 0.18);
        if (excited > 0.05)
            p.setBrush(QColor(QStringLiteral("#E85A6B")));
        else
            p.setBrush(Qt::NoBrush);
        p.drawPath(mouth);
    }
}
