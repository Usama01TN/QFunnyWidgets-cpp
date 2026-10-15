/**
 * RobotCheckWidget: an "I'm not a robot" checkbox panel with a nervous robot hiding behind it. The robot peeks out
 * now and then, sweats oil and calculates while you are being checked, winks with a thumbs-up when you pass and
 * is delighted ("One of us!") when you fail.
 */
#include "robotcheck.h"

#include <QFontMetricsF>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
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

inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

} // namespace

RobotCheckWidget::RobotCheckWidget(QWidget *parent)
    : RobotCheckWidget(Options(), parent)
{
}

RobotCheckWidget::RobotCheckWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_text(options.text),
      m_verifiedText(options.verifiedText),
      m_failChance(clamp(options.failChance, 0.0, 1.0)),
      m_panelColor(options.panelColor),
      m_textColor(options.textColor),
      m_robotColor(options.robotColor)
{
    if (options.verified)
        connect(this, &RobotCheckWidget::verified, this, options.verified);
    if (options.failed)
        connect(this, &RobotCheckWidget::failed, this, options.failed);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &RobotCheckWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal RobotCheckWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void RobotCheckWidget::setMode(Mode mode)
{
    m_mode = mode;
    m_modeAt = now();
    update();
}

QRectF RobotCheckWidget::panel() const
{
    const qreal w = width();
    const qreal h = height();
    const qreal pw = std::min(w - 16.0, 330.0);
    const qreal ph = std::min(78.0, h * 0.5);
    return QRectF((w - pw) / 2.0, h - ph - 8.0, pw, ph);
}

// ---------- Public API ----------
void RobotCheckWidget::verify()
{
    if (m_mode == IDLE || m_mode == FAILED)
        setMode(CHECKING);
}

void RobotCheckWidget::reset()
{
    setMode(IDLE);
}

bool RobotCheckWidget::isVerified() const { return m_mode == VERIFIED; }
QString RobotCheckWidget::getText() const { return m_text; }
QString RobotCheckWidget::getVerifiedText() const { return m_verifiedText; }
qreal RobotCheckWidget::getFailChance() const { return m_failChance; }
QColor RobotCheckWidget::getPanelColor() const { return m_panelColor; }
QColor RobotCheckWidget::getTextColor() const { return m_textColor; }
QColor RobotCheckWidget::getRobotColor() const { return m_robotColor; }

void RobotCheckWidget::setText(const QString &text)
{
    m_text = text;
    update();
}

void RobotCheckWidget::setVerifiedText(const QString &verifiedText)
{
    m_verifiedText = verifiedText;
    update();
}

void RobotCheckWidget::setFailChance(qreal failChance)
{
    m_failChance = clamp(failChance, 0.0, 1.0);
    update();
}

void RobotCheckWidget::setPanelColor(const QColor &color)
{
    m_panelColor = color;
    update();
}

void RobotCheckWidget::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

void RobotCheckWidget::setRobotColor(const QColor &color)
{
    m_robotColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize RobotCheckWidget::sizeHint() const
{
    return QSize(360, 190);
}

QSize RobotCheckWidget::minimumSizeHint() const
{
    return QSize(240, 140);
}

void RobotCheckWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void RobotCheckWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void RobotCheckWidget::mouseReleaseEvent(QMouseEvent *e)
{
    const QRectF pnl = panel();
    const QRectF hit(pnl.left(), pnl.top(), pnl.width() * 0.7, pnl.height());
    if (e->button() == Qt::LeftButton && hit.contains(eventPos(e)))
        verify();
    QWidget::mouseReleaseEvent(e);
}

void RobotCheckWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        verify();
        return;
    }
    QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void RobotCheckWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal since = t - m_modeAt;
    const Mode mode = m_mode;
    if (mode == CHECKING && since >= CHECK_TIME) {
        if (QRandomGenerator::global()->generateDouble() < m_failChance) {
            setMode(FAILED);
            emit failed();
        } else {
            setMode(VERIFIED);
            emit verified();
        }
    } else if (mode == FAILED && since > 2.6) {
        setMode(IDLE);
    }
    // How far the robot pokes out from behind the panel.
    qreal target;
    if (mode == CHECKING)
        target = 1.0;
    else if (mode == VERIFIED)
        target = since < 2.6 ? 1.0 : 0.12;
    else if (mode == FAILED)
        target = 0.75;
    else
        target = pyMod(t, 6.0) > 4.8 ? 0.32 : 0.12; // occasional peek
    m_riseVel += ((target - m_rise) * 160.0 - m_riseVel * 14.0) * dt;
    m_rise += m_riseVel * dt;
    update();
}

