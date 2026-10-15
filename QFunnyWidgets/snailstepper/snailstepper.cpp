/**
 * SnailStepperWidget: a wizard step indicator where a snail crawls (slowly, in pulses) from step to step, leaving
 * a shiny slime trail and complaining when you rush it. Steps are clickable.
 */
#include "snailstepper.h"

#include <QFontMetricsF>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <algorithm>
#include <cmath>

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

inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

} // namespace

QStringList SnailStepperWidget::defaultSteps()
{
    return {QStringLiteral("Cart"), QStringLiteral("Shipping"), QStringLiteral("Payment"), QStringLiteral("Review"),
            QStringLiteral("Done")};
}

SnailStepperWidget::SnailStepperWidget(QWidget *parent)
    : SnailStepperWidget(Options(), parent)
{
}

SnailStepperWidget::SnailStepperWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_steps(options.steps.isEmpty() ? defaultSteps() : options.steps),
      m_current(clampInt(options.current, 0, int(m_steps.size()) - 1)),
      m_speed(std::max(5.0, qreal(options.speed))),
      m_clickable(options.clickable),
      m_accentColor(options.accentColor),
      m_shellColor(options.shellColor)
{
    if (options.currentChanged)
        connect(this, &SnailStepperWidget::currentChanged, this, options.currentChanged);
    if (options.arrived)
        connect(this, &SnailStepperWidget::arrived, this, options.arrived);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SnailStepperWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal SnailStepperWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QList<qreal> SnailStepperWidget::layoutSteps(qreal &y, qreal &r, qreal &size) const
{
    const int n = int(m_steps.size());
    const qreal w = width();
    const qreal h = height();
    const qreal margin = 46.0;
    const qreal gap = (w - 2 * margin) / std::max(1, n - 1);
    r = clamp(gap * 0.16, 9.0, 15.0);
    y = h * 0.56;
    size = clamp(std::min(gap * 0.5, h * 0.42), 26.0, 52.0);
    QList<qreal> xs;
    for (int i = 0; i < n; ++i)
        xs.append(margin + i * gap);
    return xs;
}

// ---------- Public API ----------
void SnailStepperWidget::next()
{
    setCurrent(m_current + 1);
}

void SnailStepperWidget::previous()
{
    setCurrent(m_current - 1);
}

int SnailStepperWidget::getCurrent() const
{
    return m_current;
}

void SnailStepperWidget::setCurrent(int current)
{
    current = clampInt(current, 0, int(m_steps.size()) - 1);
    if (current != m_current) {
        m_current = current;
        m_moving = true;
        emit currentChanged(current);
        update();
    }
}

QStringList SnailStepperWidget::getSteps() const
{
    return m_steps;
}

void SnailStepperWidget::setSteps(const QStringList &steps)
{
    m_steps = steps.isEmpty() ? defaultSteps() : steps;
    m_current = std::min(m_current, int(m_steps.size()) - 1);
    update();
}

qreal SnailStepperWidget::getSpeed() const { return m_speed; }
bool SnailStepperWidget::isClickable() const { return m_clickable; }
QColor SnailStepperWidget::getAccentColor() const { return m_accentColor; }
QColor SnailStepperWidget::getShellColor() const { return m_shellColor; }

void SnailStepperWidget::setSpeed(qreal speed)
{
    m_speed = std::max(5.0, speed);
    update();
}

void SnailStepperWidget::setClickable(bool clickable)
{
    m_clickable = clickable;
}

void SnailStepperWidget::setAccentColor(const QColor &color)
{
    m_accentColor = color;
    update();
}

void SnailStepperWidget::setShellColor(const QColor &color)
{
    m_shellColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize SnailStepperWidget::sizeHint() const
{
    return QSize(520, 130);
}

QSize SnailStepperWidget::minimumSizeHint() const
{
    return QSize(260, 100);
}

void SnailStepperWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void SnailStepperWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void SnailStepperWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qreal y, r, size;
    const QList<qreal> xs = layoutSteps(y, r, size);
    m_x = xs.at(m_current);
    m_trail = std::max(m_trail, m_x);
}

void SnailStepperWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_clickable) {
        qreal y, r, size;
        const QList<qreal> xs = layoutSteps(y, r, size);
        const QPointF pos = eventPos(e);
        for (int i = 0; i < xs.size(); ++i) {
            if (std::abs(pos.x() - xs.at(i)) < r * 1.8 && std::abs(pos.y() - y) < r * 3.5) {
                setCurrent(i);
                break;
            }
        }
    }
    QWidget::mouseReleaseEvent(e);
}

void SnailStepperWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Right)
        next();
    else if (e->key() == Qt::Key_Left)
        previous();
    else
        QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void SnailStepperWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    qreal y, r, size;
    const QList<qreal> xs = layoutSteps(y, r, size);
    const qreal target = xs.at(m_current);
    if (m_x < 0) {
        m_x = target;
        m_trail = target;
    }
    const qreal d = target - m_x;
    if (std::abs(d) > 0.5) {
        // Snails move in pulses: speed rises and falls with each "step".
        m_crawl += dt * 6.0;
        const qreal v = m_speed * (0.55 + 0.45 * std::abs(std::sin(m_crawl)));
        const qreal move = std::min(std::abs(d), v * dt);
        m_x += d > 0 ? move : -move;
        m_facing = d > 0 ? 1.0 : -1.0;
    } else if (m_moving) {
        m_x = target;
        m_moving = false;
        m_arrivedAt = t;
        emit arrived(m_current);
    }
    // Slime only marks the furthest point reached; going back leaves it shining.
    m_trail = m_facing > 0 ? std::max(m_trail, m_x) : m_trail;
    m_trail = std::min(m_trail, xs.last());
    update();
}

// ---------- Painting ----------
void SnailStepperWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    qreal y, r, size;
    const QList<qreal> xs = layoutSteps(y, r, size);
    const QColor tc(palette().text().color());
    const QColor ac = m_accentColor;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Track.
    QColor track(tc);
    track.setAlpha(45);
    p.setPen(QPen(track, 4.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(xs.first(), y), QPointF(xs.last(), y));
    // Slime trail with a travelling shine.
    if (m_trail > xs.first()) {
        QColor slime(ac);
        slime.setAlpha(150);
        p.setPen(QPen(slime, 4.0, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(xs.first(), y), QPointF(m_trail, y));
        const qreal span = m_trail - xs.first();
        for (int k = 0; k < 3; ++k) {
            const qreal sx = xs.first() + pyMod(tNow * 0.4 + k / 3.0, 1.0) * span;
            p.setPen(QPen(QColor(255, 255, 255, 170), 2.0, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(sx - 4, y - 1), QPointF(sx + 4, y - 1));
        }
    }
    // Steps.
    QFont lf(font());
    lf.setPointSizeF(8.5);
    QFont nf(font());
    nf.setBold(true);
    nf.setPointSizeF(std::max(7.0, r * 0.7));
    for (int i = 0; i < xs.size(); ++i) {
        const qreal x = xs.at(i);
        const bool done = i < m_current && xs.at(i) <= m_x + 1.0;
        const bool cur = i == m_current;
        const QPointF c(x, y);
        if (cur) {
            const qreal pulse = r * (1.25 + 0.15 * std::sin(tNow * 4.0));
            QColor halo(ac);
            halo.setAlpha(50);
            p.setPen(Qt::NoPen);
            p.setBrush(halo);
            p.drawEllipse(c, pulse, pulse);
        }
        if (done) {
            p.setPen(Qt::NoPen);
            p.setBrush(ac);
            p.drawEllipse(c, r, r);
            p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), std::max(2.0, r * 0.18), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            QPainterPath path;
            path.moveTo(x - r * 0.45, y + r * 0.02);
            path.lineTo(x - r * 0.1, y + r * 0.35);
            path.lineTo(x + r * 0.45, y - r * 0.32);
            p.setBrush(Qt::NoBrush);
            p.drawPath(path);
        } else {
            const QColor bg = palette().window().color();
            p.setPen(QPen(cur ? ac : (tc.lightness() > 128 ? track.lighter(150) : track), 2.5));
            p.setBrush(bg);
            p.drawEllipse(c, r, r);
            p.setFont(nf);
            QColor nc = cur ? QColor(ac) : QColor(tc);
            if (!cur)
                nc.setAlpha(140);
            p.setPen(nc);
            p.drawText(QRectF(x - r, y - r, 2 * r, 2 * r), Qt::AlignCenter, QString::number(i + 1));
        }
        p.setFont(lf);
        QColor lc(tc);
        lc.setAlpha(cur ? 230 : 150);
        p.setPen(lc);
        const qreal lw = std::min(110.0, xs.size() > 1 ? (xs.at(1) - xs.at(0)) : 110.0);
        p.drawText(QRectF(x - lw / 2.0, y + r + 8.0, lw, 16.0), Qt::AlignHCenter | Qt::AlignTop,
                   QFontMetricsF(lf).elidedText(m_steps.at(i), Qt::ElideRight, lw));
    }
    // Snail.
    drawSnail(p, QPointF(m_x, y - 2.0), size, tNow);
    // Speech.
    const qreal gap = std::abs(xs.at(m_current) - m_x);
    const qreal unit = xs.size() > 1 ? (xs.at(1) - xs.at(0)) : 100.0;
    QString speech;
    if (gap > unit * 1.2)
        speech = tr("I'm going as fast as I can!");
    else if (gap > 0.5)
        speech = tr("On my way...");
    else if (tNow - m_arrivedAt < 1.2)
        speech = m_current < xs.size() - 1 ? tr("Made it!") : tr("Finally! Done!");
    if (!speech.isEmpty()) {
        QFont f(font());
        f.setBold(true);
        f.setPointSizeF(8.0);
        const QFontMetricsF fm(f);
        const qreal tw = fm.horizontalAdvance(speech);
        const qreal bw = tw + 16.0;
        const qreal bh = fm.height() + 8.0;
        const qreal bx = clamp(m_x - bw / 2.0, 2.0, width() - bw - 2.0);
        const qreal by = std::max(2.0, y - size * 1.05 - bh);
        QPainterPath bubble;
        bubble.addRoundedRect(QRectF(bx, by, bw, bh), bh / 2.0, bh / 2.0);
        p.setPen(QPen(QColor(20, 20, 30, 60), 1.0));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawPath(bubble);
        p.setFont(f);
        p.setPen(QColor(QStringLiteral("#22222A")));
        p.drawText(QRectF(bx, by, bw, bh), Qt::AlignCenter, speech);
    }
    p.end();
}

void SnailStepperWidget::drawSnail(QPainter &p, const QPointF &base, qreal s, qreal now)
{
    const bool moving = m_moving;
    const qreal stretch = 1.0 + (moving ? 0.1 * std::sin(m_crawl * 2.0) : 0.0);
    const QColor body(QStringLiteral("#D8C3A5"));
    const QColor shell = m_shellColor;
    p.save();
    p.translate(base);
    p.scale(m_facing, 1.0);
    // Body (foot), stretching while crawling.
    p.save();
    p.scale(stretch, 2.0 - stretch);
    QPainterPath foot;
    foot.moveTo(-s * 0.6, 0);
    foot.quadTo(-s * 0.2, -s * 0.12, s * 0.3, -s * 0.12);
    foot.quadTo(s * 0.52, -s * 0.16, s * 0.5, -s * 0.02);
    foot.lineTo(s * 0.45, 0);
    foot.closeSubpath();
    p.setPen(QPen(body.darker(140), std::max(1.0, s * 0.02)));
    p.setBrush(body);
    p.drawPath(foot);
    p.restore();
    // Eye stalks (wobbly).
    const qreal hx = s * 0.42 * stretch;
    const qreal wob = std::sin(now * 5.0) * s * 0.03;
    static const qreal stalks[2][2] = {{0.06, 0.42}, {-0.04, 0.36}};
    for (int k = 0; k < 2; ++k) {
        const QPointF tip(hx + stalks[k][0] * s + wob * (k ? 1 : -1), -s * stalks[k][1]);
        p.setPen(QPen(body.darker(120), std::max(1.5, s * 0.04), Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(hx - s * 0.02, -s * 0.12), tip);
        p.setPen(QPen(body.darker(150), 1.0));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(tip, s * 0.06, s * 0.06);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#1E1E24")));
        p.drawEllipse(QPointF(tip.x() + s * 0.02, tip.y()), s * 0.03, s * 0.03);
    }
    // Smile.
    p.setPen(QPen(body.darker(170), std::max(1.0, s * 0.025), Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(hx - s * 0.04, -s * 0.11, s * 0.1, s * 0.06), 200 * 16, 140 * 16);
    // Shell with spiral.
    const QPointF sc(-s * 0.1, -s * 0.34);
    const qreal sr = s * 0.3;
    p.setPen(QPen(shell.darker(140), std::max(1.0, s * 0.03)));
    p.setBrush(shell);
    p.drawEllipse(sc, sr, sr);
    QPainterPath spiral;
    const qreal turns = 2.2;
    for (int k = 0; k < 41; ++k) {
        const qreal t = k / 40.0;
        const qreal ang = t * turns * 2.0 * PI + (moving ? m_crawl * 0.5 : 0.0);
        const qreal rad = sr * 0.85 * (1.0 - t);
        const QPointF pt(sc.x() + std::cos(ang) * rad, sc.y() + std::sin(ang) * rad);
        if (k == 0)
            spiral.moveTo(pt);
        else
            spiral.lineTo(pt);
    }
    p.setPen(QPen(shell.darker(150), std::max(1.0, s * 0.03), Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawPath(spiral);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 80));
    p.drawEllipse(QPointF(sc.x() - sr * 0.35, sc.y() - sr * 0.45), sr * 0.25, sr * 0.14);
    p.restore();
}
