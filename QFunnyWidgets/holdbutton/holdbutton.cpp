/**
 * NervousHoldButtonWidget: a hold-to-confirm button for destructive actions. While held it fills up, its face
 * panics more and more (wider eyes, shaking, sweat, screaming) and its label escalates; at 100 % it faints.
 * Releasing early shows relief.
 */
#include "holdbutton.h"

#include <QCursor>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
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

} // namespace

QStringList NervousHoldButtonWidget::defaultStages()
{
    return {QStringLiteral("Hold to delete"), QStringLiteral("Are you sure?"), QStringLiteral("Really, really sure?"),
            QStringLiteral("No going back!!")};
}

NervousHoldButtonWidget::NervousHoldButtonWidget(QWidget *parent)
    : NervousHoldButtonWidget(Options(), parent)
{
}

NervousHoldButtonWidget::NervousHoldButtonWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_stages(options.stages.isEmpty() ? defaultStages() : options.stages),
      m_doneText(options.doneText),
      m_reliefText(options.reliefText),
      m_holdDuration(std::max(200, options.holdDuration)),
      m_color(options.color),
      m_textColor(options.textColor),
      m_autoReset(std::max(0, options.autoReset))
{
    if (options.confirmed)
        connect(this, &NervousHoldButtonWidget::confirmed, this, options.confirmed);
    if (options.cancelled)
        connect(this, &NervousHoldButtonWidget::cancelled, this, options.cancelled);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &NervousHoldButtonWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal NervousHoldButtonWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

bool NervousHoldButtonWidget::isFainted() const
{
    return m_faintAt >= 0;
}

QRectF NervousHoldButtonWidget::buttonRect() const
{
    const qreal w = width();
    const qreal h = height();
    const qreal bw = std::min(w - 24.0, 280.0);
    const qreal bh = std::min(h - 24.0, 58.0);
    return QRectF((w - bw) / 2.0, (h - bh) / 2.0, bw, bh);
}

QString NervousHoldButtonWidget::label(qreal now) const
{
    if (isFainted())
        return m_doneText;
    if (now < m_reliefUntil && !m_holding)
        return m_reliefText;
    if (!m_holding)
        return m_stages.first();
    const int n = m_stages.size();
    return m_stages.at(clampInt(int(m_progress * n), 0, n - 1));
}

void NervousHoldButtonWidget::press()
{
    if (!isFainted()) {
        m_holding = true;
        m_squashVel -= 4.0;
    }
}

void NervousHoldButtonWidget::release()
{
    if (m_holding) {
        m_holding = false;
        if (!isFainted() && m_progress > 0.02) {
            m_reliefUntil = now() + 1.2;
            m_squashVel += 6.0;
            emit cancelled(m_progress);
        }
    }
}

// ---------- Public API ----------
void NervousHoldButtonWidget::reset()
{
    m_faintAt = -1.0;
    m_progress = 0.0;
    m_holding = false;
    update();
}

qreal NervousHoldButtonWidget::getProgress() const
{
    return m_progress;
}

QStringList NervousHoldButtonWidget::getStages() const
{
    return m_stages;
}

void NervousHoldButtonWidget::setStages(const QStringList &stages)
{
    m_stages = stages.isEmpty() ? defaultStages() : stages;
    update();
}

QString NervousHoldButtonWidget::getDoneText() const
{
    return m_doneText;
}

void NervousHoldButtonWidget::setDoneText(const QString &doneText)
{
    m_doneText = doneText;
    update();
}

QString NervousHoldButtonWidget::getReliefText() const
{
    return m_reliefText;
}

void NervousHoldButtonWidget::setReliefText(const QString &reliefText)
{
    m_reliefText = reliefText;
    update();
}

int NervousHoldButtonWidget::getHoldDuration() const
{
    return m_holdDuration;
}

void NervousHoldButtonWidget::setHoldDuration(int holdDuration)
{
    m_holdDuration = std::max(200, holdDuration);
}

QColor NervousHoldButtonWidget::getColor() const
{
    return m_color;
}

void NervousHoldButtonWidget::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QColor NervousHoldButtonWidget::getTextColor() const
{
    return m_textColor;
}

void NervousHoldButtonWidget::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        update();
    }
}

int NervousHoldButtonWidget::getAutoReset() const
{
    return m_autoReset;
}

void NervousHoldButtonWidget::setAutoReset(int autoReset)
{
    m_autoReset = std::max(0, autoReset);
}

// ---------- Qt overrides ----------
QSize NervousHoldButtonWidget::sizeHint() const
{
    return QSize(320, 110);
}

QSize NervousHoldButtonWidget::minimumSizeHint() const
{
    return QSize(180, 60);
}

void NervousHoldButtonWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void NervousHoldButtonWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    release();
    QWidget::hideEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void NervousHoldButtonWidget::enterEvent(QEnterEvent *e)
