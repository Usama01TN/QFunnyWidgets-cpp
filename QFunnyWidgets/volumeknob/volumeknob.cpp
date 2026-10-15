/**
 * VolumeKnobWidget: a rotary volume dial with a face. Muted it sleeps with a zipped mouth, then it smiles, sings
 * (with floating music notes), squints at loud volumes and screams (shaking, glowing red, sound waves) at the top.
 */
#include "volumeknob.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QTimer>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <utility>

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

inline qreal lerp(qreal a, qreal b, qreal t)
{
    return a + (b - a) * t;
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

inline qreal degrees(qreal rad)
{
    return rad * 180.0 / PI;
}

inline qreal radians(qreal deg)
{
    return deg * PI / 180.0;
}

} // namespace

QColor VolumeKnobWidget::levelColor(qreal t)
{
    static const struct { qreal pos; const char *color; } stops[] = {
        {0.0, "#3BCEAC"}, {0.55, "#FFD23F"}, {0.8, "#FF9F1C"}, {1.0, "#E63946"}};
    t = clamp(t, 0.0, 1.0);
    for (int i = 0; i < 3; ++i) {
        const qreal a = stops[i].pos;
        const qreal b = stops[i + 1].pos;
        const QColor ca(QString::fromLatin1(stops[i].color));
        const QColor cb(QString::fromLatin1(stops[i + 1].color));
        if (t <= b) {
            const qreal k = (t - a) / (b - a);
            return QColor(int(lerp(ca.red(), cb.red(), k)), int(lerp(ca.green(), cb.green(), k)),
                          int(lerp(ca.blue(), cb.blue(), k)));
        }
    }
    return QColor(QString::fromLatin1(stops[3].color));
}

VolumeKnobWidget::VolumeKnobWidget(QWidget *parent)
    : VolumeKnobWidget(Options(), parent)
{
}

VolumeKnobWidget::VolumeKnobWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_value(clampInt(options.value, 0, 100)),
      m_knobColor(options.knobColor),
      m_trackColor(options.trackColor),
      m_showLabel(options.showLabel)
{
    if (options.valueChanged)
        connect(this, &VolumeKnobWidget::valueChanged, this, options.valueChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::OpenHandCursor);
    // State.
    m_shown = m_value / 100.0;
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &VolumeKnobWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal VolumeKnobWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void VolumeKnobWidget::geometry(QPointF &c, qreal &R, qreal &labelTop) const
{
    const qreal w = width();
    const qreal h = height();
    const qreal labelH = m_showLabel ? 40.0 : 0.0;
    R = std::max(12.0, std::min(w, h - labelH) / 2.0 * 0.62);
    c = QPointF(w / 2.0, (h - labelH) / 2.0);
    labelTop = h - labelH;
}

int VolumeKnobWidget::valueFromPos(const QPointF &pos) const
{
    QPointF c;
    qreal R, top;
    geometry(c, R, top);
    const qreal ang = degrees(std::atan2(-(pos.y() - c.y()), pos.x() - c.x()));
    const qreal d = pyMod(START_ANGLE - ang, 360.0);
    if (d > SWEEP) {
        // Dead zone at the bottom: snap to the closer end.
        return d < SWEEP + (360.0 - SWEEP) / 2.0 ? 100 : 0;
    }
    return int(std::nearbyint(d / SWEEP * 100.0));
}

QString VolumeKnobWidget::mood(qreal t)
{
    static const struct { qreal limit; const char *name; } MOODS[] = {
        {0.0, "Muted"}, {0.3, "Whisper"}, {0.65, "Chill"}, {0.9, "Party"}, {1.01, "NEIGHBOURS ARE CALLING"}};
    if (t <= 0.001)
        return QString::fromLatin1(MOODS[0].name);
    for (int i = 1; i < 5; ++i) {
        if (t < MOODS[i].limit)
            return QString::fromLatin1(MOODS[i].name);
    }
    return QString::fromLatin1(MOODS[4].name);
}

// ---------- Public API ----------
void VolumeKnobWidget::toggleMute()
{
    if (m_value > 0) {
        m_beforeMute = m_value;
        setValue(0);
    } else {
        setValue(m_beforeMute ? m_beforeMute : 40);
    }
}

int VolumeKnobWidget::getValue() const
{
    return m_value;
}

void VolumeKnobWidget::setValue(int value)
{
    value = clampInt(value, 0, 100);
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
        update();
    }
}

QColor VolumeKnobWidget::getKnobColor() const
{
    return m_knobColor;
}

