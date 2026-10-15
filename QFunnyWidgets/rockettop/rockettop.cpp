/**
 * RocketBackToTop: a "back to top" button for scroll areas. Create it with a QAbstractScrollArea as parent: once
 * the user scrolls past the threshold a little rocket rises into the bottom-right corner of the viewport; clicking
 * it makes it rumble and blast off, scrolling the content back to the top. Next time it floats back down on a
 * parachute.
 */
#include "rockettop.h"

#include <QAbstractScrollArea>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QRegion>
#include <QScrollBar>
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

inline qreal uniform(qreal a, qreal b)
{
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}

inline qreal easeInOut(qreal t)
{
    t = clamp(t, 0.0, 1.0);
    return 3 * t * t - 2 * t * t * t;
}

} // namespace

RocketBackToTop::RocketBackToTop(QWidget *parent)
    : RocketBackToTop(Options(), parent)
{
}

RocketBackToTop::RocketBackToTop(QWidget *parent, const Options &options)
    : RocketBackToTop(options, parent)
{
}

RocketBackToTop::RocketBackToTop(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_threshold(std::max(0, options.threshold)),
      m_color(options.color),
      m_bodyColor(options.bodyColor),
      m_label(options.label)
{
    if (options.launched)
        connect(this, &RocketBackToTop::launched, this, options.launched);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Back to top"));
    // Timing (the clock is needed before the scroll hook can fire).
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &RocketBackToTop::tick);
    // Hook into the scroll area.
    m_area = qobject_cast<QAbstractScrollArea *>(parentWidget());
    m_viewport = m_area ? m_area->viewport() : nullptr;
    if (m_area) {
        // Watch the viewport, not the area: its geometry is only final once the viewport itself resizes.
        m_area->viewport()->installEventFilter(this);
        connect(m_area->verticalScrollBar(), &QScrollBar::valueChanged, this, &RocketBackToTop::onScroll);
    }
    place();
    hide();
}

// ---------- Helpers ----------
qreal RocketBackToTop::now() const
{
    return m_clock.elapsed() / 1000.0;
}

qreal RocketBackToTop::home() const
{
    return height() - 56.0;
}

void RocketBackToTop::place()
{
    if (!m_area)
        return;
    const QRect vg = m_area->viewport()->geometry();
    setGeometry(vg.right() - STRIP - 4, vg.top(), STRIP, vg.height());
    if (m_mode == IDLE)
        m_y = home();
}

void RocketBackToTop::setMode(Mode mode)
{
    m_mode = mode;
    m_modeAt = now();
    if (mode != HIDDEN) {
        show();
        raise();
        if (!m_timer->isActive()) {
            m_last = now();
            m_timer->start(16);
        }
    }
}

void RocketBackToTop::updateMask()
{
    if (m_mode == LAUNCH || !m_smoke.isEmpty())
        setMask(QRegion(rect()));
    else
        setMask(QRegion(QRect(0, int(m_y - 70), STRIP, 130)));
}

void RocketBackToTop::onScroll(int value)
{
    if (m_mode == LAUNCH)
        return;
    const bool isFar = value > m_threshold;
    if (isFar && (m_mode == HIDDEN || m_mode == SINKING)) {
        if (m_flown) {
            m_y = -60.0;
            m_vy = 0.0;
            setMode(PARACHUTE);
        } else {
            if (m_mode == HIDDEN)
                m_y = height() + 60.0;
            setMode(RISING);
        }
    } else if (!isFar && (m_mode == RISING || m_mode == IDLE || m_mode == PARACHUTE)) {
        setMode(SINKING);
    }
}

// ---------- Public API ----------
void RocketBackToTop::launch()
{
    if ((m_mode == IDLE || m_mode == RISING || m_mode == PARACHUTE) && m_area) {
        m_startScroll = m_area->verticalScrollBar()->value();
        m_y = home();
        setMode(LAUNCH);
        emit launched();
    }
}

int RocketBackToTop::getThreshold() const { return m_threshold; }
QColor RocketBackToTop::getColor() const { return m_color; }
QColor RocketBackToTop::getBodyColor() const { return m_bodyColor; }
QString RocketBackToTop::getLabel() const { return m_label; }

void RocketBackToTop::setThreshold(int threshold)
{
    m_threshold = std::max(0, threshold);
    update();
}

void RocketBackToTop::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void RocketBackToTop::setBodyColor(const QColor &color)
{
    m_bodyColor = color;
    update();
}

void RocketBackToTop::setLabel(const QString &label)
{
    m_label = label;
    update();
}

