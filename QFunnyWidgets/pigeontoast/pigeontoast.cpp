/**
 * PigeonToast: a toast notification overlay delivered by a carrier pigeon. Attach it to a parent widget; it covers
 * the parent (following its size), and for each message a pigeon flies in carrying a swinging banner, hovers
 * while the message is shown, drops a few feathers and flies away. Messages are queued.
 */
#include "pigeontoast.h"

#include <QEvent>
#include <QFontMetricsF>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

const qreal PI = 3.14159265358979323846;

inline qreal clamp(qreal x, qreal a, qreal b)
{
    return x < a ? a : (x > b ? b : x);
}

inline qreal uniform(qreal a, qreal b)
{
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}

inline qreal easeOutBack(qreal t)
{
    const qreal c1 = 1.4;
    t -= 1.0;
    return 1.0 + (c1 + 1.0) * t * t * t + c1 * t * t;
}

inline qreal easeInCubic(qreal t)
{
    return t * t * t;
}

} // namespace

PigeonToast::PigeonToast(QWidget *parent)
    : PigeonToast(Options(), parent)
{
}

PigeonToast::PigeonToast(QWidget *parent, const Options &options)
    : PigeonToast(options, parent)
{
}

PigeonToast::PigeonToast(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_duration(std::max(300, options.duration)),
      m_bannerColor(options.bannerColor),
      m_textColor(options.textColor),
      m_pigeonColor(options.pigeonColor),
      m_caption(options.caption),
      m_margin(std::max(0, options.margin))
{
    if (options.messageShown)
        connect(this, &PigeonToast::messageShown, this, options.messageShown);
    if (options.messageFinished)
        connect(this, &PigeonToast::messageFinished, this, options.messageFinished);
    // Overlay flags.
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::NoFocus);
    if (parentWidget() != nullptr) {
        parentWidget()->installEventFilter(this);
        setGeometry(parentWidget()->rect());
    }
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PigeonToast::tick);
    hide();
}

// ---------- Helpers ----------
qreal PigeonToast::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void PigeonToast::bannerSize(const QString &text, qreal &bw, qreal &bh, qreal &mh) const
{
    QFont f(font());
    f.setPointSizeF(10.0);
    const QFontMetricsF fm(f);
    const qreal maxW = clamp(width() * 0.45, 140.0, 280.0);
    const QRectF br = fm.boundingRect(QRectF(0, 0, maxW, 1000), Qt::TextWordWrap, text);
    const qreal capH = !m_caption.isEmpty() ? 16.0 : 0.0;
    bw = br.width() + 28.0;
    bh = br.height() + capH + 20.0;
    mh = br.height();
}

void PigeonToast::next()
{
    if (m_queue.isEmpty()) {
        m_hasCurrent = false;
        if (m_feathers.isEmpty()) {
            m_timer->stop();
            hide();
        }
        return;
    }
    m_current = m_queue.takeFirst();
    m_hasCurrent = true;
    m_start = now();
    m_holdEnd = m_start + FLY_IN + m_current.second / 1000.0;
    m_dropped = false;
    m_pos = QPointF(-999, -999);
}

QPointF PigeonToast::path(qreal now, Phase &phase) const
{
    qreal bw, bh, mh;
    bannerSize(m_current.first, bw, bh, mh);
    const qreal w = width();
    const qreal s = 46.0;
    const QPointF land(w - m_margin - bw / 2.0, m_margin + s * 0.5);
    const QPointF startPt(w + bw / 2.0 + 40.0, land.y() + 30.0);
    const QPointF endPt(-bw / 2.0 - 60.0, land.y() - 20.0);
    const qreal t = now - m_start;
    if (t < FLY_IN) {
        const qreal k = easeOutBack(t / FLY_IN);
        const qreal arc = std::sin(t / FLY_IN * PI) * -40.0;
        phase = In;
        return QPointF(startPt.x() + (land.x() - startPt.x()) * k, startPt.y() + (land.y() - startPt.y()) * k + arc);
    }
    if (now < m_holdEnd) {
        phase = Hold;
        return QPointF(land.x(), land.y() + std::sin(now * 3.0) * 4.0);
    }
    const qreal t2 = (now - m_holdEnd) / FLY_OUT;
    if (t2 < 1.0) {
        const qreal k = easeInCubic(t2);
        phase = Out;
        return QPointF(land.x() + (endPt.x() - land.x()) * k,
                       land.y() + (endPt.y() - land.y()) * k - std::sin(t2 * PI) * 30.0);
    }
    phase = Done;
    return endPt;
}

