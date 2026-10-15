/**
 * BatteryBuddyWidget: a battery status indicator with moods: bouncing and energized when full, getting sleepy
 * (heavy eyelids, yawning), sweating when low, flickering and dizzy when almost empty, and happily munching with
 * a lightning bolt and sparks while charging.
 */
#include "batterybuddy.h"

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

BatteryBuddyWidget::BatteryBuddyWidget(QWidget *parent)
    : BatteryBuddyWidget(Options(), parent)
{
}

BatteryBuddyWidget::BatteryBuddyWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_level(clampInt(options.level, 0, 100)),
      m_charging(options.charging),
      m_lowThreshold(clampInt(options.lowThreshold, 1, 99)),
      m_caseColor(options.caseColor),
      m_showLabel(options.showLabel)
{
    if (options.levelChanged)
        connect(this, &BatteryBuddyWidget::levelChanged, this, options.levelChanged);
    if (options.chargingChanged)
        connect(this, &BatteryBuddyWidget::chargingChanged, this, options.chargingChanged);
    if (options.lowBattery)
        connect(this, &BatteryBuddyWidget::lowBattery, this, options.lowBattery);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // State.
    m_shown = m_level / 100.0;
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &BatteryBuddyWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal BatteryBuddyWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void BatteryBuddyWidget::geometry(QRectF &body, qreal &labelTop) const
{
    const qreal w = width();
    const qreal h = height();
    const qreal labelH = m_showLabel ? 42.0 : 0.0;
    const qreal bw = std::max(40.0, std::min(w - 40.0, (h - labelH - 30.0) * 2.0));
    const qreal bh = bw * 0.5;
    body = QRectF((w - bw) / 2.0 - bw * 0.03, (h - labelH - bh) / 2.0 + 6.0, bw, bh);
    labelTop = h - labelH;
}

QString BatteryBuddyWidget::status() const
{
    const int lv = m_level;
    if (m_charging)
        return lv < 100 ? tr("Charging... yum") : tr("Full and smug about it");
    if (lv >= 80)
        return tr("Fully energized");
    if (lv >= 50)
        return tr("Doing fine");
    if (lv >= m_lowThreshold)
        return tr("Getting sleepy");
    if (lv >= 5)
        return tr("Running on fumes");
    return tr("PLUG ME IN!!");
}

// ---------- Public API ----------
int BatteryBuddyWidget::getLevel() const
{
    return m_level;
}

void BatteryBuddyWidget::setLevel(int level)
{
    level = clampInt(level, 0, 100);
    if (level != m_level) {
        const int old = m_level;
        m_level = level;
        emit levelChanged(level);
        if (old >= m_lowThreshold && m_lowThreshold > level)
            emit lowBattery();
        update();
    }
}

bool BatteryBuddyWidget::isCharging() const
{
    return m_charging;
}

void BatteryBuddyWidget::setCharging(bool charging)
{
    if (charging != m_charging) {
        m_charging = charging;
        emit chargingChanged(charging);
        update();
    }
}

int BatteryBuddyWidget::getLowThreshold() const
{
    return m_lowThreshold;
}

void BatteryBuddyWidget::setLowThreshold(int lowThreshold)
{
    m_lowThreshold = clampInt(lowThreshold, 1, 99);
    update();
}

QColor BatteryBuddyWidget::getCaseColor() const
{
    return m_caseColor;
}

void BatteryBuddyWidget::setCaseColor(const QColor &color)
{
    if (m_caseColor != color) {
        m_caseColor = color;
        update();
    }
}

bool BatteryBuddyWidget::isShowLabel() const
{
    return m_showLabel;
}

void BatteryBuddyWidget::setShowLabel(bool showLabel)
{
    m_showLabel = showLabel;
    update();
}

// ---------- Qt overrides ----------
QSize BatteryBuddyWidget::sizeHint() const
{
    return QSize(280, 200);
}

QSize BatteryBuddyWidget::minimumSizeHint() const
{
    return QSize(120, 100);
}

void BatteryBuddyWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void BatteryBuddyWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void BatteryBuddyWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    m_shown += (m_level / 100.0 - m_shown) * std::min(1.0, dt * 6.0);
    if (t >= m_blinkAt + 0.14)
        m_blinkAt = t + uniform(2.0, 4.5);
    if (m_charging) {
        QRectF body;
        qreal top;
        geometry(body, top);
        m_sparkAcc += dt * 14.0;
        while (m_sparkAcc > 1.0) {
            m_sparkAcc -= 1.0;
            const qreal a = uniform(-PI * 0.5, PI * 0.5);
            const qreal sp = uniform(60, 150);
            Spark s;
            s.x = body.right() + body.height() * 0.12;
            s.y = body.center().y() + uniform(-6, 6);
            s.vx = std::cos(a) * sp;
            s.vy = std::sin(a) * sp;
            s.life = uniform(0.2, 0.45);
            m_sparks.append(s);
        }
    }
    QList<Spark> alive;
    for (Spark s : std::as_const(m_sparks)) {
        s.x += s.vx * dt;
        s.y += s.vy * dt;
        s.life -= dt;
        if (s.life > 0)
            alive.append(s);
    }
    m_sparks = alive;
    update();
}

