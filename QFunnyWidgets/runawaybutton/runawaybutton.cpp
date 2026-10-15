/**
 * RunawayButtonWidget: a pill button that dodges the cursor with cartoon spring physics, taunts the user in a
 * speech bubble, gets tired after a number of escapes and celebrates with confetti when it is finally caught.
 */
#include "runawaybutton.h"

#include <QFontMetricsF>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace {

const qreal PI = 3.14159265358979323846;

const char *const CONFETTI_COLORS[] = {"#FF4D6D", "#FFD23F", "#3BCEAC", "#5E60CE", "#FF9F1C", "#4CC9F0"};

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

inline int randomIndex(int n)
{
    return int(QRandomGenerator::global()->bounded(n));
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

} // namespace

QStringList RunawayButtonWidget::defaultTaunts()
{
    return {QStringLiteral("Nope!"), QStringLiteral("Too slow!"), QStringLiteral("Missed me!"),
            QStringLiteral("Hehe"), QStringLiteral("Not today!"), QStringLiteral("Catch me if you can!"),
            QStringLiteral("Almost!"), QStringLiteral("Wheee!"), QStringLiteral("Try harder!")};
}

RunawayButtonWidget::RunawayButtonWidget(QWidget *parent)
    : RunawayButtonWidget(Options(), parent)
{
}

RunawayButtonWidget::RunawayButtonWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_text(options.text),
      m_tiredText(options.tiredText),
      m_color(options.color),
      m_textColor(options.textColor),
      m_dodgeRadius(std::max(10.0, qreal(options.dodgeRadius))),
      m_maxEscapes(std::max(0, options.maxEscapes)),
      m_taunts(options.taunts.isEmpty() ? defaultTaunts() : options.taunts),
      m_autoReset(std::max(0, options.autoReset))
{
    if (options.clicked)
        connect(this, &RunawayButtonWidget::clicked, this, options.clicked);
    if (options.escaped)
        connect(this, &RunawayButtonWidget::escaped, this, options.escaped);
    if (options.caught)
        connect(this, &RunawayButtonWidget::caught, this, options.caught);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &RunawayButtonWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal RunawayButtonWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

bool RunawayButtonWidget::isTired() const
{
    return 0 < m_maxEscapes && m_maxEscapes <= m_escapes;
}

QRectF RunawayButtonWidget::buttonRect() const
{
    return QRectF(m_pos.x() - BTN_W / 2.0, m_pos.y() - BTN_H / 2.0, BTN_W, BTN_H);
}

void RunawayButtonWidget::ensurePos()
{
    if (m_pos.x() < 0) {
        const QPointF c(width() / 2.0, height() / 2.0);
        m_pos = c;
        m_target = c;
    }
}

QPointF RunawayButtonWidget::clampCentre(const QPointF &pt) const
{
    const qreal mx = BTN_W / 2.0 + 6.0;
    const qreal my = BTN_H / 2.0 + 30.0;
    return QPointF(clamp(pt.x(), mx, std::max(mx, width() - mx)),
                   clamp(pt.y(), my, std::max(my, height() - my + 22.0)));
}

void RunawayButtonWidget::escape(const QPointF &cursor)
{
    bool found = false;
    QPointF best;
    qreal bestD = -1.0;
    for (int i = 0; i < 14; ++i) {
        const QPointF cand = clampCentre(QPointF(uniform(0, width()), uniform(0, height())));
        const qreal d = std::hypot(cand.x() - cursor.x(), cand.y() - cursor.y());
        const qreal travel = std::hypot(cand.x() - m_pos.x(), cand.y() - m_pos.y());
        const qreal score = d - std::abs(travel - 160.0) * 0.4; // prefer medium hops
        if (score > bestD) {
            bestD = score;
            best = cand;
            found = true;
        }
    }
    if (found)
        m_target = best;
    m_escapes += 1;
    m_taunt = !isTired() ? m_taunts.at(randomIndex(m_taunts.size())) : QStringLiteral("I give up...");
    m_tauntUntil = now() + 1.1;
    emit escaped(m_escapes);
}

void RunawayButtonWidget::burst(const QPointF &c)
{
    for (int i = 0; i < 70; ++i) {
        const qreal a = uniform(0, 2.0 * PI);
        const qreal s = uniform(120, 420);
        Particle pt;
        pt.x = c.x();
        pt.y = c.y();
        pt.vx = std::cos(a) * s;
        pt.vy = std::sin(a) * s - 220;
        pt.rot = uniform(0, 360);
        pt.vrot = uniform(-720, 720);
        pt.color = QColor(QString::fromLatin1(CONFETTI_COLORS[randomIndex(6)]));
        pt.life = uniform(1.0, 1.8);
        m_particles.append(pt);
    }
}

// ---------- Public API ----------
void RunawayButtonWidget::reset()
{
    m_escapes = 0;
    m_caughtAt = -1.0;
    m_taunt.clear();
    update();
}

int RunawayButtonWidget::getEscapes() const
{
    return m_escapes;
}

QString RunawayButtonWidget::getText() const
{
    return m_text;
}

void RunawayButtonWidget::setText(const QString &text)
{
    m_text = text;
    update();
}

QString RunawayButtonWidget::getTiredText() const
{
    return m_tiredText;
}

void RunawayButtonWidget::setTiredText(const QString &tiredText)
{
    m_tiredText = tiredText;
    update();
}

QColor RunawayButtonWidget::getColor() const
{
    return m_color;
}

void RunawayButtonWidget::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QColor RunawayButtonWidget::getTextColor() const
{
    return m_textColor;
}

void RunawayButtonWidget::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        update();
    }
}