// ---------- Painting ----------
void RobotCheckWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const QRectF pnl = panel();
    const Mode mode = m_mode;
    const qreal since = tNow - m_modeAt;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Robot first, so the panel hides its lower half.
    const qreal s = std::min(pnl.height() * 1.05, (pnl.top() - 4.0) * 0.95);
    const qreal rx = pnl.left() + pnl.width() * 0.63;
    const qreal ry = pnl.top() + s * 0.55 - m_rise * s * 0.95;
    drawRobot(p, QPointF(rx, ry), s, mode, since, tNow);
    // Panel.
    p.setPen(QPen(QColor(0, 0, 0, 40), 1.0));
    p.setBrush(m_panelColor);
    p.drawRoundedRect(pnl, 6, 6);
    // Robot hands gripping the panel edge while it peeks.
    if (m_rise > 0.25) {
        p.setPen(QPen(m_robotColor.darker(140), 1.2));
        p.setBrush(m_robotColor);
        for (qreal d : {-1.0, 1.0}) {
            if (mode == VERIFIED && d > 0 && since < 2.6)
                continue;
            p.drawRoundedRect(QRectF(rx + d * s * 0.42 - s * 0.08, pnl.top() - s * 0.05, s * 0.16, s * 0.1), 3, 3);
        }
    }
    // Checkbox.
    const qreal bs = std::min(28.0, pnl.height() * 0.38);
    const QRectF box(pnl.left() + 16.0, pnl.center().y() - bs / 2.0, bs, bs);
    m_boxRect = box;
    const QColor tc = m_textColor;
    if (mode == CHECKING) {
        p.setPen(QPen(QColor(QStringLiteral("#4A90E2")), 3.0, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        const int start = int(pyMod(tNow * 540.0, 360.0)) * 16;
        const int span = int(90 + 120 * std::abs(std::sin(tNow * 3.0))) * 16;
        p.drawArc(box.adjusted(2, 2, -2, -2), start, span);
    } else if (mode == VERIFIED) {
        const qreal k = clamp(since / 0.35, 0.0, 1.0);
        p.setPen(QPen(QColor(QStringLiteral("#1FA463")), 3.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPointF a(box.left() + bs * 0.18, box.top() + bs * 0.52);
        const QPointF b(box.left() + bs * 0.42, box.top() + bs * 0.76);
        const QPointF c(box.left() + bs * 0.86, box.top() + bs * 0.2);
        if (k < 0.4) {
            const qreal t = k / 0.4;
            p.drawLine(a, QPointF(a.x() + (b.x() - a.x()) * t, a.y() + (b.y() - a.y()) * t));
        } else {
            const qreal t = (k - 0.4) / 0.6;
            QPainterPath path;
            path.moveTo(a);
            path.lineTo(b);
            path.lineTo(QPointF(b.x() + (c.x() - b.x()) * t, b.y() + (c.y() - b.y()) * t));
            p.setBrush(Qt::NoBrush);
            p.drawPath(path);
        }
    } else {
        QColor border = mode == FAILED ? QColor(QStringLiteral("#E63946")) : QColor(tc);
        if (mode != FAILED)
            border.setAlpha(120);
        p.setPen(QPen(border, 2.0));
        p.setBrush(m_panelColor.lightness() > 128 ? QColor(QStringLiteral("#FFFFFF")) : m_panelColor.lighter(130));
        p.drawRoundedRect(box, 3, 3);
        if (mode == FAILED) {
            p.setPen(QPen(QColor(QStringLiteral("#E63946")), 3.0, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(box.topLeft() + QPointF(7, 7), box.bottomRight() - QPointF(7, 7));
            p.drawLine(QPointF(box.right() - 7, box.top() + 7), QPointF(box.left() + 7, box.bottom() - 7));
        }
    }
    // Label.
    QFont f(font());
    f.setPointSizeF(10.5);
    p.setFont(f);
    QString label;
    if (mode == VERIFIED)
        label = m_verifiedText;
    else if (mode == FAILED)
        label = tr("Suspiciously robotic. Try again.");
    else
        label = m_text;
    p.setPen(mode == FAILED ? QColor(QStringLiteral("#E63946")) : tc);
    p.drawText(QRectF(box.right() + 12.0, pnl.top(), pnl.width() * 0.62 - bs - 28.0, pnl.height()),
               Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, label);
    // Logo block.
    const qreal lx = pnl.right() - pnl.width() * 0.24;
    const QPointF lc(lx + pnl.width() * 0.12, pnl.top() + pnl.height() * 0.36);
    const qreal lr = pnl.height() * 0.17;
    p.setPen(QPen(QColor(QStringLiteral("#4A90E2")), std::max(2.0, lr * 0.3), Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    const qreal rot = mode == CHECKING ? tNow * 40.0 : 0.0;
    for (int k = 0; k < 3; ++k) {
        const int st = int((rot + k * 120 + 20) * 16);
        p.drawArc(QRectF(lc.x() - lr, lc.y() - lr, lr * 2, lr * 2), st, 80 * 16);
    }
    QFont f2(font());
    f2.setPointSizeF(7.0);
    p.setFont(f2);
    QColor sub(tc);
    sub.setAlpha(150);
    p.setPen(sub);
    p.drawText(QRectF(lx, lc.y() + lr + 2.0, pnl.width() * 0.24, 12.0), Qt::AlignCenter, QStringLiteral("notCAPTCHA"));
    f2.setPointSizeF(6.0);
    p.setFont(f2);
    p.drawText(QRectF(lx, lc.y() + lr + 13.0, pnl.width() * 0.24, 10.0), Qt::AlignCenter, tr("Privacy - Terms"));
    p.end();
}

void RobotCheckWidget::drawRobot(QPainter &p, const QPointF &c, qreal s, Mode mode, qreal since, qreal now)
{
    const QColor rc = m_robotColor;
    const QColor dark(QStringLiteral("#2B2F3A"));
    const qreal hw = s * 0.8;
    const qreal hh = s * 0.6;
    const QRectF head(c.x() - hw / 2.0, c.y() - hh / 2.0, hw, hh);
    // Antenna.
    p.setPen(QPen(rc.darker(140), std::max(1.5, s * 0.03)));
    p.drawLine(QPointF(c.x(), head.top()), QPointF(c.x(), head.top() - s * 0.2));
    const bool on = pyMod(now, 1.0) < 0.5 || mode == CHECKING;
    QColor bulb = on ? QColor(QStringLiteral("#E63946")) : QColor(QStringLiteral("#7A2A30"));
    if (mode == VERIFIED)
        bulb = QColor(QStringLiteral("#3BCEAC"));
    p.setPen(Qt::NoPen);
    p.setBrush(bulb);
    p.drawEllipse(QPointF(c.x(), head.top() - s * 0.22), s * 0.055, s * 0.055);
    // Head + ear bolts.
    p.setPen(QPen(rc.darker(150), std::max(1.0, s * 0.02)));
    p.setBrush(rc);
    for (qreal d : {-1.0, 1.0})
        p.drawRoundedRect(QRectF(c.x() + d * hw / 2.0 - s * 0.05, c.y() - s * 0.08, s * 0.1, s * 0.16), 2, 2);
    p.drawRoundedRect(head, s * 0.12, s * 0.12);
    // Visor with LED eyes.
    const QRectF visor(head.left() + hw * 0.12, head.top() + hh * 0.18, hw * 0.76, hh * 0.38);
    p.setPen(Qt::NoPen);
    p.setBrush(dark);
    p.drawRoundedRect(visor, hh * 0.12, hh * 0.12);
    const QColor led(QStringLiteral("#6FE3FF"));
    const qreal er = visor.height() * 0.3;
    const qreal dart = mode == CHECKING ? std::sin(now * 12.0) * visor.width() * 0.06 : 0.0;
    for (qreal d : {-1.0, 1.0}) {
        const qreal ex = visor.center().x() + d * visor.width() * 0.22 + dart;
        const qreal ey = visor.center().y();
        if (mode == VERIFIED && d > 0 && since < 2.6) {
            p.setPen(QPen(led, std::max(1.5, er * 0.5), Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(ex - er, ey), QPointF(ex + er, ey)); // wink
            p.setPen(Qt::NoPen);
            continue;
        }
        if (mode == FAILED) {
            p.setPen(QPen(QColor(QStringLiteral("#FF6B6B")), std::max(1.5, er * 0.45), Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(ex - er * 0.8, ey - er * 0.8), QPointF(ex + er * 0.8, ey + er * 0.8));
            p.drawLine(QPointF(ex - er * 0.8, ey + er * 0.8), QPointF(ex + er * 0.8, ey - er * 0.8));
            p.setPen(Qt::NoPen);
            continue;
        }
        QColor glow(led);
        glow.setAlpha(80);
        p.setBrush(glow);
        p.drawEllipse(QPointF(ex, ey), er * 1.5, er * 1.5);
        p.setBrush(led);
        p.drawEllipse(QPointF(ex, ey), er, er * (mode == CHECKING ? 0.45 : 1.0));
    }
    // Mouth grille.
    const QRectF mouth(c.x() - hw * 0.22, head.top() + hh * 0.66, hw * 0.44, hh * 0.16);
    p.setBrush(dark);
    p.drawRoundedRect(mouth, 2, 2);
    p.setPen(QPen(rc, std::max(1.0, s * 0.015)));
    for (int k = 1; k < 5; ++k) {
        const qreal x = mouth.left() + mouth.width() * k / 5.0;
        const qreal wob = mode == CHECKING ? std::sin(now * 30.0 + k) * mouth.height() * 0.2 : 0.0;
        p.drawLine(QPointF(x, mouth.top() + 1 + wob), QPointF(x, mouth.bottom() - 1));
    }
    // Oil sweat while being "checked".
    if (mode == CHECKING) {
        for (int k = 0; k < 2; ++k) {
            const qreal t = pyMod(now * 1.2 + k * 0.5, 1.0);
            QColor oc(QStringLiteral("#3A3020"));
            oc.setAlpha(int(230 * (1.0 - t)));
            const qreal x = head.right() - hw * 0.08 + k * hw * 0.1;
            const qreal y = head.top() + hh * 0.1 + t * hh * 0.5;
            QPainterPath drop;
            drop.moveTo(x, y - 5.0);
            drop.quadTo(x + 3.5, y + 1.0, x, y + 3.0);
            drop.quadTo(x - 3.5, y + 1.0, x, y - 5.0);
            p.fillPath(drop, oc);
        }
    }
    // Thumbs-up when verified.
    if (mode == VERIFIED && since < 2.6) {
        const qreal hx = head.right() + s * 0.18;
        const qreal hy = head.center().y() + s * 0.05;
        p.setPen(QPen(rc.darker(150), std::max(1.0, s * 0.02)));
        p.setBrush(rc);
        p.drawRoundedRect(QRectF(hx - s * 0.1, hy - s * 0.05, s * 0.2, s * 0.18), 4, 4);
        p.drawRoundedRect(QRectF(hx - s * 0.05, hy - s * 0.2, s * 0.08, s * 0.17), 3, 3);
    }
    // Speech.
    QString speech;
    if (mode == CHECKING)
        speech = since < 0.9 ? tr("beep...?") : tr("Calculating humanity...");
    else if (mode == VERIFIED && since < 2.6)
        speech = tr("Definitely human. Totally.");
    else if (mode == FAILED)
        speech = tr("Oh no. One of us!");
    if (!speech.isEmpty() && m_rise > 0.6) {
        QFont f(font());
        f.setPointSizeF(8.5);
        f.setBold(true);
        const QFontMetricsF fm(f);
        const qreal tw = fm.horizontalAdvance(speech);
        const qreal bw = tw + 16.0;
        const qreal bh = fm.height() + 8.0;
        const qreal bx = clamp(head.left() - bw - 6.0, 2.0, width() - bw - 2.0);
        const qreal by = std::max(2.0, head.top() - bh * 0.4);
        QPainterPath path;
        path.addRoundedRect(QRectF(bx, by, bw, bh), bh / 2.0, bh / 2.0);
        p.setPen(QPen(QColor(20, 20, 30, 60), 1.0));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawPath(path);
        p.setFont(f);
        p.setPen(QColor(QStringLiteral("#22222A")));
        p.drawText(QRectF(bx, by, bw, bh), Qt::AlignCenter, speech);
    }
}
