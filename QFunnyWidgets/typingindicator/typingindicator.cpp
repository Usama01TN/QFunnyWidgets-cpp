/**
 * TypingIndicatorWidget: a chat "someone is typing" bubble whose three dots have faces and show what the other
 * person is really doing: typing (hopping dots), thinking (floating dots and a "?"), deleting (angry dots that
 * vanish right-to-left) or sleeping (squashed snoring dots). Optionally cycles through the states like a human.
 */
#include "typingindicator.h"

#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <cmath>

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

QString TypingIndicatorWidget::label(int state)
{
    switch (state) {
    case Typing:
        return QStringLiteral("is typing...");
    case Thinking:
        return QStringLiteral("is thinking really hard...");
    case Deleting:
        return QStringLiteral("is deleting everything...");
    default:
        return QStringLiteral("fell asleep on the keyboard");
    }
}

TypingIndicatorWidget::TypingIndicatorWidget(QWidget *parent)
    : TypingIndicatorWidget(Options(), parent)
{
}

TypingIndicatorWidget::TypingIndicatorWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_state(States(clampInt(options.state, 0, 3))),
      m_name(options.name),
      m_bubbleColor(options.bubbleColor),
      m_dotColor(options.dotColor),
      m_autoCycle(options.autoCycle)
{
    if (options.stateChanged)
        connect(this, &TypingIndicatorWidget::stateChanged, this, options.stateChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TypingIndicatorWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal TypingIndicatorWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

// ---------- Public API ----------
TypingIndicatorWidget::States TypingIndicatorWidget::getState() const
{
    return m_state;
}

void TypingIndicatorWidget::setState(int state)
{
    const States st = States(clampInt(state, 0, 3));
    if (st != m_state) {
        m_state = st;
        m_stateAt = now();
        emit stateChanged(int(st));
        update();
    }
}

QString TypingIndicatorWidget::getName() const
{
    return m_name;
}

void TypingIndicatorWidget::setName(const QString &name)
{
    m_name = name;
    update();
}

QColor TypingIndicatorWidget::getBubbleColor() const
{
    return m_bubbleColor;
}

void TypingIndicatorWidget::setBubbleColor(const QColor &color)
{
    if (m_bubbleColor != color) {
        m_bubbleColor = color;
        update();
    }
}

QColor TypingIndicatorWidget::getDotColor() const
{
    return m_dotColor;
}

void TypingIndicatorWidget::setDotColor(const QColor &color)
{
    if (m_dotColor != color) {
        m_dotColor = color;
        update();
    }
}

bool TypingIndicatorWidget::isAutoCycle() const
{
    return m_autoCycle;
}

void TypingIndicatorWidget::setAutoCycle(bool autoCycle)
{
    m_autoCycle = autoCycle;
    m_nextCycle = now() + 2.0;
}

// ---------- Qt overrides ----------
QSize TypingIndicatorWidget::sizeHint() const
{
    return QSize(260, 90);
}

QSize TypingIndicatorWidget::minimumSizeHint() const
{
    return QSize(90, 40);
}

void TypingIndicatorWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void TypingIndicatorWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void TypingIndicatorWidget::tick()
{
    const qreal t = now();
    if (m_autoCycle && t >= m_nextCycle) {
        // Typing is the hub: people type, hesitate, delete, type again... and sometimes doze off.
        States nxt;
        if (m_state == Typing) {
            static const States options[] = {Thinking, Thinking, Deleting, Deleting, Sleeping};
            nxt = options[QRandomGenerator::global()->bounded(5)];
        } else {
            nxt = Typing;
        }
        setState(nxt);
        m_nextCycle = t + (nxt == Typing ? uniform(2.5, 4.0) : uniform(2.0, 3.0));
    }
    m_blend = clamp((t - m_stateAt) * 5.0, 0.0, 1.0);
    update();
}

// ---------- Painting ----------
void TypingIndicatorWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal w = width();
    const qreal h = height();
    const qreal labelH = !m_name.isEmpty() ? 22.0 : 0.0;
    const qreal bh = clamp(h - labelH - 10.0, 24.0, 64.0);
    const qreal bw = std::min(w - 10.0, bh * 2.6);
    const QRectF bubble(6.0, (h - labelH - bh) / 2.0, bw, bh);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Bubble with a little tail.
    QPainterPath path;
    path.addRoundedRect(bubble, bh / 2.0, bh / 2.0);
    QPainterPath tail;
    tail.addEllipse(QPointF(bubble.left() + bh * 0.1, bubble.bottom() - bh * 0.05), bh * 0.13, bh * 0.13);
    p.setPen(Qt::NoPen);
    p.setBrush(m_bubbleColor);
    p.drawPath(path.united(tail));
    p.drawEllipse(QPointF(bubble.left() - bh * 0.05, bubble.bottom() + bh * 0.12), bh * 0.06, bh * 0.06);
    // Dots.
    const States st = m_state;
    const qreal r = bh * 0.17;
    const qreal gap = bh * 0.55;
    const qreal cx = bubble.center().x();
    const qreal cy = bubble.center().y();
    const qreal since = tNow - m_stateAt;
    for (int i = 0; i < 3; ++i) {
        qreal x = cx + (i - 1) * gap;
        qreal y = cy;
        qreal sx = 1.0;
        qreal sy = 1.0;
        if (st == Typing) {
            const qreal ph = pyMod(tNow * 2.4 - i * 0.18, 1.0);
            const qreal hop = ph < 0.5 ? std::sin(ph * 2.0 * PI) : 0.0;
            y -= std::max(0.0, hop) * r * 1.2;
            if (ph > 0.5 && ph < 0.62) {
                sx = 1.18; // landing squash
                sy = 0.82;
            }
        } else if (st == Thinking) {
            y += std::sin(tNow * 1.6 + i * 0.8) * r * 0.25;
        } else if (st == Deleting) {
            x += uniform(-1.0, 1.0) * r * 0.25;
            y += uniform(-1.0, 1.0) * r * 0.2;
            // Dots get "deleted" right-to-left, then all pop back.
            const qreal ph = pyMod(since * 0.9, 1.0);
            const int j = 2 - i;
            const qreal t0 = 0.15 + j * 0.22;
            if (ph >= 0.88)
                sx = clamp((ph - 0.88) * 9.0, 0.0, 1.0);
            else if (ph >= t0)
                sx = 1.0 - clamp((ph - t0) * 12.0, 0.0, 1.0);
            sy = sx;
        } else if (st == Sleeping) {
            const qreal breathe = std::sin(tNow * 2.0 + i * 0.4);
            sx = 1.25 + 0.05 * breathe;
            sy = 0.7 - 0.05 * breathe;
            y += r * 0.35;
        }
        if (sx <= 0.01)
            continue;
        drawDot(p, QPointF(x, y), r, sx, sy, i, st, tNow);
    }
    // Thinking "?" and sleeping "z".
    QFont f(font());
    f.setBold(true);
    if (st == Thinking) {
        f.setPointSizeF(std::max(7.0, bh * 0.22));
        p.setFont(f);
        QColor qc(m_dotColor);
        qc.setAlpha(int(255 * m_blend));
        p.setPen(qc);
        p.drawText(QPointF(cx + gap * 1.0 - r * 0.3, bubble.top() + bh * 0.1 + std::sin(tNow * 3.0) * 2.0),
                   QStringLiteral("?"));
    } else if (st == Sleeping) {
        for (int k = 0; k < 2; ++k) {
            const qreal ph = pyMod(tNow * 0.5 + k * 0.5, 1.0);
            f.setPointSizeF(std::max(6.0, bh * (0.14 + 0.1 * ph)));
            p.setFont(f);
            QColor zc(m_dotColor);
            zc.setAlpha(int(230 * std::sin(ph * PI)));
            p.setPen(zc);
            p.drawText(QPointF(bubble.right() - bh * 0.1 + ph * bh * 0.3, bubble.top() + bh * 0.2 - ph * bh * 0.45),
                       QStringLiteral("z"));
        }
    }
    // Label.
    if (!m_name.isEmpty()) {
        QFont lf(font());
        lf.setPointSizeF(8.5);
        lf.setItalic(true);
        p.setFont(lf);
        QColor tc(palette().text().color());
        tc.setAlpha(170);
        p.setPen(tc);
        p.drawText(QRectF(bubble.left() + 4.0, bubble.bottom() + 8.0, w - 10.0, labelH), Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("%1 %2").arg(m_name, label(int(st))));
    }
    p.end();
}

void TypingIndicatorWidget::drawDot(QPainter &p, const QPointF &c, qreal r, qreal sx, qreal sy, int, States st, qreal)
{
    const QColor dc = m_dotColor;
    p.save();
    p.translate(c);
    p.scale(sx, sy);
    p.setPen(Qt::NoPen);
    p.setBrush(dc);
    p.drawEllipse(QPointF(0, 0), r, r);
    p.setBrush(QColor(255, 255, 255, 70));
    p.drawEllipse(QPointF(-r * 0.35, -r * 0.4), r * 0.3, r * 0.2);
    const QColor ink(QStringLiteral("#FFFFFF"));
    const qreal ex = r * 0.38;
    const qreal er = r * 0.2;
    if (st == Sleeping) {
        p.setPen(QPen(ink, std::max(1.0, r * 0.14), Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(d * ex - er, -er * 0.4, er * 2.0, er * 1.2), 180 * 16, 180 * 16);
    } else if (st == Deleting) {
        // Determined, angry little eyes.
        p.setPen(QPen(ink, std::max(1.0, r * 0.15), Qt::SolidLine, Qt::RoundCap));
        for (qreal d : {-1.0, 1.0})
            p.drawLine(QPointF(d * (ex + er), -er * 1.2), QPointF(d * (ex - er), -er * 0.4));
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(d * ex, er * 0.2), er * 0.7, er * 0.7);
    } else {
        const QPointF look = st == Thinking ? QPointF(0, -er * 0.6) : QPointF(er * 0.35, 0);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(d * ex, -er * 0.3), er, er * 1.15);
        p.setBrush(QColor(QStringLiteral("#1E1E28")));
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(d * ex, -er * 0.3) + look, er * 0.55, er * 0.6);
    }
    p.restore();
}