// ---------- Painting ----------
void BatteryBuddyWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QRectF body;
    qreal labelTop;
    geometry(body, labelTop);
    const qreal t = clamp(m_shown, 0.0, 1.0);
    const int lv = m_level;
    const bool chg = m_charging;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    p.save();
    const QPointF c = body.center();
    // Body language: bounce when full, slump when empty.
    if (chg || lv >= 80) {
        p.translate(0, -std::abs(std::sin(tNow * (chg ? 7.0 : 4.0))) * body.height() * 0.06);
    } else if (lv < m_lowThreshold) {
        p.translate(c);
        p.rotate(4.0 + std::sin(tNow * 1.2) * 1.5);
        p.translate(-c);
        p.translate(0, body.height() * 0.05);
    }
    const qreal bh = body.height();
    const qreal rad = bh * 0.2;
    const QColor caseColor = m_caseColor;
    // Shadow.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 45));
    p.drawRoundedRect(body.translated(0, bh * 0.06), rad, rad);
    // Nub.
    p.setBrush(caseColor);
    p.drawRoundedRect(QRectF(body.right() - 2.0, c.y() - bh * 0.18, bh * 0.14, bh * 0.36), bh * 0.05, bh * 0.05);
    // Case interior.
    p.setBrush(caseColor.lighter(165)); // light interior keeps the face readable on any theme
    p.setPen(QPen(caseColor, std::max(2.5, bh * 0.07)));
    p.drawRoundedRect(body, rad, rad);
    // Charge fill.
    const QRectF inner = body.adjusted(bh * 0.1, bh * 0.1, -bh * 0.1, -bh * 0.1);
    QColor fc = lv >= 50 ? QColor(QStringLiteral("#3BCEAC"))
                         : (lv >= m_lowThreshold ? QColor(QStringLiteral("#FFD23F")) : QColor(QStringLiteral("#E63946")));
    if (chg)
        fc = QColor(QStringLiteral("#4CC9F0"));
    const bool flicker = lv < 5 && !chg && std::sin(tNow * 23.0) * std::sin(tNow * 7.0) > 0.3;
    if (!flicker && t > 0.002) {
        const QRectF fill(inner.left(), inner.top(), std::max(inner.height() * 0.15, inner.width() * t), inner.height());
        QLinearGradient grad(fill.topLeft(), fill.bottomLeft());
        grad.setColorAt(0.0, fc.lighter(115));
        grad.setColorAt(1.0, fc.darker(108));
        QPainterPath fp;
        fp.addRoundedRect(fill, rad * 0.6, rad * 0.6);
        p.setPen(Qt::NoPen);
        p.fillPath(fp, grad);
        if (chg) {
            // Shimmer sweeping through the charge.
            p.save();
            p.setClipPath(fp);
            const qreal sx = fill.left() + pyMod(tNow * 0.8, 1.0) * (fill.width() + 60.0) - 30.0;
            QLinearGradient sh(QPointF(sx - 30, 0), QPointF(sx + 30, 0));
            sh.setColorAt(0.0, QColor(255, 255, 255, 0));
            sh.setColorAt(0.5, QColor(255, 255, 255, 110));
            sh.setColorAt(1.0, QColor(255, 255, 255, 0));
            p.fillRect(fill, sh);
            p.restore();
        }
    }
    drawFace(p, QPointF(inner.center().x(), inner.center().y()), bh * 0.5, lv, chg, tNow);
    p.restore();
    // Bolt + sparks while charging.
    if (chg) {
        const qreal pulse = 1.0 + 0.12 * std::sin(tNow * 8.0);
        drawBolt(p, QPointF(body.right() + bh * 0.02, body.top() - bh * 0.18), bh * 0.28 * pulse);
        for (const Spark &s : std::as_const(m_sparks)) {
            QColor sc(QStringLiteral("#FFD23F"));
            sc.setAlpha(int(255 * clamp(s.life / 0.45, 0.0, 1.0)));
            p.setPen(QPen(sc, 2.0, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(s.x, s.y), QPointF(s.x - s.vx * 0.04, s.y - s.vy * 0.04));
        }
    }
    // Low battery sweat.
    if (!chg && 5 <= lv && lv < m_lowThreshold) {
        const qreal k = pyMod(tNow * 0.9, 1.0);
        QColor dc(QStringLiteral("#7FD6FF"));
        dc.setAlpha(int(255 * (1.0 - k)));
        const qreal x = body.left() + bh * 0.1;
        const qreal y = body.top() - bh * 0.05 + k * bh * 0.4;
        QPainterPath drop;
        drop.moveTo(x, y - 6.0);
        drop.quadTo(x + 4.0, y + 1.0, x, y + 3.5);
        drop.quadTo(x - 4.0, y + 1.0, x, y - 6.0);
        p.fillPath(drop, dc);
    }
    // Labels.
    if (m_showLabel) {
        const QColor tc(palette().text().color());
        QFont f(font());
        f.setBold(true);
        f.setPointSizeF(12.0);
        p.setFont(f);
        p.setPen(tc);
        p.drawText(QRectF(0, labelTop, width(), 22), Qt::AlignCenter, QStringLiteral("%1%").arg(lv));
        QFont f2(font());
        f2.setPointSizeF(8.5);
        const bool critical = lv < 5 && !chg;
        f2.setBold(critical);
        p.setFont(f2);
        QColor sc = critical ? QColor(QStringLiteral("#E63946")) : QColor(tc);
        if (!critical)
            sc.setAlpha(165);
        p.setPen(sc);
        p.drawText(QRectF(0, labelTop + 21, width(), 18), Qt::AlignCenter, status());
    }
    p.end();
}