#else
void NervousHoldButtonWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void NervousHoldButtonWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void NervousHoldButtonWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && buttonRect().contains(eventPos(e)))
        press();
    QWidget::mousePressEvent(e);
}

void NervousHoldButtonWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        release();
    QWidget::mouseReleaseEvent(e);
}

void NervousHoldButtonWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        if (!e->isAutoRepeat())
            press();
        return;
    }
    QWidget::keyPressEvent(e);
}

void NervousHoldButtonWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        if (!e->isAutoRepeat())
            release();
        return;
    }
    QWidget::keyReleaseEvent(e);
}

// ---------- Animation tick ----------
void NervousHoldButtonWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    if (isFainted()) {
        if (m_autoReset && t - m_faintAt > m_autoReset / 1000.0)
            reset();
    } else if (m_holding) {
        m_progress = std::min(1.0, m_progress + dt * 1000.0 / m_holdDuration);
        if (m_progress >= 1.0) {
            m_holding = false;
            m_faintAt = t;
            m_squashVel += 10.0;
            emit confirmed();
        }
    } else {
        m_progress = std::max(0.0, m_progress - dt * 3.0);
    }
    m_squashVel += (-m_squash * 260.0 - m_squashVel * 12.0) * dt;
    m_squash += m_squashVel * dt;
    m_hover += ((m_hovered ? 1.0 : 0.0) - m_hover) * std::min(1.0, dt * 10.0);
    // Sweat.
    const QRectF rect = buttonRect();
    if (m_holding && m_progress > 0.25) {
        m_dropAcc += dt * m_progress * 14.0;
        while (m_dropAcc > 1.0) {
            m_dropAcc -= 1.0;
            const qreal side = uniform(0, 1) < 0.5 ? -1.0 : 1.0;
            Drop d;
            d.x = side < 0 ? rect.left() + 4.0 : rect.right() - 4.0;
            d.y = rect.top() + uniform(2, 14);
            d.vx = side * uniform(30, 80);
            d.vy = -uniform(60, 120);
            d.life = uniform(0.5, 0.8);
            m_drops.append(d);
        }
    }
    QList<Drop> alive;
    for (Drop d : std::as_const(m_drops)) {
        d.vy += 520.0 * dt;
        d.x += d.vx * dt;
        d.y += d.vy * dt;
        d.life -= dt;
        if (d.life > 0)
            alive.append(d);
    }
    m_drops = alive;
    update();
}

