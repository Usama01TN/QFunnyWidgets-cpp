/**
 * MoodSliderWidget: a slider whose thumb is a face going from furious (with steam) to ecstatic (with floating
 * hearts). The thumb springs to its position, tilts while moving and shows the current mood in a bubble.
 */
#include "moodslider.h"

#include <QFontMetricsF>
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

/** Python's floor division for ints. */
inline int floorDiv(int a, int b)
{
    int q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0)))
        --q;
    return q;
}

QPainterPath heartPath(const QPointF &c, qreal size)
{
    const qreal x = c.x();
    const qreal y = c.y();
    const qreal s = size;
    QPainterPath path;
    path.moveTo(x, y + s * 0.9);
    path.cubicTo(x - s * 1.6, y - s * 0.1, x - s * 0.7, y - s * 1.4, x, y - s * 0.45);
    path.cubicTo(x + s * 0.7, y - s * 1.4, x + s * 1.6, y - s * 0.1, x, y + s * 0.9);
    return path;
}

} // namespace

QStringList MoodSliderWidget::defaultLabels()
{
    return {QStringLiteral("Furious"), QStringLiteral("Grumpy"), QStringLiteral("Meh"), QStringLiteral("Happy"),
            QStringLiteral("Ecstatic")};
}

QColor MoodSliderWidget::moodColor(qreal t)
{
    static const struct { qreal pos; const char *color; } MOOD_STOPS[] = {
        {0.0, "#E63946"}, {0.25, "#F4845F"}, {0.5, "#FFD23F"}, {0.75, "#9BDB4D"}, {1.0, "#3BCEAC"}};
    t = clamp(t, 0.0, 1.0);
    for (int i = 0; i < 4; ++i) {
        const qreal a = MOOD_STOPS[i].pos;
        const qreal b = MOOD_STOPS[i + 1].pos;
        const QColor ca(QString::fromLatin1(MOOD_STOPS[i].color));
        const QColor cb(QString::fromLatin1(MOOD_STOPS[i + 1].color));
        if (t <= b) {
            const qreal k = (t - a) / (b - a);
            return QColor(int(lerp(ca.red(), cb.red(), k)), int(lerp(ca.green(), cb.green(), k)),
                          int(lerp(ca.blue(), cb.blue(), k)));
        }
    }
    return QColor(QString::fromLatin1(MOOD_STOPS[4].color));
}

MoodSliderWidget::MoodSliderWidget(QWidget *parent)
    : MoodSliderWidget(Options(), parent)
{
}

