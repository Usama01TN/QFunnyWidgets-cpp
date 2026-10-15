/**
 * HamsterLoaderWidget: a progress indicator powered by a hamster running in a wheel. The hamster speeds up as
 * progress grows, sweats near the end, celebrates with confetti when done and shows rotating funny messages.
 * A negative progress means indeterminate.
 */
#include "hamsterloader.h"

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

const char *const CONFETTI_COLORS[] = {"#FF4D6D", "#FFD23F", "#3BCEAC", "#5E60CE", "#FF9F1C", "#4CC9F0"};

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

/** Python's round(): half to even. */
inline int pyRound(qreal x)
{
    return int(std::nearbyint(x));
}

} // namespace

QStringList HamsterLoaderWidget::defaultMessages()
{
    return {QStringLiteral("Feeding the hamster..."), QStringLiteral("Reticulating splines..."),
            QStringLiteral("Bribing the CPU..."), QStringLiteral("Asking the cloud nicely..."),
            QStringLiteral("Hamster on coffee break..."), QStringLiteral("Counting to infinity (twice)..."),
            QStringLiteral("Untangling the internet..."), QStringLiteral("Almost there (probably)...")};
}

HamsterLoaderWidget::HamsterLoaderWidget(QWidget *parent)
    : HamsterLoaderWidget(Options(), parent)
{
}