// ---------- Public API ----------
void PigeonToast::showMessage(const QString &text, int duration)
{
    m_queue.append(qMakePair(text, duration < 0 ? m_duration : duration));
    if (parentWidget() != nullptr)
        setGeometry(parentWidget()->rect());
    show();
    raise();
    if (!m_timer->isActive()) {
        m_last = now();
        m_timer->start(16);
    }
    if (!m_hasCurrent)
        next();
}

void PigeonToast::dismiss()
{
    const qreal t = now();
    if (m_hasCurrent && t < m_holdEnd) {
        m_holdEnd = std::max(t, m_start + FLY_IN);
        update();
    }
}

int PigeonToast::getPending() const
{
    return int(m_queue.size()) + (m_hasCurrent ? 1 : 0);
}

int PigeonToast::getDuration() const { return m_duration; }
QColor PigeonToast::getBannerColor() const { return m_bannerColor; }
QColor PigeonToast::getTextColor() const { return m_textColor; }
QColor PigeonToast::getPigeonColor() const { return m_pigeonColor; }
QString PigeonToast::getCaption() const { return m_caption; }
int PigeonToast::getMargin() const { return m_margin; }

void PigeonToast::setDuration(int duration)
{
    m_duration = std::max(300, duration);
}

void PigeonToast::setBannerColor(const QColor &color)
{
    m_bannerColor = color;
    update();
}

void PigeonToast::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

void PigeonToast::setPigeonColor(const QColor &color)
{
    m_pigeonColor = color;
    update();
}

void PigeonToast::setCaption(const QString &caption)
{
    m_caption = caption;
}

void PigeonToast::setMargin(int margin)
{
    m_margin = std::max(0, margin);
}

// ---------- Qt overrides ----------
bool PigeonToast::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parentWidget() && event->type() == QEvent::Resize)
        setGeometry(parentWidget()->rect());
    return QWidget::eventFilter(obj, event);
}

// ---------- Animation tick ----------
void PigeonToast::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.001, t - m_last));
    m_last = t;
    if (m_hasCurrent) {
        Phase phase;
        const QPointF pos = path(t, phase);
        if (m_pos.x() > -900) {
            const qreal vx = (pos.x() - m_pos.x()) / dt;
            m_vx += (vx - m_vx) * std::min(1.0, dt * 10.0);
        }
        m_pos = pos;
        if ((phase == In || phase == Hold) && !m_dropped && t - m_start > FLY_IN * 0.85) {
            m_dropped = true;
            emit messageShown(m_current.first);
            for (int i = 0; i < 3; ++i) {
                const qreal x = pos.x() + uniform(-15, 15);
                const qreal y = pos.y() + uniform(-5, 10);
                const qreal life = uniform(1.8, 2.6);
                const qreal ph = uniform(0, 2 * PI);
                m_feathers.append(Feather(x, y, life, ph));
            }
        }
        if (phase == Done) {
            emit messageFinished(m_current.first);
            next();
        }
    }
    // Banner swings like a pendulum, pushed by the flight speed.
    m_swingVel += ((-m_vx * 0.03 - m_swing) * 60.0 - m_swingVel * 4.0) * dt;
    m_swing = clamp(m_swing + m_swingVel * dt, -25.0, 25.0);
    QList<Feather> alive;
    for (Feather f : std::as_const(m_feathers)) {
        f.y += 28.0 * dt;
        f.x += std::sin(t * 2.5 + f.phase) * 30.0 * dt;
        f.life -= dt;
        if (f.life > 0)
            alive.append(f);
    }
    m_feathers = alive;
    if (!m_hasCurrent && m_feathers.isEmpty()) {
        m_timer->stop();
        hide();
    }
    update();
}