// ---------- Painting ----------
void NervousHoldButtonWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const QRectF rect = buttonRect();
    const qreal prog = m_progress;
    const bool fainted = isFainted();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const QPointF c = rect.center();
    // Shadow.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 50));
    p.drawRoundedRect(rect.translated(0, 5).adjusted(6, 0, -6, 0), rect.height() / 2.0, rect.height() / 2.0);
    p.save();
    p.translate(c);
    // Shake grows with fear (quadratic, so it starts calm).
    const qreal shake = m_holding ? prog * prog * 5.0 : 0.0;
    const qreal jx = uniform(-shake, shake);
    const qreal jy = uniform(-shake, shake) * 0.6 - 2.0 * m_hover;
    p.translate(jx, jy);
    if (fainted) {
        const qreal t = clamp((tNow - m_faintAt) / 0.35, 0.0, 1.0);
        p.translate(0, 6.0 * t);
        p.rotate(-7.0 * t);
    }
    const qreal sq = clamp(m_squash, -0.3, 0.3);
    p.scale(1.0 + sq * 0.5, 1.0 - sq);
    p.translate(-c);
    const qreal r = rect.height() / 2.0;
    QPainterPath body;
    body.addRoundedRect(rect, r, r);
    const QColor base = fainted ? QColor(QStringLiteral("#7A7F8E")) : m_color;
    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
    grad.setColorAt(0.0, base.lighter(115));
    grad.setColorAt(1.0, base.darker(110));
    p.fillPath(body, grad);
    // Hold fill.
    if (prog > 0 && !fainted) {
        p.save();
        p.setClipPath(body);
        const QRectF fill(rect.left(), rect.top(), rect.width() * prog, rect.height());
        p.fillRect(fill, base.darker(150));
        p.restore();
    }
    p.setPen(QPen(base.darker(140), 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawPath(body);
    drawFace(p, rect, prog, fainted, tNow);
    // Label.
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(clamp(rect.height() * 0.2, 8.0, 12.0));
    p.setFont(f);
    p.setPen(m_textColor);
    p.drawText(QRectF(rect.left() + r * 1.9, rect.top(), rect.width() - r * 2.3, rect.height()),
               Qt::AlignVCenter | Qt::AlignHCenter, label(tNow));
    p.restore();
    // Sweat drops.
    for (const Drop &d : std::as_const(m_drops)) {
        QColor col(QStringLiteral("#7FD6FF"));
        col.setAlpha(int(255 * clamp(d.life / 0.6, 0.0, 1.0)));
        QPainterPath drop;
        drop.moveTo(d.x, d.y - 5.0);
        drop.quadTo(d.x + 3.5, d.y + 1.0, d.x, d.y + 3.0);
        drop.quadTo(d.x - 3.5, d.y + 1.0, d.x, d.y - 5.0);
        p.fillPath(drop, col);
    }
    p.end();
}

void NervousHoldButtonWidget::drawFace(QPainter &p, const QRectF &rect, qreal prog, bool fainted, qreal now)
{
    const qreal r = rect.height() / 2.0;
    const qreal fx = rect.left() + r * 1.05;
    const qreal fy = rect.center().y();
    const QColor white(QStringLiteral("#FFFFFF"));
    const QColor ink(QStringLiteral("#1E1E24"));
    const qreal er = r * (0.17 + 0.1 * prog); // eyes widen with fear
    const qreal ex = r * 0.3;
    const qreal ey = fy - r * 0.15;
    const QPen pen(white, std::max(1.5, r * 0.08), Qt::SolidLine, Qt::RoundCap);
    if (fainted) {
        p.setPen(pen);
        for (qreal d : {-1.0, 1.0}) {
            const qreal x = fx + d * ex;
            const qreal k = r * 0.13;
            p.drawLine(QPointF(x - k, ey - k), QPointF(x + k, ey + k));
            p.drawLine(QPointF(x - k, ey + k), QPointF(x + k, ey - k));
        }
        // Tongue out.
        p.drawLine(QPointF(fx - r * 0.2, fy + r * 0.32), QPointF(fx + r * 0.2, fy + r * 0.32));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FF8FA3")));
        p.drawEllipse(QPointF(fx + r * 0.08, fy + r * 0.42), r * 0.1, r * 0.12);
        return;
    }
    // Eyes look at the cursor while calm, dart around when scared.
    const QPoint cur = mapFromGlobal(QCursor::pos());
    qreal lx = clamp((cur.x() - fx) / 120.0, -1.0, 1.0);
    qreal ly = clamp((cur.y() - fy) / 120.0, -1.0, 1.0);
    if (m_holding && prog > 0.4) {
        lx = std::sin(now * 17.0);
        ly = 0.0;
    }
    for (qreal d : {-1.0, 1.0}) {
        const qreal x = fx + d * ex;
        p.setPen(Qt::NoPen);
        p.setBrush(white);
        p.drawEllipse(QPointF(x, ey), er, er);
        const qreal pr = er * (0.55 - 0.25 * prog); // pupils shrink
        p.setBrush(ink);
        p.drawEllipse(QPointF(x + lx * (er - pr) * 0.8, ey + ly * (er - pr) * 0.8), pr, pr);
    }
    // Worried brows.
    if (prog > 0.15) {
        const QPen bp(white, std::max(1.2, r * 0.07), Qt::SolidLine, Qt::RoundCap);
        p.setPen(bp);
        const qreal lift = er + r * 0.12;
        for (qreal d : {-1.0, 1.0}) {
            const qreal x = fx + d * ex;
            p.drawLine(QPointF(x + d * er * 0.8, ey - lift + r * 0.04), QPointF(x - d * er * 0.6, ey - lift - r * 0.08 * prog));
        }
    }
    // Mouth: smile -> wobbly line -> open scream.
    p.setPen(pen);
    const qreal my = fy + r * 0.38;
    const qreal mw = r * 0.26;
    if (prog < 0.05) {
        const bool relieved = now < m_reliefUntil;
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(fx - mw, my - r * 0.22, mw * 2.0, r * 0.3), 200 * 16, 140 * 16);
        if (relieved)
            p.drawLine(QPointF(fx + r * 0.62, fy - r * 0.55), QPointF(fx + r * 0.5, fy - r * 0.35));
    } else if (prog < 0.7) {
        QPainterPath mouth;
        mouth.moveTo(fx - mw, my);
        const int steps = 6;
        for (int k = 1; k <= steps; ++k) {
            const qreal x = fx - mw + 2.0 * mw * k / qreal(steps);
            const qreal y = my + ((k % 2) ? r * 0.05 : -r * 0.05) * (1.0 + std::sin(now * 30.0));
            mouth.lineTo(x, y);
        }
        p.setBrush(Qt::NoBrush);
        p.drawPath(mouth);
    } else {
        p.setBrush(ink);
        const qreal open = r * (0.12 + 0.05 * std::sin(now * 25.0));
        p.drawEllipse(QPointF(fx, my), r * 0.13, open);
    }
}