HamsterLoaderWidget::HamsterLoaderWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_progress(std::min(1.0, qreal(options.progress))),
      m_speed(std::max(0.0, qreal(options.speed))),
      m_hamsterColor(options.hamsterColor),
      m_wheelColor(options.wheelColor),
      m_accentColor(options.accentColor),
      m_messages(options.messages.isEmpty() ? defaultMessages() : options.messages),
      m_messageInterval(std::max(300, options.messageInterval)),
      m_doneText(options.doneText),
      m_showPercent(options.showPercent)
{
    if (options.finished)
        connect(this, &HamsterLoaderWidget::finished, this, options.finished);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // State.
    m_shownProgress = std::max(0.0, m_progress);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HamsterLoaderWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal HamsterLoaderWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

bool HamsterLoaderWidget::isDone() const
{
    return m_doneAt >= 0;
}

void HamsterLoaderWidget::geometry(QPointF &c, qreal &R, QRectF &bar) const
{
    const qreal w = width();
    const qreal h = height();
    const qreal barH = 12.0;
    const qreal textH = 44.0;
    R = std::max(10.0, std::min(w * 0.32, (h - barH - textH - 24.0) / 2.0));
    c = QPointF(w / 2.0, 10.0 + R);
    const qreal bw = std::min(w - 32.0, std::max(160.0, R * 3.2));
    bar = QRectF((w - bw) / 2.0, c.y() + R + 18.0, bw, barH);
}

// ---------- Public API ----------
void HamsterLoaderWidget::reset()
{
    m_doneAt = -1.0;
    m_progress = 0.0;
    m_shownProgress = 0.0;
    m_particles.clear();
    emit progressChanged(0.0);
    update();
}

qreal HamsterLoaderWidget::getProgress() const
{
    return m_progress;
}

void HamsterLoaderWidget::setProgress(qreal progress)
{
    progress = std::min(1.0, progress);
    if (progress != m_progress) {
        if (progress < 1.0 && isDone())
            m_doneAt = -1.0;
        m_progress = progress;
        emit progressChanged(progress);
        update();
    }
}

qreal HamsterLoaderWidget::getSpeed() const
{
    return m_speed;
}

void HamsterLoaderWidget::setSpeed(qreal speed)
{
    m_speed = std::max(0.0, speed);
}

QColor HamsterLoaderWidget::getHamsterColor() const
{
    return m_hamsterColor;
}

void HamsterLoaderWidget::setHamsterColor(const QColor &color)
{
    if (m_hamsterColor != color) {
        m_hamsterColor = color;
        update();
    }
}

QColor HamsterLoaderWidget::getWheelColor() const
{
    return m_wheelColor;
}

void HamsterLoaderWidget::setWheelColor(const QColor &color)
{
    if (m_wheelColor != color) {
        m_wheelColor = color;
        update();
    }
}

QColor HamsterLoaderWidget::getAccentColor() const
{
    return m_accentColor;
}

void HamsterLoaderWidget::setAccentColor(const QColor &color)
{
    if (m_accentColor != color) {
        m_accentColor = color;
        update();
    }
}

QStringList HamsterLoaderWidget::getMessages() const
{
    return m_messages;
}

void HamsterLoaderWidget::setMessages(const QStringList &messages)
{
    m_messages = messages.isEmpty() ? defaultMessages() : messages;
}

int HamsterLoaderWidget::getMessageInterval() const
{
    return m_messageInterval;
}

void HamsterLoaderWidget::setMessageInterval(int messageInterval)
{
    m_messageInterval = std::max(300, messageInterval);
}

QString HamsterLoaderWidget::getDoneText() const
{
    return m_doneText;
}

void HamsterLoaderWidget::setDoneText(const QString &doneText)
{
    m_doneText = doneText;
    update();
}

bool HamsterLoaderWidget::isShowPercent() const
{
    return m_showPercent;
}

void HamsterLoaderWidget::setShowPercent(bool showPercent)
{
    m_showPercent = showPercent;
    update();
}

// ---------- Qt overrides ----------
QSize HamsterLoaderWidget::sizeHint() const
{
    return QSize(300, 260);
}

QSize HamsterLoaderWidget::minimumSizeHint() const
{
    return QSize(180, 160);
}

void HamsterLoaderWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void HamsterLoaderWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void HamsterLoaderWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const bool indeterminate = m_progress < 0;
    // Displayed progress eases toward the real one.
    if (!indeterminate) {
        m_shownProgress += (m_progress - m_shownProgress) * std::min(1.0, dt * 6.0);
        if (m_progress >= 1.0 && !isDone() && m_shownProgress > 0.995) {
            m_shownProgress = 1.0;
            m_doneAt = t;
            QPointF c;
            qreal R;
            QRectF bar;
            geometry(c, R, bar);
            for (int i = 0; i < 80; ++i) {
                const qreal a = uniform(-PI, 0);
                const qreal s = uniform(150, 420);
                Particle pt;
                pt.x = c.x();
                pt.y = c.y() + R * 0.5;
                pt.vx = std::cos(a) * s;
                pt.vy = std::sin(a) * s;
                pt.rot = uniform(0, 360);
                pt.vrot = uniform(-720, 720);
                pt.color = QColor(QString::fromLatin1(
                    CONFETTI_COLORS[QRandomGenerator::global()->bounded(6)]));
                pt.life = uniform(1.2, 2.2);
                pt.kind = 0;
                m_particles.append(pt);
            }
            emit finished();
        }
    }
    // Running speed: faster towards the end, panics a little at 90 %+.
    const qreal p = indeterminate ? 0.5 : m_shownProgress;
    const qreal target = isDone() ? 0.0 : m_speed * (1.0 + 1.6 * p);
    m_runSpeed += (target - m_runSpeed) * std::min(1.0, dt * 3.0);
    m_wheelAngle = pyMod(m_wheelAngle + m_runSpeed * 160.0 * dt, 360.0);
    m_runPhase += m_runSpeed * 14.0 * dt;
    // Sweat drops when working hard.
    if (!isDone() && p > 0.6) {
        m_sweatAcc += dt * (p - 0.6) * 18.0;
        while (m_sweatAcc > 1.0) {
            m_sweatAcc -= 1.0;
            const QPointF hp = m_hamsterPos;
            Particle pt;
            pt.x = hp.x() + uniform(-4, 8);
            pt.y = hp.y() - 8;
            pt.vx = uniform(-90, -20);
            pt.vy = uniform(-160, -80);
            pt.rot = 0;
            pt.vrot = 0;
            pt.color = QColor(QStringLiteral("#7FD6FF"));
            pt.life = 0.7;
            pt.kind = 1;
            m_particles.append(pt);
        }
    }
    QList<Particle> alive;
    for (Particle pt : std::as_const(m_particles)) {
        pt.vy += (pt.kind == 0 ? 650.0 : 500.0) * dt;
        pt.x += pt.vx * dt;
        pt.y += pt.vy * dt;
        pt.rot += pt.vrot * dt;
        pt.life -= dt;
        if (pt.life > 0)
            alive.append(pt);
    }
    m_particles = alive;
    update();
}