// ---------- Painting ----------
void PigeonToast::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Feathers.
    for (const Feather &f : std::as_const(m_feathers)) {
        const qreal a = clamp(f.life / f.maxLife, 0.0, 1.0);
        p.save();
        p.translate(f.x, f.y);
        p.rotate(std::sin(tNow * 2.5 + f.phase) * 35.0);
        p.setOpacity(a);
        drawFeather(p);
        p.restore();
    }
    if (m_hasCurrent && m_pos.x() > -900) {
        const QPointF pos = m_pos;
        drawBanner(p, pos, m_current.first);
        const qreal flap = std::abs(m_vx) > 30 ? 20.0 : 13.0;
        drawPigeon(p, pos, 46.0, std::sin(tNow * flap), m_vx);
    }
    p.end();
}

void PigeonToast::drawBanner(QPainter &p, const QPointF &pos, const QString &text)
{
    qreal bw, bh, mh;
    bannerSize(text, bw, bh, mh);
    const QPointF hang(pos.x(), pos.y() + 20.0);
    p.save();
    p.translate(hang);
    p.rotate(m_swing);
    const qreal top = 22.0;
    const QRectF rect(-bw / 2.0, top, bw, bh);
    // Strings.
    p.setPen(QPen(QColor(120, 110, 100), 1.2));
    p.drawLine(QPointF(-4.0, 0), QPointF(rect.left() + 16.0, rect.top()));
    p.drawLine(QPointF(4.0, 0), QPointF(rect.right() - 16.0, rect.top()));
    // Shadow + banner.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 50));
    p.drawRoundedRect(rect.translated(0, 4), 10, 10);
    p.setBrush(m_bannerColor);
    p.setPen(QPen(QColor(0, 0, 0, 35), 1.0));
    p.drawRoundedRect(rect, 10, 10);
    p.setBrush(QColor(QStringLiteral("#C47F3A")));
    p.setPen(Qt::NoPen);
    for (qreal x : {rect.left() + 16.0, rect.right() - 16.0})
        p.drawEllipse(QPointF(x, rect.top() + 4.0), 2.6, 2.6);
    qreal y = rect.top() + 10.0;
    if (!m_caption.isEmpty()) {
        QFont cf(font());
        cf.setPointSizeF(7.5);
        cf.setItalic(true);
        p.setFont(cf);
        QColor cc(m_textColor);
        cc.setAlpha(130);
        p.setPen(cc);
        p.drawText(QRectF(rect.left() + 14.0, y, bw - 28.0, 14.0), Qt::AlignLeft | Qt::AlignVCenter, m_caption);
        y += 16.0;
    }
    QFont mf(font());
    mf.setPointSizeF(10.0);
    p.setFont(mf);
    p.setPen(m_textColor);
    p.drawText(QRectF(rect.left() + 14.0, y, bw - 28.0, mh + 2.0), Qt::TextWordWrap, text);
    p.restore();
}