qreal RunawayButtonWidget::getDodgeRadius() const
{
    return m_dodgeRadius;
}

void RunawayButtonWidget::setDodgeRadius(qreal dodgeRadius)
{
    m_dodgeRadius = std::max(10.0, dodgeRadius);
}

int RunawayButtonWidget::getMaxEscapes() const
{
    return m_maxEscapes;
}

void RunawayButtonWidget::setMaxEscapes(int maxEscapes)
{
    m_maxEscapes = std::max(0, maxEscapes);
    update();
}

QStringList RunawayButtonWidget::getTaunts() const
{
    return m_taunts;
}

void RunawayButtonWidget::setTaunts(const QStringList &taunts)
{
    m_taunts = taunts.isEmpty() ? defaultTaunts() : taunts;
    update();
}

int RunawayButtonWidget::getAutoReset() const
{
    return m_autoReset;
}

void RunawayButtonWidget::setAutoReset(int autoReset)
{
    m_autoReset = std::max(0, autoReset);
}

// ---------- Qt overrides ----------
QSize RunawayButtonWidget::sizeHint() const
{
    return QSize(460, 260);
}

QSize RunawayButtonWidget::minimumSizeHint() const
{
    return QSize(220, 140);
}

void RunawayButtonWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_pos.x() >= 0) {
        m_pos = clampCentre(m_pos);
        m_target = clampCentre(m_target);
    }
}

void RunawayButtonWidget::mouseMoveEvent(QMouseEvent *e)
{
    ensurePos();
    m_mouse = eventPos(e);
    const QRectF rect = buttonRect();
    m_hover = rect.contains(m_mouse);
    if (m_caughtAt < 0 && !isTired()) {
        // Distance from cursor to the button rectangle (0 when inside).
        const qreal dx = std::max({rect.left() - m_mouse.x(), 0.0, m_mouse.x() - rect.right()});
        const qreal dy = std::max({rect.top() - m_mouse.y(), 0.0, m_mouse.y() - rect.bottom()});
        const bool moving = std::hypot(m_vel.x(), m_vel.y()) > 250.0;
        if (std::hypot(dx, dy) < m_dodgeRadius && !moving)
            escape(m_mouse);
    }
    QWidget::mouseMoveEvent(e);
}

void RunawayButtonWidget::leaveEvent(QEvent *e)
{
    m_hover = false;
    m_mouse = QPointF(-999, -999);
    QWidget::leaveEvent(e);
}

void RunawayButtonWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && buttonRect().contains(eventPos(e)))
        m_pressed = true;
    QWidget::mousePressEvent(e);
}

void RunawayButtonWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressed && buttonRect().contains(eventPos(e))) {
        emit clicked();
        if (m_caughtAt < 0) {
            m_caughtAt = now();
            m_taunt = QStringLiteral("You got me!");
            m_tauntUntil = now() + 2.0;
            burst(m_pos);
            emit caught(m_escapes);
        }
    }
    m_pressed = false;
    QWidget::mouseReleaseEvent(e);
}

void RunawayButtonWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void RunawayButtonWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void RunawayButtonWidget::tick()
{
    ensurePos();
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    // Under-damped spring = cartoon overshoot.
    const qreal k = 160.0;
    const qreal c = 11.0;
    const qreal ax = (m_target.x() - m_pos.x()) * k - m_vel.x() * c;
    const qreal ay = (m_target.y() - m_pos.y()) * k - m_vel.y() * c;
    m_vel = QPointF(m_vel.x() + ax * dt, m_vel.y() + ay * dt);
    m_pos = QPointF(m_pos.x() + m_vel.x() * dt, m_pos.y() + m_vel.y() * dt);
    // Confetti.
    QList<Particle> alive;
    for (Particle pt : std::as_const(m_particles)) {
        pt.vy += 700.0 * dt;
        pt.vx *= 0.99;
        pt.x += pt.vx * dt;
        pt.y += pt.vy * dt;
        pt.rot += pt.vrot * dt;
        pt.life -= dt;
        if (pt.life > 0 && pt.y < height() + 20)
            alive.append(pt);
    }
    m_particles = alive;
    if (m_caughtAt >= 0 && m_autoReset && t - m_caughtAt > m_autoReset / 1000.0)
        reset();
    update();
}

// ---------- Painting ----------
void RunawayButtonWidget::paintEvent(QPaintEvent *)
{
    ensurePos();
    const qreal tNow = now();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const bool tired = isTired();
    const bool isCaught = m_caughtAt >= 0;
    const qreal vx = m_vel.x();
    const qreal vy = m_vel.y();
    const qreal speed = std::hypot(vx, vy);
    // Shadow on the floor.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 40));
    p.drawEllipse(QPointF(m_pos.x(), m_pos.y() + BTN_H * 0.62), BTN_W * 0.42, 6.0);
    p.save();
    p.translate(m_pos);
    // Squash & stretch along the velocity direction.
    const qreal stretch = clamp(speed / 1800.0, 0.0, 0.35);
    const qreal ang = speed > 1 ? degrees(std::atan2(vy, vx)) : 0.0;
    p.rotate(ang);
    p.scale(1.0 + stretch, 1.0 - stretch * 0.6);
    p.rotate(-ang);
    if (tired && !isCaught) {
        p.rotate(6.0 * std::sin(tNow * 2.0)); // sleepy sway
        p.translate(0, 4.0);
    }
    const qreal bob = (tired || isCaught) ? 0.0 : 2.5 * std::sin(tNow * 6.0);
    p.translate(0, bob);
    if (m_pressed)
        p.scale(0.94, 0.9);
    const QRectF rect(-BTN_W / 2.0, -BTN_H / 2.0, BTN_W, BTN_H);
    const QColor base = isCaught ? QColor(QStringLiteral("#3BCEAC")) : (tired ? m_color.darker(115) : m_color);
    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
    grad.setColorAt(0.0, base.lighter(m_hover ? 118 : 108));
    grad.setColorAt(1.0, base.darker(112));
    QPainterPath path;
    path.addRoundedRect(rect, BTN_H / 2.0, BTN_H / 2.0);
    p.fillPath(path, grad);
    p.setPen(QPen(base.darker(140), 1.5));
    p.drawPath(path);
    // Little face on the left of the pill.
    const qreal ex = rect.left() + 26.0;
    const QPointF look(clamp((m_mouse.x() - m_pos.x()) / 80.0, -1.5, 1.5),
                       clamp((m_mouse.y() - m_pos.y()) / 80.0, -1.5, 1.5));
    p.setPen(Qt::NoPen);
    for (qreal dx : {-7.0, 7.0}) {
        if (tired && !isCaught) {
            p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), 2.0, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(ex + dx - 3.0, -4.0), QPointF(ex + dx + 3.0, -4.0));
            p.setPen(Qt::NoPen);
        } else {
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(ex + dx, -4.0), 5.0, 5.0);
            p.setBrush(QColor(QStringLiteral("#1A1A1A")));
            p.drawEllipse(QPointF(ex + dx + look.x() * 1.5, -4.0 + look.y() * 1.5), 2.4, 2.4);
        }
    }
    const QPen mp(QColor(QStringLiteral("#FFFFFF")), 2.0, Qt::SolidLine, Qt::RoundCap);
    p.setPen(mp);
    p.setBrush(Qt::NoBrush);
    if (isCaught)
        p.drawArc(QRectF(ex - 6.0, 0.0, 12.0, 9.0), 180 * 16, 180 * 16);
    else if (tired)
        p.drawEllipse(QPointF(ex, 8.0), 3.0, 3.5);
    else
        p.drawArc(QRectF(ex - 6.0, 1.0, 12.0, 8.0), 200 * 16, 140 * 16);
    // Label.
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(10.5);
    p.setFont(f);
    p.setPen(m_textColor);
    const QString label = isCaught ? QStringLiteral("Yay!") : (tired ? m_tiredText : m_text);
    p.drawText(QRectF(rect.left() + 42.0, rect.top(), rect.width() - 50.0, rect.height()),
               Qt::AlignVCenter | Qt::AlignHCenter, label);
    p.restore();
    // Sweat drops when tired.
    if (tired && !isCaught) {
        for (int i = 0; i < 2; ++i) {
            const qreal t = pyMod(tNow * 0.9 + i * 0.5, 1.0);
            const qreal sx = m_pos.x() + BTN_W * 0.5 - 4.0 + i * 9.0;
            const qreal sy = m_pos.y() - BTN_H * 0.4 + t * 26.0;
            QColor c(QStringLiteral("#7FD6FF"));
            c.setAlpha(int(255 * (1.0 - t)));
            QPainterPath drop;
            drop.moveTo(sx, sy - 6.0);
            drop.quadTo(sx + 4.0, sy, sx, sy + 3.5);
            drop.quadTo(sx - 4.0, sy, sx, sy - 6.0);
            p.fillPath(drop, c);
        }
    }
    // Speech bubble.
    if (tNow < m_tauntUntil && !m_taunt.isEmpty())
        drawBubble(p, m_taunt, tNow);
    // Confetti.
    p.setPen(Qt::NoPen);
    for (const Particle &pt : std::as_const(m_particles)) {
        QColor c(pt.color);
        c.setAlpha(int(255 * clamp(pt.life, 0.0, 1.0)));
        p.save();
        p.translate(pt.x, pt.y);
        p.rotate(pt.rot);
        p.setBrush(c);
        p.drawRect(QRectF(-4, -2, 8, 4));
        p.restore();
    }
    // Escape counter.
    if (m_maxEscapes) {
        QFont f2(font());
        f2.setPointSizeF(8.5);
        p.setFont(f2);
        p.setPen(palette().text().color());
        p.drawText(QRectF(8, height() - 22, width() - 16, 18), Qt::AlignRight | Qt::AlignVCenter,
                   QStringLiteral("Escapes: %1 / %2").arg(std::min(m_escapes, m_maxEscapes)).arg(m_maxEscapes));
    }
    p.end();
}