MoodSliderWidget::MoodSliderWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_minimum(options.minimum),
      m_maximum(std::max(options.minimum + 1, options.maximum)),
      m_value(clampInt(options.value, m_minimum, m_maximum)),
      m_labels(options.labels.isEmpty() ? defaultLabels() : options.labels),
      m_showLabel(options.showLabel),
      m_trackColor(options.trackColor)
{
    if (options.valueChanged)
        connect(this, &MoodSliderWidget::valueChanged, this, options.valueChanged);
    if (options.moodChanged)
        connect(this, &MoodSliderWidget::moodChanged, this, options.moodChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    // State.
    m_shown = ratio();
    m_lastMood = getMood();
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MoodSliderWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal MoodSliderWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

qreal MoodSliderWidget::ratio() const
{
    return (m_value - m_minimum) / qreal(m_maximum - m_minimum);
}

void MoodSliderWidget::geometry(qreal &r, qreal &left, qreal &right, qreal &y) const
{
    const qreal h = height();
    r = clamp((h - 54.0) / 2.0, 12.0, 30.0);
    left = r + 6.0;
    right = std::max(left + 1.0, width() - r - 6.0);
    y = h - r - 8.0;
}

int MoodSliderWidget::valueFromX(qreal x) const
{
    qreal r, left, right, y;
    geometry(r, left, right, y);
    const qreal t = clamp((x - left) / (right - left), 0.0, 1.0);
    return int(std::nearbyint(m_minimum + t * (m_maximum - m_minimum)));
}

// ---------- Public API ----------
QString MoodSliderWidget::getMood() const
{
    const int n = m_labels.size();
    return m_labels.at(std::min(n - 1, int(ratio() * n)));
}

int MoodSliderWidget::getValue() const
{
    return m_value;
}

void MoodSliderWidget::setValue(int value)
{
    value = clampInt(value, m_minimum, m_maximum);
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
        const QString mood = getMood();
        if (mood != m_lastMood) {
            m_lastMood = mood;
            emit moodChanged(mood);
        }
        update();
    }
}

int MoodSliderWidget::getMinimum() const
{
    return m_minimum;
}

void MoodSliderWidget::setMinimum(int minimum)
{
    m_minimum = std::min(minimum, m_maximum - 1);
    setValue(m_value);
    update();
}

int MoodSliderWidget::getMaximum() const
{
    return m_maximum;
}

void MoodSliderWidget::setMaximum(int maximum)
{
    m_maximum = std::max(maximum, m_minimum + 1);
    setValue(m_value);
    update();
}

QStringList MoodSliderWidget::getLabels() const
{
    return m_labels;
}

void MoodSliderWidget::setLabels(const QStringList &labels)
{
    m_labels = labels.isEmpty() ? defaultLabels() : labels;
    update();
}

bool MoodSliderWidget::isShowLabel() const
{
    return m_showLabel;
}

void MoodSliderWidget::setShowLabel(bool showLabel)
{
    m_showLabel = showLabel;
    update();
}

QColor MoodSliderWidget::getTrackColor() const
{
    return m_trackColor;
}

void MoodSliderWidget::setTrackColor(const QColor &color)
{
    if (m_trackColor != color) {
        m_trackColor = color;
        update();
    }
}

// ---------- Qt overrides ----------
QSize MoodSliderWidget::sizeHint() const
{
    return QSize(340, 110);
}

QSize MoodSliderWidget::minimumSizeHint() const
{
    return QSize(120, 70);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void MoodSliderWidget::enterEvent(QEnterEvent *e)
#else
void MoodSliderWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void MoodSliderWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void MoodSliderWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        setValue(valueFromX(eventPos(e).x()));
    }
    QWidget::mousePressEvent(e);
}

void MoodSliderWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging)
        setValue(valueFromX(eventPos(e).x()));
    QWidget::mouseMoveEvent(e);
}

void MoodSliderWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragging = false;
    QWidget::mouseReleaseEvent(e);
}

void MoodSliderWidget::wheelEvent(QWheelEvent *e)
{
    const int step = e->angleDelta().y() > 0 ? 1 : -1;
    setValue(m_value + step * std::max(1, floorDiv(m_maximum - m_minimum, 50)));
    e->accept();
}

void MoodSliderWidget::keyPressEvent(QKeyEvent *e)
{
    const int span = m_maximum - m_minimum;
    switch (e->key()) {
    case Qt::Key_Left:
    case Qt::Key_Down:
        setValue(m_value - 1);
        break;
    case Qt::Key_Right:
    case Qt::Key_Up:
        setValue(m_value + 1);
        break;
    case Qt::Key_PageDown:
        setValue(m_value - std::max(1, floorDiv(span, 10)));
        break;
    case Qt::Key_PageUp:
        setValue(m_value + std::max(1, floorDiv(span, 10)));
        break;
    case Qt::Key_Home:
        setValue(m_minimum);
        break;
    case Qt::Key_End:
        setValue(m_maximum);
        break;
    default:
        QWidget::keyPressEvent(e);
    }
}

void MoodSliderWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void MoodSliderWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void MoodSliderWidget::tick()
{
    const qreal tNow = now();
    const qreal dt = std::min(0.05, std::max(0.0, tNow - m_last));
    m_last = tNow;
    const qreal target = ratio();
    m_vel += ((target - m_shown) * 300.0 - m_vel * 20.0) * dt;
    m_shown += m_vel * dt;
    const bool showBubble = m_showLabel && (m_dragging || m_hovered || hasFocus());
    m_bubble += ((showBubble ? 1.0 : 0.0) - m_bubble) * std::min(1.0, dt * 10.0);
    m_press += ((m_dragging ? 1.0 : 0.0) - m_press) * std::min(1.0, dt * 14.0);
    // Spawn hearts (ecstatic) or steam (furious).
    qreal r, left, right, y;
    geometry(r, left, right, y);
    const qreal t = clamp(m_shown, 0.0, 1.0);
    const qreal cx = lerp(left, right, t);
    qreal rate = 0.0;
    int kind = 0;
    if (t > 0.9) {
        rate = (t - 0.9) * 60.0;
        kind = 0;
    } else if (t < 0.1) {
        rate = (0.1 - t) * 90.0;
        kind = 1;
    }
    m_spawn += rate * dt;
    while (m_spawn > 1.0) {
        m_spawn -= 1.0;
        if (kind == 0) {
            const qreal x = cx + uniform(-r, r);
            const qreal vx = uniform(-15, 15);
            const qreal vy = uniform(-70, -40);
            m_items.append(FloatItem(x, y - r * 0.6, vx, vy, uniform(0.9, 1.4), 0));
        } else {
            const qreal side = uniform(0, 1) < 0.5 ? -1.0 : 1.0;
            const qreal vx = side * uniform(20, 40);
            const qreal vy = uniform(-60, -35);
            m_items.append(FloatItem(cx + side * r * 0.75, y - r * 0.75, vx, vy, uniform(0.6, 0.9), 1));
        }
    }
    QList<FloatItem> alive;
    for (FloatItem it : std::as_const(m_items)) {
        it.x += it.vx * dt + (it.kind == 0 ? std::sin(tNow * 5.0 + it.y * 0.1) * 12.0 * dt : 0.0);
        it.y += it.vy * dt;
        it.life -= dt;
        if (it.life > 0)
            alive.append(it);
    }
    m_items = alive;
    update();
}

// ---------- Painting ----------
void MoodSliderWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    qreal r, left, right, y;
    geometry(r, left, right, y);
    const qreal t = clamp(m_shown, 0.0, 1.0);
    const QColor mc = moodColor(t);
    const qreal cx = lerp(left, right, t);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Track.
    const qreal th = std::max(6.0, r * 0.35);
    const QRectF track(left, y - th / 2.0, right - left, th);
    p.setPen(Qt::NoPen);
    p.setBrush(m_trackColor);
    p.drawRoundedRect(track, th / 2.0, th / 2.0);
    p.setBrush(mc);
    p.drawRoundedRect(QRectF(left, y - th / 2.0, std::max(th, cx - left), th), th / 2.0, th / 2.0);
    // Tick dots.
    const int n = m_labels.size();
    for (int i = 0; i < n; ++i) {
        const qreal tx = lerp(left, right, (i + 0.5) / qreal(n));
        p.setBrush(tx < cx ? QColor(255, 255, 255, 140) : QColor(127, 127, 127, 120));
        p.drawEllipse(QPointF(tx, y), th * 0.18, th * 0.18);
    }
    // Floating hearts / steam (behind face).
    for (const FloatItem &it : std::as_const(m_items)) {
        const qreal a = clamp(it.life / it.maxLife, 0.0, 1.0);
        if (it.kind == 0) {
            QColor c(QStringLiteral("#FF4D6D"));
            c.setAlpha(int(230 * a));
            p.fillPath(heartPath(QPointF(it.x, it.y), r * 0.18 * (1.3 - a * 0.3)), c);
        } else {
            QColor c(200, 200, 210);
            c.setAlpha(int(180 * a));
            p.setBrush(c);
            const qreal rr = r * (0.12 + 0.2 * (1.0 - a));
            p.drawEllipse(QPointF(it.x, it.y), rr, rr);
        }
    }
    // Face.
    p.save();
    const qreal shake = t < 0.1 ? (1.0 - t / 0.1) * 2.5 : 0.0;
    const qreal jx = uniform(-shake, shake);
    const qreal jy = uniform(-shake, shake) * 0.5;
    p.translate(cx + jx, y + jy);
    p.rotate(clamp(-m_vel * 25.0, -25.0, 25.0));
    p.scale(1.0 + 0.1 * m_press, 1.0 - 0.1 * m_press);
    drawFace(p, r, t, mc, tNow);
    p.restore();
    // Mood bubble.
    if (m_bubble > 0.02)
        drawBubble(p, getMood(), cx, y - r - 8.0, mc);
    p.end();
}