void VolumeKnobWidget::setKnobColor(const QColor &color)
{
    if (m_knobColor != color) {
        m_knobColor = color;
        update();
    }
}

QColor VolumeKnobWidget::getTrackColor() const
{
    return m_trackColor;
}

void VolumeKnobWidget::setTrackColor(const QColor &color)
{
    if (m_trackColor != color) {
        m_trackColor = color;
        update();
    }
}

bool VolumeKnobWidget::isShowLabel() const
{
    return m_showLabel;
}

void VolumeKnobWidget::setShowLabel(bool showLabel)
{
    m_showLabel = showLabel;
    update();
}

// ---------- Qt overrides ----------
QSize VolumeKnobWidget::sizeHint() const
{
    return QSize(240, 260);
}

QSize VolumeKnobWidget::minimumSizeHint() const
{
    return QSize(100, 120);
}

void VolumeKnobWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void VolumeKnobWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void VolumeKnobWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        setCursor(Qt::ClosedHandCursor);
        setValue(valueFromPos(eventPos(e)));
    }
    QWidget::mousePressEvent(e);
}

void VolumeKnobWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging) {
        const int v = valueFromPos(eventPos(e));
        // Avoid jumping from 100 straight to 0 across the dead zone.
        if (std::abs(v - m_value) < 60)
            setValue(v);
    }
    QWidget::mouseMoveEvent(e);
}

void VolumeKnobWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragging = false;
    setCursor(Qt::OpenHandCursor);
    QWidget::mouseReleaseEvent(e);
}

void VolumeKnobWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    toggleMute();
    QWidget::mouseDoubleClickEvent(e);
}

void VolumeKnobWidget::wheelEvent(QWheelEvent *e)
{
    setValue(m_value + (e->angleDelta().y() > 0 ? 3 : -3));
    e->accept();
}