// ---------- Qt overrides ----------
bool RocketBackToTop::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_viewport && event->type() == QEvent::Resize)
        place();
    return QWidget::eventFilter(obj, event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void RocketBackToTop::enterEvent(QEnterEvent *e)
#else
void RocketBackToTop::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void RocketBackToTop::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void RocketBackToTop::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && std::abs(eventPos(e).y() - m_y) < 50)
        launch();
    QWidget::mouseReleaseEvent(e);
}

// ---------- Animation tick ----------
void RocketBackToTop::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal since = t - m_modeAt;
    const Mode mode = m_mode;
    const qreal hm = home();
    const qreal cx = STRIP / 2.0;
    if (mode == RISING || mode == IDLE) {
        m_vy += ((hm - m_y) * 120.0 - m_vy * 13.0) * dt;
        m_y += m_vy * dt;
        if (mode == RISING && std::abs(hm - m_y) < 1.0)
            setMode(IDLE);
    } else if (mode == SINKING) {
        m_vy += 900.0 * dt;
        m_y += m_vy * dt;
        if (m_y > height() + 70) {
            m_vy = 0.0;
            setMode(HIDDEN);
        }
    } else if (mode == PARACHUTE) {
        m_y = std::min(hm, m_y + 70.0 * dt);
        if (m_y >= hm) {
            m_flown = false;
            setMode(IDLE);
        }
    } else if (mode == LAUNCH) {
        if (since > RUMBLE) {
            const qreal ft = (since - RUMBLE) / FLIGHT;
            m_y = hm - (hm + 120.0) * ft * ft;
            if (m_area)
                m_area->verticalScrollBar()->setValue(int(m_startScroll * (1.0 - easeInOut(ft))));
            m_smokeAcc += dt * 50.0;
            if (ft >= 1.0) {
                if (m_area)
                    m_area->verticalScrollBar()->setValue(0);
                m_flown = true;
                setMode(HIDDEN);
            }
        } else {
            m_smokeAcc += dt * 25.0;
        }
        while (m_smokeAcc > 1.0) {
            m_smokeAcc -= 1.0;
            const qreal x = cx + uniform(-6, 6);
            const qreal vx = uniform(-25, 25);
            const qreal vy = uniform(10, 40);
            const qreal life = uniform(0.6, 1.1);
            m_smoke.append(Smoke(x, m_y + 30.0, vx, vy, life));
        }
    }
    QList<Smoke> alive;
    for (Smoke s : std::as_const(m_smoke)) {
        s.x += s.vx * dt;
        s.y += s.vy * dt;
        s.vx *= 0.97;
        s.life -= dt;
        if (s.life > 0)
            alive.append(s);
    }
    m_smoke = alive;
    if (m_mode == HIDDEN && m_smoke.isEmpty()) {
        m_timer->stop();
        clearMask();
        hide();
        return;
    }
    updateMask();
    update();
}

// ---------- Painting ----------
void RocketBackToTop::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Smoke.
    p.setPen(Qt::NoPen);
    for (const Smoke &s : std::as_const(m_smoke)) {
        const qreal a = clamp(s.life / s.maxLife, 0.0, 1.0);
        QColor sc(210, 214, 222);
        sc.setAlpha(int(170 * a));
        p.setBrush(sc);
        const qreal r = 5.0 + 14.0 * (1.0 - a);
        p.drawEllipse(QPointF(s.x, s.y), r, r);
    }
    if (m_mode != HIDDEN) {
        qreal cx = STRIP / 2.0;
        qreal y = m_y;
        const Mode mode = m_mode;
        const qreal since = tNow - m_modeAt;
        if (mode == LAUNCH && since < RUMBLE)
            cx += uniform(-2.0, 2.0);
        else if (mode == IDLE)
            y += std::sin(tNow * 2.5) * 2.5 - (m_hovered ? 3.0 : 0.0);
        if (mode == PARACHUTE)
            drawParachute(p, QPointF(cx, y), tNow);
        drawRocket(p, QPointF(cx, y), mode, since, tNow);
    }
    p.end();
}