void BatteryBuddyWidget::drawFace(QPainter &p, const QPointF &c, qreal r, int lv, bool chg, qreal now)
{
    const QColor ink(QStringLiteral("#1E1E28"));
    const QPen pen(ink, std::max(1.5, r * 0.09), Qt::SolidLine, Qt::RoundCap);
    const qreal ex = r * 0.42;
    const qreal ey = c.y() - r * 0.15;
    const qreal my = c.y() + r * 0.32;
    const qreal er = r * 0.13;
    const bool blinking = m_blinkAt <= now && now < m_blinkAt + 0.14;
    qreal yawn = 0.0;
    qreal lidY = 0.0;
    if (!chg && m_lowThreshold <= lv && lv < 50) {
        const qreal ph = pyMod(now, 4.5);
        yawn = ph < 1.0 ? std::sin(ph / 1.0 * PI) : 0.0;
    }
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    // Eyes.
    for (qreal d : {-1.0, 1.0}) {
        const qreal x = c.x() + d * ex;
        if (chg) {
            p.drawArc(QRectF(x - er, ey - er * 0.6, er * 2.0, er * 1.4), 20 * 16, 140 * 16);
        } else if (lv < 5) {
            const qreal k = er * 0.8;
            p.drawLine(QPointF(x - k, ey - k), QPointF(x + k, ey + k));
            p.drawLine(QPointF(x - k, ey + k), QPointF(x + k, ey - k));
        } else if (blinking || yawn > 0.3) {
            p.drawLine(QPointF(x - er, ey), QPointF(x + er, ey));
        } else {
            const qreal big = er * (lv >= 80 ? 1.15 : 1.0);
            QPainterPath eye;
            eye.addEllipse(QPointF(x, ey), big, big * 1.1);
            if (lv < 50) {
                // Heavy eyelids when tired: keep only the part below the lid line.
                const qreal lid = lv >= m_lowThreshold ? 0.5 : 0.7;
                QPainterPath cut;
                lidY = ey - big * 1.1 + big * 2.2 * lid;
                cut.addRect(QRectF(x - big * 1.2, lidY, big * 2.4, big * 2.4));
                eye = eye.intersected(cut);
            }
            p.setPen(Qt::NoPen);
            p.fillPath(eye, ink);
            if (lv >= 50) {
                p.setBrush(QColor(QStringLiteral("#FFFFFF")));
                p.drawEllipse(QPointF(x - big * 0.3, ey - big * 0.35), big * 0.32, big * 0.32);
            } else {
                p.setPen(pen);
                p.drawLine(QPointF(x - big * 1.15, lidY), QPointF(x + big * 1.15, lidY));
            }
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
        }
    }
    // Mouth.
    p.setPen(pen);
    if (chg) {
        const qreal munch = std::abs(std::sin(now * 9.0));
        p.setBrush(QColor(QStringLiteral("#7A2E2E")));
        p.drawEllipse(QPointF(c.x(), my), r * 0.2, r * (0.06 + 0.14 * munch));
    } else if (lv < 5) {
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(c.x() - r * 0.2, my), QPointF(c.x() + r * 0.2, my));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FF7A90")));
        p.drawEllipse(QPointF(c.x() + r * 0.08, my + r * 0.1), r * 0.08, r * 0.1);
    } else if (yawn > 0.05) {
        p.setBrush(QColor(QStringLiteral("#7A2E2E")));
        p.drawEllipse(QPointF(c.x(), my), r * 0.14, r * 0.2 * yawn + r * 0.02);
    } else if (lv >= 80) {
        p.setBrush(QColor(QStringLiteral("#7A2E2E")));
        QPainterPath m;
        m.moveTo(c.x() - r * 0.3, my - r * 0.08);
        m.quadTo(c.x(), my + r * 0.4, c.x() + r * 0.3, my - r * 0.08);
        m.closeSubpath();
        p.drawPath(m);
    } else if (lv >= 50) {
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(c.x() - r * 0.24, my - r * 0.22, r * 0.48, r * 0.3), 200 * 16, 140 * 16);
    } else if (lv >= m_lowThreshold) {
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(c.x() - r * 0.18, my), QPointF(c.x() + r * 0.18, my));
    } else {
        p.setBrush(Qt::NoBrush);
        QPainterPath m;
        m.moveTo(c.x() - r * 0.22, my);
        for (int k = 1; k < 7; ++k)
            m.lineTo(c.x() - r * 0.22 + k * r * 0.44 / 6.0, my + ((k % 2) ? r * 0.05 : -r * 0.05));
        p.drawPath(m);
    }
}

void BatteryBuddyWidget::drawBolt(QPainter &p, const QPointF &c, qreal s)
{
    QPainterPath bolt;
    static const qreal pts[6][2] = {{0.15, -1.0}, {-0.55, 0.15}, {-0.05, 0.15}, {-0.2, 1.0}, {0.55, -0.2}, {0.05, -0.2}};
    for (int i = 0; i < 6; ++i) {
        const QPointF pt(c.x() + pts[i][0] * s, c.y() + pts[i][1] * s);
        if (i == 0)
            bolt.moveTo(pt);
        else
            bolt.lineTo(pt);
    }
    bolt.closeSubpath();
    p.setPen(QPen(QColor(QStringLiteral("#B8860B")), 1.5));
    p.setBrush(QColor(QStringLiteral("#FFD23F")));
    p.drawPath(bolt);
}