void PigeonToast::drawPigeon(QPainter &p, const QPointF &pos, qreal s, qreal flap, qreal vx)
{
    const QColor pc = m_pigeonColor;
    const QColor ink(QStringLiteral("#1E1E24"));
    p.save();
    p.translate(pos);
    p.rotate(clamp(vx * 0.02, -12.0, 12.0));
    const QPen outline(pc.darker(150), 1.2);
    // Feet holding the strings.
    p.setPen(QPen(QColor(QStringLiteral("#E58C8A")), 2.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(-4.0, s * 0.25), QPointF(-4.0, s * 0.44));
    p.drawLine(QPointF(4.0, s * 0.25), QPointF(4.0, s * 0.44));
    // Far wing (behind body).
    p.setPen(outline);
    p.setBrush(pc.darker(120));
    drawWing(p, s, -flap * 0.8 + 0.3, true);
    // Tail.
    QPainterPath tail;
    tail.moveTo(s * 0.3, -s * 0.02);
    tail.lineTo(s * 0.72, -s * 0.12);
    tail.lineTo(s * 0.75, s * 0.12);
    tail.closeSubpath();
    p.setBrush(pc.darker(130));
    p.drawPath(tail);
    // Body.
    QLinearGradient grad(QPointF(0, -s * 0.3), QPointF(0, s * 0.3));
    grad.setColorAt(0.0, pc.lighter(110));
    grad.setColorAt(1.0, pc.darker(105));
    p.setBrush(grad);
    p.drawEllipse(QPointF(0, s * 0.02), s * 0.42, s * 0.26);
    // Iridescent neck.
    QLinearGradient neck(QPointF(-s * 0.4, -s * 0.2), QPointF(-s * 0.15, s * 0.1));
    neck.setColorAt(0.0, QColor(QStringLiteral("#7A5BA8")));
    neck.setColorAt(1.0, QColor(QStringLiteral("#3E9C7A")));
    p.setPen(Qt::NoPen);
    p.setBrush(neck);
    p.drawEllipse(QPointF(-s * 0.3, -s * 0.06), s * 0.15, s * 0.17);
    // Head.
    p.setPen(outline);
    p.setBrush(pc.lighter(105));
    const QPointF hc(-s * 0.42, -s * 0.22);
    p.drawEllipse(hc, s * 0.17, s * 0.16);
    // Beak.
    QPainterPath beak;
    beak.moveTo(hc.x() - s * 0.14, hc.y() - s * 0.02);
    beak.lineTo(hc.x() - s * 0.3, hc.y() + s * 0.03);
    beak.lineTo(hc.x() - s * 0.14, hc.y() + s * 0.06);
    beak.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#3A3A40")));
    p.drawPath(beak);
    p.setBrush(QColor(QStringLiteral("#F2F2F2")));
    p.drawEllipse(QPointF(hc.x() - s * 0.13, hc.y() - s * 0.01), s * 0.035, s * 0.025);
    // Eye: orange ring, slightly crazed.
    p.setBrush(QColor(QStringLiteral("#F28C28")));
    p.drawEllipse(QPointF(hc.x() - s * 0.03, hc.y() - s * 0.03), s * 0.055, s * 0.055);
    p.setBrush(ink);
    p.drawEllipse(QPointF(hc.x() - s * 0.04, hc.y() - s * 0.03), s * 0.028, s * 0.028);
    // Near wing (in front).
    p.setPen(outline);
    p.setBrush(pc);
    drawWing(p, s, flap, false);
    p.restore();
}

void PigeonToast::drawWing(QPainter &p, qreal s, qreal flap, bool farWing)
{
    p.save();
    p.translate(s * 0.02, -s * (!farWing ? 0.1 : 0.14));
    p.rotate(-25.0 - flap * 45.0);
    QPainterPath wing;
    wing.moveTo(-s * 0.15, 0);
    wing.cubicTo(-s * 0.1, -s * 0.35, s * 0.35, -s * 0.5, s * 0.5, -s * 0.42);
    wing.cubicTo(s * 0.35, -s * 0.2, s * 0.2, -s * 0.02, s * 0.15, 0);
    wing.closeSubpath();
    p.drawPath(wing);
    p.restore();
}

void PigeonToast::drawFeather(QPainter &p)
{
    const QColor pc = m_pigeonColor;
    QPainterPath path;
    path.moveTo(0, -8.0);
    path.quadTo(5.0, 0, 0, 8.0);
    path.quadTo(-5.0, 0, 0, -8.0);
    p.setPen(QPen(pc.darker(140), 0.8));
    p.setBrush(pc.lighter(115));
    p.drawPath(path);
    p.drawLine(QPointF(0, -8.0), QPointF(0, 10.0));
}
