/**
 * DeadlineCountdownWidget: a countdown timer whose desk calendar copes progressively worse: relaxed with sunglasses,
 * focused, nervous and sweating, panicking with trembling digits, and finally on fire saying "this is fine".
 * Supports pause/resume, extensions (with a sigh of relief) and a time scale for demos.
 */
#include "deadline.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

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

/** Python divmod for ints. */
inline void divmod(qint64 a, qint64 b, qint64 &q, qint64 &r)
{
    q = a / b;
    r = a % b;
    if (r != 0 && ((r < 0) != (b < 0))) {
        q -= 1;
        r += b;
    }
}

} // namespace

QString formatTime(qreal seconds)
{
    const qint64 s = qint64(seconds);
    qint64 h, rem, m, sec;
    divmod(s, 3600, h, rem);
    divmod(rem, 60, m, sec);
    if (h)
        return QStringLiteral("%1:%2:%3").arg(h).arg(m, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
    return QStringLiteral("%1:%2").arg(m, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
}

DeadlineCountdownWidget::DeadlineCountdownWidget(QWidget *parent)
    : DeadlineCountdownWidget(Options(), parent)
{
}

DeadlineCountdownWidget::DeadlineCountdownWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_duration(std::max(1, options.duration)),
      m_title(options.title),
      m_timeScale(std::max(0.01, qreal(options.timeScale))),
      m_accentColor(options.accentColor)
{
    if (options.expired)
        connect(this, &DeadlineCountdownWidget::expired, this, options.expired);
    if (options.remainingChanged)
        connect(this, &DeadlineCountdownWidget::remainingChanged, this, options.remainingChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // State.
    m_clock.start();
    m_base = m_duration;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DeadlineCountdownWidget::tick);
    m_timer->start(16);
    if (options.autoStart)
        start();
}

// ---------- Helpers ----------
qreal DeadlineCountdownWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

// ---------- Public API ----------
qreal DeadlineCountdownWidget::getRemaining() const
{
    if (!m_running)
        return m_base;
    return m_base - (now() - m_runFrom) * m_timeScale;
}

void DeadlineCountdownWidget::start(int seconds)
{
    if (seconds >= 0)
        m_duration = std::max(1, seconds);
    m_base = m_duration;
    m_runFrom = now();
    m_running = true;
    m_expiredSent = false;
    m_flames.clear();
    update();
}

void DeadlineCountdownWidget::pause()
{
    if (m_running) {
        m_base = getRemaining();
        m_running = false;
    }
}

void DeadlineCountdownWidget::resume()
{
    if (!m_running) {
        m_runFrom = now();
        m_running = true;
    }
}

void DeadlineCountdownWidget::extend(int seconds)
{
    const qreal rem = getRemaining();
    m_base = std::max(rem, 0.0) + std::max(0, seconds);
    m_runFrom = now();
    m_duration = std::max(m_duration, int(m_base));
    m_expiredSent = false;
    m_reliefAt = now();
    m_flames.clear();
}

bool DeadlineCountdownWidget::isRunning() const { return m_running; }
int DeadlineCountdownWidget::getDuration() const { return m_duration; }
QString DeadlineCountdownWidget::getTitle() const { return m_title; }
qreal DeadlineCountdownWidget::getTimeScale() const { return m_timeScale; }
QColor DeadlineCountdownWidget::getAccentColor() const { return m_accentColor; }

void DeadlineCountdownWidget::setDuration(int duration)
{
    start(std::max(1, duration));
}

void DeadlineCountdownWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void DeadlineCountdownWidget::setTimeScale(qreal timeScale)
{
    const qreal rem = getRemaining();
    m_timeScale = std::max(0.01, timeScale);
    m_base = rem;
    m_runFrom = now();
}

void DeadlineCountdownWidget::setAccentColor(const QColor &color)
{
    m_accentColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize DeadlineCountdownWidget::sizeHint() const
{
    return QSize(440, 170);
}

QSize DeadlineCountdownWidget::minimumSizeHint() const
{
    return QSize(260, 110);
}

void DeadlineCountdownWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void DeadlineCountdownWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
QRectF DeadlineCountdownWidget::calRect() const
{
    const qreal h = height();
    const qreal ch = h * 0.72;
    const qreal cw = ch * 0.82;
    return QRectF(14.0, (h - ch) / 2.0 + h * 0.06, cw, ch);
}

void DeadlineCountdownWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal rem = getRemaining();
    const int whole = rem >= 0 ? int(rem) : -int(-rem) - 1;
    if (whole != m_lastWhole) {
        m_lastWhole = whole;
        emit remainingChanged(whole);
    }
    if (rem <= 0 && !m_expiredSent) {
        m_expiredSent = true;
        emit expired();
    }
    if (rem <= 0) {
        const QRectF cal = calRect();
        m_flameAcc += dt * 40.0;
        while (m_flameAcc > 1.0) {
            m_flameAcc -= 1.0;
            const qreal x = uniform(cal.left() + 4, cal.right() - 4);
            const qreal vy = -uniform(40, 90);
            const qreal size = uniform(5, 11) * cal.width() / 90.0;
            const qreal life = uniform(0.5, 0.9);
            m_flames.append(Flame(x, cal.top() + cal.height() * 0.15, vy, size, life));
        }
    }
    QList<Flame> alive;
    for (Flame f : std::as_const(m_flames)) {
        f.y += f.vy * dt;
        f.x += std::sin(t * 9.0 + f.y * 0.1) * 12.0 * dt;
        f.life -= dt;
        if (f.life > 0)
            alive.append(f);
    }
    m_flames = alive;
    update();
}

// ---------- Painting ----------
void DeadlineCountdownWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal rem = getRemaining();
    const qreal frac = rem / qreal(m_duration);
    const bool relieved = tNow - m_reliefAt < 1.8;
    const QRectF cal = calRect();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Flames behind the calendar.
    drawFlames(p, true);
    p.save();
    if (0 < frac && frac < 0.1) {
        const qreal jx = uniform(-1.5, 1.5);
        const qreal jy = uniform(-1.5, 1.5);
        p.translate(jx, jy);
    }
    drawCalendar(p, cal, frac, rem, relieved, tNow);
    p.restore();
    drawFlames(p, false);
    // Text block.
    const qreal x0 = cal.right() + 22.0;
    const qreal tw = width() - x0 - 10.0;
    const QColor tc(palette().text().color());
    QFont tf(font());
    tf.setPointSizeF(9.5);
    tf.setBold(true);
    p.setFont(tf);
    QColor sub(tc);
    sub.setAlpha(170);
    p.setPen(sub);
    const qreal y = cal.top();
    p.drawText(QRectF(x0, y, tw, 18), Qt::AlignLeft | Qt::AlignVCenter, m_title.toUpper());
    // Big digits.
    QFont df(QStringLiteral("Monospace"));
    df.setStyleHint(QFont::TypeWriter);
    df.setBold(true);
    df.setPointSizeF(clamp(cal.height() * 0.24, 14.0, 34.0));
    p.setFont(df);
    const bool overdue = rem <= 0;
    QString digits;
    QColor dc;
    if (overdue) {
        digits = QStringLiteral("+") + formatTime(-rem);
        dc = QColor(m_accentColor);
    } else {
        digits = formatTime(rem + 0.999);
        dc = frac < 0.1 ? QColor(m_accentColor) : (frac < 0.25 ? QColor(QStringLiteral("#FF9F1C")) : QColor(tc));
    }
    const QFontMetricsF dfm(df);
    const qreal dy = y + 20.0;
    if (0 < frac && frac < 0.1) {
        // Each digit trembles on its own.
        qreal x = x0;
        p.setPen(dc);
        for (const QChar ch : std::as_const(digits)) {
            const qreal adv = dfm.horizontalAdvance(ch);
            const qreal jx = uniform(-1.5, 1.5);
            const qreal jy = uniform(-2, 2);
            p.drawText(QPointF(x + jx, dy + dfm.ascent() + jy), QString(ch));
            x += adv;
        }
    } else {
        p.setPen(dc);
        p.drawText(QRectF(x0, dy, tw, dfm.height()), Qt::AlignLeft | Qt::AlignVCenter, digits);
    }
    // Progress bar.
    const qreal by = dy + dfm.height() + 8.0;
    const QRectF bar(x0, by, std::min(tw, 260.0), 6.0);
    QColor track(tc);
    track.setAlpha(40);
    p.setPen(Qt::NoPen);
    p.setBrush(track);
    p.drawRoundedRect(bar, 3, 3);
    p.setBrush(!overdue ? dc : m_accentColor);
    p.drawRoundedRect(QRectF(bar.left(), bar.top(), bar.width() * clamp(frac, 0.0, 1.0), bar.height()), 3, 3);
    // Status line.
    QString status;
    if (relieved)
        status = tr("Deadline extended! Phew.");
    else if (overdue)
        status = tr("This is fine.");
    else if (frac > 0.5)
        status = tr("Plenty of time. Coffee first.");
    else if (frac > 0.25)
        status = tr("Okay, focusing now.");
    else if (frac > 0.1)
        status = tr("Why did I check email.");
    else
        status = tr("AAAAAAAAAA");
    QFont sf(font());
    sf.setPointSizeF(9.0);
    sf.setItalic(true);
    p.setFont(sf);
    p.setPen(sub);
    p.drawText(QRectF(x0, by + 12.0, tw, 18), Qt::AlignLeft | Qt::AlignVCenter, status);
    p.end();
}

void DeadlineCountdownWidget::drawFlames(QPainter &p, bool back)
{
    p.setPen(Qt::NoPen);
    for (int i = 0; i < m_flames.size(); ++i) {
        if ((i % 2 == 0) != back)
            continue;
        const Flame &f = m_flames.at(i);
        const qreal k = clamp(f.life / f.maxLife, 0.0, 1.0);
        QColor col(255, int(80 + 150 * k), int(30 * k));
        col.setAlpha(int(220 * k));
        const qreal r = f.size * (0.5 + 0.5 * k);
        QPainterPath flame;
        flame.moveTo(f.x, f.y - r * 2.0);
        flame.cubicTo(f.x + r, f.y - r * 0.6, f.x + r, f.y + r * 0.8, f.x, f.y + r);
        flame.cubicTo(f.x - r, f.y + r * 0.8, f.x - r, f.y - r * 0.6, f.x, f.y - r * 2.0);
        p.fillPath(flame, col);
    }
}

void DeadlineCountdownWidget::drawCalendar(QPainter &p, const QRectF &cal, qreal frac, qreal rem, bool relieved,
                                           qreal now)
{
    const QColor ink(QStringLiteral("#23232B"));
    const qreal r = cal.width() * 0.12;
    // Page stack shadow.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 50));
    p.drawRoundedRect(cal.translated(4, 5), r, r);
    p.setBrush(QColor(QStringLiteral("#E7E3DA")));
    p.drawRoundedRect(cal.translated(2, 2.5), r, r);
    // Page.
    const bool scorched = rem <= 0;
    const QColor page = !scorched ? QColor(QStringLiteral("#FFFDF7")) : QColor(QStringLiteral("#F3E2C8"));
    p.setPen(QPen(QColor(0, 0, 0, 40), 1.0));
    p.setBrush(page);
    p.drawRoundedRect(cal, r, r);
    // Binding.
    const QRectF band(cal.left(), cal.top(), cal.width(), cal.height() * 0.22);
    QPainterPath bp;
    bp.addRoundedRect(band, r, r);
    bp.addRect(QRectF(band.left(), band.center().y(), band.width(), band.height() / 2.0));
    p.setPen(Qt::NoPen);
    p.fillPath(bp.simplified(), m_accentColor);
    // Rings.
    p.setBrush(QColor(QStringLiteral("#B9BCC6")));
    for (qreal k : {0.3, 0.7})
        p.drawRoundedRect(QRectF(cal.left() + cal.width() * k - 3.0, cal.top() - 6.0, 6.0, 14.0), 3, 3);
    // Month label on the binding.
    QFont f(font());
    f.setBold(true);
    f.setPixelSize(std::max(7, int(band.height() * 0.55)));
    p.setFont(f);
    p.setPen(QColor(QStringLiteral("#FFFFFF")));
    p.drawText(band.adjusted(0, 3, 0, 0), Qt::AlignCenter, tr("DUE"));
    // Scorch marks.
    if (scorched) {
        p.setBrush(QColor(60, 30, 10, 60));
        p.drawEllipse(QPointF(cal.left() + cal.width() * 0.25, cal.bottom() - cal.height() * 0.15),
                      cal.width() * 0.12, cal.height() * 0.06);
        p.drawEllipse(QPointF(cal.right() - cal.width() * 0.2, cal.top() + cal.height() * 0.32),
                      cal.width() * 0.08, cal.height() * 0.05);
    }
    // Face.
    const QPointF fc(cal.center().x(), cal.top() + cal.height() * 0.58);
    const qreal s = cal.width() * 0.5;
    const qreal ex = s * 0.38;
    const qreal ey = fc.y() - s * 0.2;
    const QPen pen(ink, std::max(1.5, s * 0.07), Qt::SolidLine, Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    const qreal my = fc.y() + s * 0.35;
    if (relieved) {
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(fc.x() + d * ex - s * 0.13, ey - s * 0.05, s * 0.26, s * 0.16), 180 * 16, 180 * 16);
        p.drawArc(QRectF(fc.x() - s * 0.22, my - s * 0.2, s * 0.44, s * 0.28), 200 * 16, 140 * 16);
        // Exhale puff.
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#C9D6E8")));
        const qreal k = (now - m_reliefAt) / 1.8;
        p.drawEllipse(QPointF(fc.x() + s * 0.35 + k * s * 0.5, my - k * s * 0.2), s * 0.08 * (1 + k), s * 0.06 * (1 + k));
    } else if (scorched) {
        // "This is fine": calm closed eyes, gentle smile.
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(fc.x() + d * ex - s * 0.13, ey - s * 0.02, s * 0.26, s * 0.16), 180 * 16, 180 * 16);
        p.drawArc(QRectF(fc.x() - s * 0.18, my - s * 0.18, s * 0.36, s * 0.22), 200 * 16, 140 * 16);
    } else if (frac > 0.5) {
        // Sunglasses, smirk.
        p.setPen(QPen(ink, std::max(1.2, s * 0.05)));
        p.drawLine(QPointF(fc.x() - ex - s * 0.2, ey - s * 0.05), QPointF(fc.x() + ex + s * 0.2, ey - s * 0.05));
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0}) {
            QPainterPath lens;
            lens.addRoundedRect(QRectF(fc.x() + d * ex - s * 0.2, ey - s * 0.08, s * 0.4, s * 0.24), s * 0.08, s * 0.1);
            p.drawPath(lens);
        }
        p.setBrush(QColor(255, 255, 255, 120));
        p.drawRect(QRectF(fc.x() - ex - s * 0.12, ey - s * 0.04, s * 0.08, s * 0.04));
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        QPainterPath sm;
        sm.moveTo(fc.x() - s * 0.18, my);
        sm.quadTo(fc.x() + s * 0.05, my + s * 0.12, fc.x() + s * 0.24, my - s * 0.08);
        p.drawPath(sm);
    } else if (frac > 0.1) {
        const bool nervous = frac <= 0.25;
        const qreal look = nervous ? std::sin(now * 6.0) * s * 0.05 : 0.0;
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(fc.x() + d * ex + look, ey), s * 0.08, s * 0.1);
        p.setPen(pen);
        for (qreal d : {-1.0, 1.0}) {
            if (nervous)
                p.drawLine(QPointF(fc.x() + d * (ex + s * 0.14), ey - s * 0.2), QPointF(fc.x() + d * (ex - s * 0.12), ey - s * 0.28));
            else
                p.drawLine(QPointF(fc.x() + d * (ex + s * 0.14), ey - s * 0.28), QPointF(fc.x() + d * (ex - s * 0.12), ey - s * 0.2));
        }
        if (nervous) {
            QPainterPath m;
            m.moveTo(fc.x() - s * 0.2, my);
            for (int k = 1; k < 7; ++k)
                m.lineTo(fc.x() - s * 0.2 + k * s * 0.4 / 6.0, my + ((k % 2) ? s * 0.04 : -s * 0.04));
            p.drawPath(m);
            QColor dc(QStringLiteral("#7FD6FF"));
            const qreal k = pyMod(now * 0.9, 1.0);
            dc.setAlpha(int(255 * (1.0 - k)));
            const qreal x = fc.x() + s * 0.62;
            const qreal yy = ey - s * 0.2 + k * s * 0.5;
            QPainterPath drop;
            drop.moveTo(x, yy - 6.0);
            drop.quadTo(x + 4.0, yy + 1.0, x, yy + 3.5);
            drop.quadTo(x - 4.0, yy + 1.0, x, yy - 6.0);
            p.fillPath(drop, dc);
        } else {
            p.drawLine(QPointF(fc.x() - s * 0.16, my), QPointF(fc.x() + s * 0.16, my));
        }
    } else {
        // Panic.
        for (qreal d : {-1.0, 1.0}) {
            p.setPen(QPen(ink, std::max(1.0, s * 0.04)));
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(fc.x() + d * ex, ey), s * 0.17, s * 0.19);
            p.setPen(Qt::NoPen);
            p.setBrush(ink);
            const qreal jx = uniform(-1, 1);
            const qreal jy = uniform(-1, 1);
            p.drawEllipse(QPointF(fc.x() + d * ex + jx, ey + jy), s * 0.04, s * 0.04);
        }
        p.setPen(pen);
        p.setBrush(QColor(QStringLiteral("#7A1F1F")));
        const qreal o = 1.0 + 0.15 * std::sin(now * 25.0);
        p.drawEllipse(QPointF(fc.x(), my + s * 0.02), s * 0.16, s * 0.18 * o);
    }
}