void MoodSliderWidget::drawFace(QPainter &p, qreal r, qreal t, const QColor &mc, qreal now)
{
    const QColor ink(QStringLiteral("#2B2B33"));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 45));
    p.drawEllipse(QPointF(0, r * 0.1), r, r);
    QRadialGradient grad(QPointF(-r * 0.35, -r * 0.4), r * 1.7);
    grad.setColorAt(0.0, mc.lighter(130));
    grad.setColorAt(1.0, mc.darker(115));
    p.setBrush(grad);
    p.setPen(QPen(mc.darker(140), std::max(1.0, r * 0.05)));
    p.drawEllipse(QPointF(0, 0), r, r);
    const qreal ex = r * 0.36;
    const qreal ey = -r * 0.12;
    const QPen pen(ink, std::max(1.2, r * 0.08), Qt::SolidLine, Qt::RoundCap);
    // Eyebrows: angry (inner low) -> relaxed -> raised.
    const qreal browTilt = lerp(0.5, -0.15, t) * r;
    const qreal browY = ey - r * lerp(0.28, 0.42, t);
    if (t < 0.5 || t > 0.75) {
        p.setPen(pen);
        for (qreal d : {-1.0, 1.0})
            p.drawLine(QPointF(d * (ex - r * 0.2), browY + browTilt * 0.5),
                       QPointF(d * (ex + r * 0.18), browY - browTilt * 0.5));
    }
    // Eyes.
    if (t > 0.92) {
        const qreal beat = 1.0 + 0.12 * std::sin(now * 10.0);
        for (qreal d : {-1.0, 1.0})
            p.fillPath(heartPath(QPointF(d * ex, ey), r * 0.17 * beat), QColor(QStringLiteral("#E5383B")));
    } else {
        // Pupils slightly squinted when furious, wide when happy.
        const qreal eh = r * lerp(0.08, 0.15, t);
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0}) {
            p.drawEllipse(QPointF(d * ex, ey), r * 0.11, eh);
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(d * ex - r * 0.035, ey - eh * 0.4), r * 0.035, r * 0.035);
            p.setBrush(ink);
        }
    }
    // Mouth: curvature from frown to grin.
    const qreal curve = lerp(-0.32, 0.45, t) * r;
    const qreal mw = r * lerp(0.5, 0.75, t);
    const qreal my = r * lerp(0.42, 0.3, t);
    QPainterPath mouth;
    mouth.moveTo(-mw / 2.0, my);
    mouth.quadTo(0, my + curve, mw / 2.0, my);
    p.setPen(pen);
    if (t > 0.7) {
        mouth.closeSubpath();
        p.setBrush(QColor(QStringLiteral("#8E1B2A")));
    } else {
        p.setBrush(Qt::NoBrush);
    }
    if (0.42 < t && t < 0.58) {
        // "Meh": perfectly flat, slightly off-center.
        p.drawLine(QPointF(-mw / 2.0 + r * 0.05, my + r * 0.02), QPointF(mw / 2.0, my - r * 0.04));
    } else {
        p.drawPath(mouth);
    }
    // Blush when happy.
    if (t > 0.6) {
        QColor bc(QStringLiteral("#FF7A90"));
        bc.setAlpha(int(160 * (t - 0.6) / 0.4));
        p.setPen(Qt::NoPen);
        p.setBrush(bc);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(d * r * 0.62, r * 0.18), r * 0.14, r * 0.08);
    }
}

void MoodSliderWidget::drawBubble(QPainter &p, const QString &text, qreal x, qreal bottom, const QColor &color)
{
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(9.0);
    const QFontMetricsF fm(f);
    const qreal tw = fm.horizontalAdvance(text);
    const qreal bw = tw + 18.0;
    const qreal bh = fm.height() + 8.0;
    const qreal bx = clamp(x - bw / 2.0, 2.0, std::max(2.0, width() - bw - 2.0));
    const qreal by = bottom - bh - 6.0;
    const qreal a = m_bubble;
    p.save();
    p.setOpacity(a);
    p.translate(x, bottom);
    p.scale(0.7 + 0.3 * a, 0.7 + 0.3 * a);
    p.translate(-x, -bottom);
    QPainterPath path;
    path.addRoundedRect(QRectF(bx, by, bw, bh), bh / 2.0, bh / 2.0);
    QPainterPath tail;
    tail.moveTo(x - 5.0, by + bh - 1.0);
    tail.lineTo(x, bottom);
    tail.lineTo(x + 5.0, by + bh - 1.0);
    tail.closeSubpath();
    path = path.united(tail);
    p.setPen(Qt::NoPen);
    p.setBrush(color.darker(125));
    p.drawPath(path);
    p.setFont(f);
    p.setPen(QColor(QStringLiteral("#FFFFFF")));
    p.drawText(QRectF(bx, by, bw, bh), Qt::AlignCenter, text);
    p.restore();
}