void RocketBackToTop::drawRocket(QPainter &p, const QPointF &c, Mode mode, qreal since, qreal now)
{
    const QColor accent = m_color;
    const QColor body = m_bodyColor;
    // Flame.
    qreal length;
    if (mode == LAUNCH)
        length = 18.0 + (since > RUMBLE ? 40.0 : since / RUMBLE * 18.0);
    else if (mode == PARACHUTE)
        length = 0.0;
    else
        length = 8.0 + 4.0 * std::abs(std::sin(now * 17.0)) + (m_hovered ? 6.0 : 0.0);
    if (length > 0) {
        const struct { const char *col; qreal wf; qreal lf; } flames[2] = {{"#FF9F1C", 1.0, 1.0}, {"#FFD23F", 0.6, 0.65}};
        for (const auto &fdef : flames) {
            QPainterPath fl;
            const qreal top = c.y() + 22.0;
            const qreal ln = length * fdef.lf * (0.9 + 0.2 * std::abs(std::sin(now * 23.0)));
            fl.moveTo(c.x() - 8.0 * fdef.wf, top);
            fl.quadTo(c.x() - 6.0 * fdef.wf, top + ln * 0.6, c.x(), top + ln);
            fl.quadTo(c.x() + 6.0 * fdef.wf, top + ln * 0.6, c.x() + 8.0 * fdef.wf, top);
            fl.closeSubpath();
            p.setPen(Qt::NoPen);
            p.fillPath(fl, QColor(QString::fromLatin1(fdef.col)));
        }
    }
    // Fins.
    p.setPen(QPen(accent.darker(140), 1.2));
    p.setBrush(accent);
    for (qreal d : {-1.0, 1.0}) {
        QPainterPath fin;
        fin.moveTo(c.x() + d * 12.0, c.y() + 2.0);
        fin.lineTo(c.x() + d * 22.0, c.y() + 22.0);
        fin.lineTo(c.x() + d * 12.0, c.y() + 18.0);
        fin.closeSubpath();
        p.drawPath(fin);
    }
    // Body + nose cone.
    QPainterPath hull;
    hull.moveTo(c.x(), c.y() - 38.0);
    hull.cubicTo(c.x() + 14.0, c.y() - 26.0, c.x() + 14.0, c.y() - 8.0, c.x() + 12.0, c.y() + 20.0);
    hull.lineTo(c.x() - 12.0, c.y() + 20.0);
    hull.cubicTo(c.x() - 14.0, c.y() - 8.0, c.x() - 14.0, c.y() - 26.0, c.x(), c.y() - 38.0);
    p.setPen(QPen(body.darker(140), 1.2));
    p.setBrush(body);
    p.drawPath(hull);
    p.save();
    p.setClipPath(hull);
    p.fillRect(QRectF(c.x() - 16, c.y() - 40, 32, 13), accent);
    p.restore();
    p.drawPath(hull);
    p.setPen(QPen(accent.darker(140), 1.0));
    p.setBrush(accent.darker(110));
    p.drawRect(QRectF(c.x() - 7.0, c.y() + 19.0, 14.0, 4.0));
    // Porthole with a tiny pilot.
    const QPointF win(c.x(), c.y() - 12.0);
    p.setPen(QPen(QColor(QStringLiteral("#8D99AE")), 2.0));
    p.setBrush(QColor(QStringLiteral("#BDE6FF")));
    p.drawEllipse(win, 7.0, 7.0);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#1E1E24")));
    const qreal look = (m_hovered || mode == LAUNCH) ? -1.5 : 0.0;
    if (mode == LAUNCH && since > RUMBLE) {
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(win.x() + d * 2.2, win.y() - 1.0), 1.4, 2.2); // wide-eyed
        p.drawEllipse(QPointF(win.x(), win.y() + 2.8), 1.6, 1.8);               // "wheee"
    } else {
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(win.x() + d * 2.2, win.y() + look), 1.1, 1.3);
        p.setPen(QPen(QColor(QStringLiteral("#1E1E24")), 0.9, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(win.x() - 2.0, win.y() + 0.5, 4.0, 2.6), 200 * 16, 140 * 16);
    }
    // Label.
    QFont f(font());
    f.setBold(true);
    f.setPixelSize(8);
    p.setFont(f);
    p.setPen(accent.darker(120));
    p.drawText(QRectF(c.x() - 12.0, c.y() + 2.0, 24.0, 12.0), Qt::AlignCenter, m_label);
}

void RocketBackToTop::drawParachute(QPainter &p, const QPointF &c, qreal now)
{
    const qreal sway = std::sin(now * 2.0) * 3.0;
    const QPointF top(c.x() + sway, c.y() - 82.0);
    QPainterPath canopy;
    canopy.moveTo(top.x() - 30.0, top.y() + 18.0);
    canopy.cubicTo(top.x() - 30.0, top.y() - 12.0, top.x() + 30.0, top.y() - 12.0, top.x() + 30.0, top.y() + 18.0);
    for (int k = 0; k < 4; ++k) {
        const qreal x0 = top.x() + 30.0 - k * 15.0;
        canopy.quadTo(x0 - 7.5, top.y() + 12.0, x0 - 15.0, top.y() + 18.0);
    }
    p.setPen(QPen(QColor(QStringLiteral("#9AA0AD")), 1.0));
    p.setBrush(m_color.lighter(130));
    p.drawPath(canopy);
    p.setPen(QPen(QColor(QStringLiteral("#9AA0AD")), 0.9));
    for (qreal x : {-30.0, -10.0, 10.0, 30.0})
        p.drawLine(QPointF(top.x() + x, top.y() + 18.0), QPointF(c.x() + x * 0.3, c.y() - 34.0));
}