void VolumeKnobWidget::keyPressEvent(QKeyEvent *e)
{
    const int k = e->key();
    if (k == Qt::Key_Up || k == Qt::Key_Right)
        setValue(m_value + 1);
    else if (k == Qt::Key_Down || k == Qt::Key_Left)
        setValue(m_value - 1);
    else if (k == Qt::Key_PageUp)
        setValue(m_value + 10);
    else if (k == Qt::Key_PageDown)
        setValue(m_value - 10);
    else if (k == Qt::Key_M)
        toggleMute();
    else
        QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void VolumeKnobWidget::tick()
{
    const qreal tNow = now();
    const qreal dt = std::min(0.05, std::max(0.0, tNow - m_last));
    m_last = tNow;
    m_shown += (m_value / 100.0 - m_shown) * std::min(1.0, dt * 14.0);
    const qreal t = m_shown;
    QPointF c;
    qreal R, top;
    geometry(c, R, top);
    if (0.05 < t && t < 0.9) {
        m_noteAcc += dt * (0.8 + t * 4.0);
        while (m_noteAcc > 1.0) {
            m_noteAcc -= 1.0;
            const qreal side = uniform(0, 1) < 0.5 ? -1.0 : 1.0;
            const qreal vx = side * uniform(15, 35);
            const qreal vy = -uniform(25, 45);
            const qreal life = uniform(1.2, 1.8);
            const qreal size = R / 60.0 * uniform(0.8, 1.2);
            m_notes.append(Note(c.x() + side * R * 0.9, c.y() - R * 0.3, vx, vy, life, size));
        }
    }
    QList<Note> alive;
    for (Note n : std::as_const(m_notes)) {
        n.x += n.vx * dt + std::sin(tNow * 4.0 + n.y * 0.05) * 10.0 * dt;
        n.y += n.vy * dt;
        n.life -= dt;
        if (n.life > 0)
            alive.append(n);
    }
    m_notes = alive;
    update();
}

// ---------- Painting ----------
void VolumeKnobWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF c;
    qreal R, labelTop;
    geometry(c, R, labelTop);
    const qreal t = clamp(m_shown, 0.0, 1.0);
    const QColor lc = levelColor(t);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Sound waves.
    if (t > 0.65) {
        const qreal loud = (t - 0.65) / 0.35;
        QColor wc(lc);
        for (int k = 0; k < 3; ++k) {
            const qreal ph = pyMod(tNow * (1.5 + loud * 2.0) + k / 3.0, 1.0);
            wc.setAlpha(int(200 * (1.0 - ph) * (0.4 + 0.6 * loud)));
            p.setPen(QPen(wc, std::max(1.5, R * 0.05), Qt::SolidLine, Qt::RoundCap));
            p.setBrush(Qt::NoBrush);
            const qreal rr = R * (1.4 + ph * 0.5);
            const QRectF rect(c.x() - rr, c.y() - rr, rr * 2, rr * 2);
            p.drawArc(rect, -30 * 16, 60 * 16);
            p.drawArc(rect, 150 * 16, 60 * 16);
        }
    }
    // Track + level ring.
    const qreal rr = R * 1.2;
    const QRectF ring(c.x() - rr, c.y() - rr, rr * 2, rr * 2);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(m_trackColor, R * 0.11, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(ring, int(START_ANGLE * 16), int(-SWEEP * 16));
    if (t > 0.004) {
        p.setPen(QPen(lc, R * 0.11, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(ring, int(START_ANGLE * 16), int(-SWEEP * t * 16));
    }
    // Knob (shakes when loud).
    const qreal shake = R * 0.04 * std::pow(clamp((t - 0.7) / 0.3, 0.0, 1.0), 2);
    const qreal jx = uniform(-shake, shake);
    const qreal jy = uniform(-shake, shake);
    const QPointF kc(c.x() + jx, c.y() + jy);
    QColor base = m_knobColor;
    if (t > 0.9) {
        const qreal k = (t - 0.9) / 0.1 * (0.6 + 0.4 * std::sin(tNow * 14.0));
        base = QColor(int(lerp(base.red(), 255, k * 0.5)), int(lerp(base.green(), 120, k * 0.5)),
                      int(lerp(base.blue(), 120, k * 0.5)));
    }
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 55));
    p.drawEllipse(kc + QPointF(0, R * 0.08), R, R);
    QRadialGradient grad(kc + QPointF(-R * 0.3, -R * 0.4), R * 1.7);
    grad.setColorAt(0.0, base.lighter(110));
    grad.setColorAt(1.0, base.darker(125));
    p.setBrush(grad);
    p.setPen(QPen(base.darker(140), std::max(1.0, R * 0.03)));
    p.drawEllipse(kc, R, R);
    // Grip ridges + notch rotate with the value.
    const qreal ang = radians(START_ANGLE - SWEEP * t);
    p.setPen(QPen(base.darker(118), std::max(1.0, R * 0.03), Qt::SolidLine, Qt::RoundCap));
    for (int k = 0; k < 24; ++k) {
        const qreal a = ang + k * 3.14159265 * 2.0 / 24.0;
        p.drawLine(QPointF(kc.x() + std::cos(a) * R * 0.88, kc.y() - std::sin(a) * R * 0.88),
                   QPointF(kc.x() + std::cos(a) * R * 0.97, kc.y() - std::sin(a) * R * 0.97));
    }
    p.setPen(Qt::NoPen);
    p.setBrush(lc.darker(110));
    p.drawEllipse(QPointF(kc.x() + std::cos(ang) * R * 0.76, kc.y() - std::sin(ang) * R * 0.76), R * 0.07, R * 0.07);
    drawFace(p, kc, R * 0.62, t, tNow);
    // Notes.
    for (const Note &n : std::as_const(m_notes)) {
        QColor nc(lc);
        nc.setAlpha(int(230 * clamp(n.life / n.maxLife, 0.0, 1.0)));
        drawNote(p, QPointF(n.x, n.y), n.size, nc);
    }
    // Label.
    if (m_showLabel) {
        const QColor tc(palette().text().color());
        QFont f(font());
        f.setBold(true);
        f.setPointSizeF(11.0);
        p.setFont(f);
        p.setPen(tc);
        p.drawText(QRectF(0, labelTop, width(), 20), Qt::AlignCenter, QStringLiteral("%1%").arg(m_value));
        QFont f2(font());
        f2.setPointSizeF(8.5);
        f2.setBold(t >= 0.9);
        p.setFont(f2);
        QColor mc = t >= 0.9 ? QColor(lc) : QColor(tc);
        if (t < 0.9)
            mc.setAlpha(160);
        p.setPen(mc);
        p.drawText(QRectF(0, labelTop + 19, width(), 18), Qt::AlignCenter, mood(m_value / 100.0));
    }
    p.end();
}

void VolumeKnobWidget::drawFace(QPainter &p, const QPointF &c, qreal r, qreal t, qreal now)
{
    const QColor ink(QStringLiteral("#2B2B33"));
    const QPen pen(ink, std::max(1.2, r * 0.08), Qt::SolidLine, Qt::RoundCap);
    const qreal ex = r * 0.38;
    const qreal ey = c.y() - r * 0.2;
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    if (t <= 0.004) {
        // Peaceful closed eyes, zipped mouth.
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(c.x() + d * ex - r * 0.14, ey - r * 0.06, r * 0.28, r * 0.18), 180 * 16, 180 * 16);
        const qreal my = c.y() + r * 0.35;
        p.drawLine(QPointF(c.x() - r * 0.35, my), QPointF(c.x() + r * 0.35, my));
        const QPen zp(ink, std::max(1.0, r * 0.05));
        p.setPen(zp);
        for (int k = 0; k < 7; ++k) {
            const qreal x = c.x() - r * 0.3 + k * r * 0.1;
            p.drawLine(QPointF(x, my - r * 0.07), QPointF(x, my + r * 0.07));
        }
        p.setBrush(QColor(QStringLiteral("#C9A227")));
        p.drawRoundedRect(QRectF(c.x() + r * 0.33, my - r * 0.08, r * 0.14, r * 0.2), 2, 2);
        return;
    }
    if (t < 0.3) {
        // Content: open eyes, gentle smile.
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(c.x() + d * ex, ey), r * 0.09, r * 0.11);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(c.x() - r * 0.22, c.y() + r * 0.05, r * 0.44, r * 0.3), 200 * 16, 140 * 16);
    } else if (t < 0.65) {
        // Singing: happy closed eyes, "o" mouth pulsing with the beat.
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(c.x() + d * ex - r * 0.14, ey - r * 0.1, r * 0.28, r * 0.2), 20 * 16, 140 * 16);
        const qreal beat = 0.75 + 0.25 * std::abs(std::sin(now * 6.0));
        p.setBrush(QColor(QStringLiteral("#7A2E2E")));
        p.drawEllipse(QPointF(c.x(), c.y() + r * 0.3), r * 0.14 * beat, r * 0.18 * beat);
    } else if (t < 0.9) {
        // Loud: squinting > <, wide mouth.
        for (qreal d : {-1.0, 1.0}) {
            const qreal x = c.x() + d * ex;
            p.drawLine(QPointF(x - d * r * 0.13, ey - r * 0.1), QPointF(x + d * r * 0.08, ey));
            p.drawLine(QPointF(x + d * r * 0.08, ey), QPointF(x - d * r * 0.13, ey + r * 0.1));
        }
        p.setBrush(QColor(QStringLiteral("#7A2E2E")));
        QPainterPath mouth;
        mouth.addRoundedRect(QRectF(c.x() - r * 0.3, c.y() + r * 0.12, r * 0.6, r * 0.38), r * 0.15, r * 0.15);
        p.drawPath(mouth);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawRect(QRectF(c.x() - r * 0.22, c.y() + r * 0.14, r * 0.44, r * 0.08));
    } else {
        // TOO LOUD: huge eyes with tiny pupils, screaming.
        for (qreal d : {-1.0, 1.0}) {
            const qreal x = c.x() + d * ex;
            p.setPen(QPen(ink, std::max(1.0, r * 0.05)));
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(x, ey - r * 0.05), r * 0.22, r * 0.24);
            p.setPen(Qt::NoPen);
            p.setBrush(ink);
            const qreal jx = uniform(-1, 1);
            const qreal jy = uniform(-1, 1);
            p.drawEllipse(QPointF(x + jx, ey - r * 0.05 + jy), r * 0.05, r * 0.05);
        }
        p.setPen(pen);
        p.setBrush(QColor(QStringLiteral("#5A1A1A")));
        const qreal o = 1.0 + 0.12 * std::sin(now * 30.0);
        p.drawEllipse(QPointF(c.x(), c.y() + r * 0.38), r * 0.24, r * 0.26 * o);
    }
}

void VolumeKnobWidget::drawNote(QPainter &p, const QPointF &pt, qreal s, const QColor &color)
{
    p.save();
    p.translate(pt);
    p.scale(s, s);
    p.rotate(-20);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawEllipse(QPointF(0, 0), 5.0, 3.6);
    p.restore();
    p.save();
    p.translate(pt);
    p.scale(s, s);
    p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(4.2, -1.0), QPointF(4.2, -15.0));
    QPainterPath path;
    path.moveTo(4.2, -15.0);
    path.quadTo(10.0, -12.0, 8.0, -6.0);
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    p.restore();
}