// ---------- Painting ----------
void HamsterLoaderWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF c;
    qreal R;
    QRectF bar;
    geometry(c, R, bar);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const QColor wc = m_wheelColor;
    // Stand.
    p.setPen(QPen(wc.darker(130), std::max(3.0, R * 0.06), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(c, QPointF(c.x() - R * 0.55, c.y() + R + 6.0));
    p.drawLine(c, QPointF(c.x() + R * 0.55, c.y() + R + 6.0));
    // Wheel back rim, spokes and rungs (rotating).
    p.save();
    p.translate(c);
    p.rotate(m_wheelAngle);
    const QPen spoke(wc, std::max(1.5, R * 0.025));
    p.setPen(spoke);
    for (int i = 0; i < 8; ++i) {
        const qreal a = i * PI / 4.0;
        p.drawLine(QPointF(0, 0), QPointF(std::cos(a) * R, std::sin(a) * R));
    }
    const QPen rung(wc.lighter(115), std::max(1.5, R * 0.035), Qt::SolidLine, Qt::RoundCap);
    p.setPen(rung);
    for (int i = 0; i < 24; ++i) {
        const qreal a = i * 2.0 * PI / 24.0;
        p.drawLine(QPointF(std::cos(a) * (R - 5.0), std::sin(a) * (R - 5.0)), QPointF(std::cos(a) * R, std::sin(a) * R));
    }
    p.restore();
    const QPen rim(wc, std::max(3.0, R * 0.07));
    p.setPen(rim);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(c, R, R);
    p.setPen(Qt::NoPen);
    p.setBrush(wc.darker(140));
    p.drawEllipse(c, R * 0.08, R * 0.08);
    // Hamster.
    drawHamster(p, c, R, tNow);
    // Particles.
    p.setPen(Qt::NoPen);
    for (const Particle &pt : std::as_const(m_particles)) {
        QColor col(pt.color);
        col.setAlpha(int(255 * clamp(pt.life * 1.5, 0.0, 1.0)));
        if (pt.kind == 0) {
            p.save();
            p.translate(pt.x, pt.y);
            p.rotate(pt.rot);
            p.setBrush(col);
            p.drawRect(QRectF(-4, -2, 8, 4));
            p.restore();
        } else {
            QPainterPath drop;
            drop.moveTo(pt.x, pt.y - 4.0);
            drop.quadTo(pt.x + 3.0, pt.y + 1.0, pt.x, pt.y + 2.5);
            drop.quadTo(pt.x - 3.0, pt.y + 1.0, pt.x, pt.y - 4.0);
            p.fillPath(drop, col);
        }
    }
    // Progress bar.
    const bool indeterminate = m_progress < 0;
    QPainterPath track;
    track.addRoundedRect(bar, bar.height() / 2.0, bar.height() / 2.0);
    p.fillPath(track, QColor(127, 127, 127, 60));
    p.save();
    p.setClipPath(track);
    QRectF fill;
    if (indeterminate) {
        const qreal seg = bar.width() * 0.3;
        const qreal x = bar.left() - seg + pyMod(tNow * 0.8, 1.0) * (bar.width() + seg);
        fill = QRectF(x, bar.top(), seg, bar.height());
    } else {
        fill = QRectF(bar.left(), bar.top(), bar.width() * m_shownProgress, bar.height());
    }
    QLinearGradient grad(fill.topLeft(), fill.topRight());
    const QColor ac = isDone() ? QColor(QStringLiteral("#3BCEAC")) : m_accentColor;
    grad.setColorAt(0.0, ac.lighter(125));
    grad.setColorAt(1.0, ac);
    QPainterPath fp;
    fp.addRoundedRect(fill, bar.height() / 2.0, bar.height() / 2.0);
    p.fillPath(fp, grad);
    // Moving stripes on the filled part.
    p.setClipPath(fp, Qt::IntersectClip);
    const QPen sp(QColor(255, 255, 255, 50), 4.0);
    p.setPen(sp);
    const qreal off = pyMod(tNow * 30.0, 14.0);
    qreal x = bar.left() - 14.0 + off;
    while (x < bar.right() + 14.0) {
        p.drawLine(QPointF(x, bar.bottom()), QPointF(x + 10.0, bar.top()));
        x += 14.0;
    }
    p.restore();
    // Texts.
    const QColor txt = palette().text().color();
    QFont f(font());
    f.setPointSizeF(9.5);
    p.setFont(f);
    QString msg;
    if (isDone()) {
        msg = m_doneText;
    } else {
        const int idx = int((tNow * 1000.0) / m_messageInterval) % m_messages.size();
        msg = m_messages.at(idx);
    }
    QColor sub(txt);
    sub.setAlpha(200);
    p.setPen(sub);
    p.drawText(QRectF(0, bar.bottom() + 6.0, width(), 20.0), Qt::AlignHCenter | Qt::AlignTop, msg);
    if (m_showPercent && !indeterminate) {
        f.setBold(true);
        p.setFont(f);
        p.setPen(txt);
        p.drawText(QRectF(bar.right() - 60.0, bar.top() - 20.0, 60.0, 18.0), Qt::AlignRight | Qt::AlignVCenter,
                   QStringLiteral("%1%").arg(pyRound(m_shownProgress * 100)));
    }
    p.end();
}

void HamsterLoaderWidget::drawHamster(QPainter &p, const QPointF &c, qreal R, qreal now)
{
    const qreal s = R / 70.0; // scale everything to the wheel
    const bool done = isDone();
    const qreal ph = m_runPhase;
    const QColor hc = m_hamsterColor;
    const QColor ink(QStringLiteral("#2B2B33"));
    qreal jump = 0.0;
    if (done) {
        const qreal t = now - m_doneAt;
        jump = std::abs(std::sin(t * 6.0)) * 22.0 * s * std::max(0.0, 1.0 - t / 3.0) + (t < 3.0 ? 6.0 * s : 0.0);
    }
    const qreal bob = !done ? std::abs(std::sin(ph)) * 3.0 * s : 0.0;
    const QPointF base(c.x(), c.y() + R - 26.0 * s - bob - jump);
    m_hamsterPos = QPointF(base.x() + 18.0 * s, base.y() - 4.0 * s);
    p.save();
    p.translate(base);
    const qreal lean = done ? 0.0 : clamp(m_runSpeed * 4.0, 0.0, 12.0);
    p.rotate(lean);
    // Legs (behind body).
    const QPen legPen(hc.darker(150), 4.0 * s, Qt::SolidLine, Qt::RoundCap);
    p.setPen(legPen);
    const qreal legs[2][2] = {{-12.0, 0.0}, {12.0, PI}};
    for (const auto &leg : legs) {
        const qreal lx = leg[0];
        const qreal off = leg[1];
        const qreal a = std::sin(ph + off) * (!done ? 0.9 : 0.0);
        p.drawLine(QPointF(lx * s, 10.0 * s), QPointF(lx * s + std::sin(a) * 11.0 * s, 10.0 * s + std::cos(a) * 14.0 * s));
    }
    // Tail.
    p.setPen(QPen(hc.darker(120), 3.0 * s, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(-24.0 * s, 0), QPointF(-30.0 * s, -3.0 * s + std::sin(ph * 2.0) * 2.0 * s));
    // Body.
    p.setPen(QPen(hc.darker(135), 1.5 * s));
    p.setBrush(hc);
    p.drawEllipse(QPointF(0, 0), 26.0 * s, 18.0 * s);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#FFF1E0")));
    p.drawEllipse(QPointF(6.0 * s, 6.0 * s), 15.0 * s, 9.0 * s);
    // Head.
    const qreal hx = 22.0 * s;
    const qreal hy = -8.0 * s;
    p.setPen(QPen(hc.darker(135), 1.5 * s));
    p.setBrush(hc);
    p.drawEllipse(QPointF(hx - 6.0 * s, hy - 13.0 * s), 6.0 * s, 6.0 * s); // ear
    p.drawEllipse(QPointF(hx, hy), 14.0 * s, 12.0 * s);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#FFB3C1")));
    p.drawEllipse(QPointF(hx - 6.0 * s, hy - 13.0 * s), 3.0 * s, 3.0 * s);
    p.drawEllipse(QPointF(hx + 3.0 * s, hy + 5.0 * s), 4.0 * s, 2.5 * s); // cheek
    p.setBrush(ink);
    p.drawEllipse(QPointF(hx + 13.0 * s, hy + 1.0 * s), 2.2 * s, 2.0 * s); // nose
    if (done) {
        // Happy ^ ^ eyes.
        p.setPen(QPen(ink, 1.8 * s, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(hx + 1.0 * s, hy - 2.0 * s), QPointF(hx + 4.0 * s, hy - 5.0 * s));
        p.drawLine(QPointF(hx + 4.0 * s, hy - 5.0 * s), QPointF(hx + 7.0 * s, hy - 2.0 * s));
    } else {
        p.drawEllipse(QPointF(hx + 4.0 * s, hy - 3.0 * s), 2.6 * s, 2.8 * s);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(hx + 4.8 * s, hy - 4.0 * s), 0.9 * s, 0.9 * s);
        // Determined eyebrow when going fast.
        if (m_runSpeed > 1.8) {
            p.setPen(QPen(ink, 1.5 * s, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(hx + 1.0 * s, hy - 8.5 * s), QPointF(hx + 7.0 * s, hy - 6.5 * s));
        }
    }
    // Whiskers.
    p.setPen(QPen(QColor(60, 60, 70, 150), 0.8 * s));
    for (qreal dy : {-1.5, 1.5})
        p.drawLine(QPointF(hx + 12.0 * s, hy + 2.0 * s), QPointF(hx + 21.0 * s, hy + 2.0 * s + dy * 2.0 * s));
    p.restore();
}