void RunawayButtonWidget::drawBubble(QPainter &p, const QString &text, qreal now)
{
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(9.5);
    const QFontMetricsF fm(f);
    const qreal tw = fm.horizontalAdvance(text);
    const qreal bw = tw + 22.0;
    const qreal bh = fm.height() + 12.0;
    const qreal left = m_tauntUntil - now;
    const qreal pop = left > 0.2 ? clamp((1.1 - left) * 8.0, 0.0, 1.0) : clamp(left * 5.0, 0.0, 1.0);
    const qreal bx = clamp(m_pos.x() - bw / 2.0, 4.0, width() - bw - 4.0);
    const qreal by = std::max(2.0, m_pos.y() - BTN_H / 2.0 - bh - 14.0);
    p.save();
    p.setOpacity(pop);
    p.translate(bx + bw / 2.0, by + bh);
    p.scale(0.6 + 0.4 * pop, 0.6 + 0.4 * pop);
    p.translate(-(bx + bw / 2.0), -(by + bh));
    QPainterPath path;
    path.addRoundedRect(QRectF(bx, by, bw, bh), 10, 10);
    QPainterPath tail;
    const qreal tx = clamp(m_pos.x(), bx + 14.0, bx + bw - 14.0);
    tail.moveTo(tx - 7.0, by + bh - 1.0);
    tail.lineTo(tx, by + bh + 9.0);
    tail.lineTo(tx + 7.0, by + bh - 1.0);
    tail.closeSubpath();
    path = path.united(tail);
    p.setPen(QPen(QColor(20, 20, 30, 70), 1.2));
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    p.drawPath(path);
    p.setFont(f);
    p.setPen(QColor(QStringLiteral("#22222A")));
    p.drawText(QRectF(bx, by, bw, bh), Qt::AlignCenter, text);
    p.restore();
}
