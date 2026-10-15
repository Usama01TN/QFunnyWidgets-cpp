/**
 * EmptyStateWidget: a "nothing here yet" placeholder: a tumbleweed rolls through a desert past a bored, chirping
 * cactus, above a title, a subtitle and an optional call-to-action button.
 */
#include "emptystate.h"

#include <QFontMetricsF>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <utility>

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

inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

/**
 * Bit-exact port of Python's random.Random for integer seeds in [0, 2^32): MT19937 seeded with init_by_array(),
 * random() with 53-bit precision and uniform(a, b) = a + (b - a) * random(). Used so that seeded decorations
 * look exactly like in the Python version.
 */
class PyRandom
{
public:
    explicit PyRandom(quint32 seed)
    {
        initByArray(&seed, 1);
    }

    double random()
    {
        const quint32 a = genrand() >> 5;
        const quint32 b = genrand() >> 6;
        return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
    }

    double uniform(double a, double b)
    {
        return a + (b - a) * random();
    }

private:
    static const int N = 624;
    static const int M = 397;
    quint32 m_mt[N];
    int m_mti = N + 1;

    void initGenrand(quint32 s)
    {
        m_mt[0] = s;
        for (m_mti = 1; m_mti < N; ++m_mti)
            m_mt[m_mti] = 1812433253U * (m_mt[m_mti - 1] ^ (m_mt[m_mti - 1] >> 30)) + quint32(m_mti);
    }

    void initByArray(const quint32 *key, int keyLength)
    {
        initGenrand(19650218U);
        int i = 1;
        int j = 0;
        for (int k = N > keyLength ? N : keyLength; k; --k) {
            m_mt[i] = (m_mt[i] ^ ((m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) * 1664525U)) + key[j] + quint32(j);
            ++i;
            ++j;
            if (i >= N) {
                m_mt[0] = m_mt[N - 1];
                i = 1;
            }
            if (j >= keyLength)
                j = 0;
        }
        for (int k = N - 1; k; --k) {
            m_mt[i] = (m_mt[i] ^ ((m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) * 1566083941U)) - quint32(i);
            ++i;
            if (i >= N) {
                m_mt[0] = m_mt[N - 1];
                i = 1;
            }
        }
        m_mt[0] = 0x80000000U;
    }

    quint32 genrand()
    {
        static const quint32 mag01[2] = {0x0U, 0x9908b0dfU};
        quint32 y;
        if (m_mti >= N) {
            int kk;
            for (kk = 0; kk < N - M; ++kk) {
                y = (m_mt[kk] & 0x80000000U) | (m_mt[kk + 1] & 0x7fffffffU);
                m_mt[kk] = m_mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1U];
            }
            for (; kk < N - 1; ++kk) {
                y = (m_mt[kk] & 0x80000000U) | (m_mt[kk + 1] & 0x7fffffffU);
                m_mt[kk] = m_mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1U];
            }
            y = (m_mt[N - 1] & 0x80000000U) | (m_mt[0] & 0x7fffffffU);
            m_mt[N - 1] = m_mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1U];
            m_mti = 0;
        }
        y = m_mt[m_mti++];
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680U;
        y ^= (y << 15) & 0xefc60000U;
        y ^= (y >> 18);
        return y;
    }
};

} // namespace

EmptyStateWidget::EmptyStateWidget(QWidget *parent)
    : EmptyStateWidget(Options(), parent)
{
}

EmptyStateWidget::EmptyStateWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_title(options.title),
      m_subtitle(options.subtitle),
      m_actionText(options.actionText),
      m_accentColor(options.accentColor),
      m_sandColor(options.sandColor),
      m_speed(std::max(0.05, qreal(options.speed)))
{
    if (options.actionClicked)
        connect(this, &EmptyStateWidget::actionClicked, this, options.actionClicked);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    // State: tumbleweed made of random loops (seeded so it always looks the same).
    PyRandom rnd(7);
    for (int i = 0; i < 14; ++i) {
        Loop l;
        l.ox = rnd.uniform(-0.35, 0.35);
        l.oy = rnd.uniform(-0.35, 0.35);
        l.rx = rnd.uniform(0.35, 0.65);
        l.ry = rnd.uniform(0.25, 0.55);
        l.ang = rnd.uniform(0, 180);
        m_loops.append(l);
    }
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal EmptyStateWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

// ---------- Public API ----------
QString EmptyStateWidget::getTitle() const { return m_title; }
QString EmptyStateWidget::getSubtitle() const { return m_subtitle; }
QString EmptyStateWidget::getActionText() const { return m_actionText; }
QColor EmptyStateWidget::getAccentColor() const { return m_accentColor; }
QColor EmptyStateWidget::getSandColor() const { return m_sandColor; }
qreal EmptyStateWidget::getSpeed() const { return m_speed; }

void EmptyStateWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void EmptyStateWidget::setSubtitle(const QString &subtitle)
{
    m_subtitle = subtitle;
    update();
}

void EmptyStateWidget::setActionText(const QString &actionText)
{
    m_actionText = actionText;
    update();
}

void EmptyStateWidget::setAccentColor(const QColor &color)
{
    if (m_accentColor != color) {
        m_accentColor = color;
        update();
    }
}

void EmptyStateWidget::setSandColor(const QColor &color)
{
    if (m_sandColor != color) {
        m_sandColor = color;
        update();
    }
}

void EmptyStateWidget::setSpeed(qreal speed)
{
    m_speed = std::max(0.05, speed);
}

// ---------- Qt overrides ----------
QSize EmptyStateWidget::sizeHint() const
{
    return QSize(420, 300);
}

QSize EmptyStateWidget::minimumSizeHint() const
{
    return QSize(220, 180);
}

void EmptyStateWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void EmptyStateWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void EmptyStateWidget::mouseMoveEvent(QMouseEvent *e)
{
    const bool hover = m_btnRect.contains(eventPos(e));
    if (hover != m_btnHover) {
        m_btnHover = hover;
        setCursor(hover ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
    QWidget::mouseMoveEvent(e);
}

void EmptyStateWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_btnRect.contains(eventPos(e)))
        m_btnPress = true;
    QWidget::mousePressEvent(e);
}

void EmptyStateWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_btnPress && m_btnRect.contains(eventPos(e)))
        emit actionClicked();
    m_btnPress = false;
    QWidget::mouseReleaseEvent(e);
}

// ---------- Painting ----------
void EmptyStateWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal w = width();
    const qreal h = height();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const qreal ground = h * 0.8;
    const qreal unit = clamp(std::min(w, h) / 10.0, 8.0, 40.0);
    drawScene(p, w, h, ground, unit, tNow);
    drawText(p, w, ground, unit);
    p.end();
}

void EmptyStateWidget::drawScene(QPainter &p, qreal w, qreal h, qreal ground, qreal unit, qreal now)
{
    const QColor sand = m_sandColor;
    // Setting sun, half hidden behind the dunes.
    QColor sun(QStringLiteral("#FFB703"));
    sun.setAlpha(60);
    p.setPen(Qt::NoPen);
    p.setBrush(sun);
    p.drawEllipse(QPointF(w * 0.16, ground - unit * 0.15), unit * 1.0, unit * 1.0);
    sun.setAlpha(150);
    p.setBrush(sun);
    p.drawEllipse(QPointF(w * 0.16, ground - unit * 0.15), unit * 0.65, unit * 0.65);
    // Dunes.
    QPainterPath dune;
    dune.moveTo(0, h);
    dune.lineTo(0, ground);
    dune.cubicTo(w * 0.25, ground - unit * 0.6, w * 0.45, ground + unit * 0.25, w * 0.7, ground - unit * 0.35);
    dune.cubicTo(w * 0.85, ground - unit * 0.6, w * 0.95, ground, w, ground - unit * 0.1);
    dune.lineTo(w, h);
    dune.closeSubpath();
    QLinearGradient grad(QPointF(0, ground - unit), QPointF(0, h));
    QColor top(sand);
    top.setAlpha(170);
    QColor bottom(sand.darker(130));
    bottom.setAlpha(120);
    grad.setColorAt(0.0, top);
    grad.setColorAt(1.0, bottom);
    p.setBrush(grad);
    p.drawPath(dune);
    // Cactus.
    const qreal cx = w * 0.82;
    const qreal cbase = ground + unit * 0.05;
    const QColor cg(QStringLiteral("#2A9D8F"));
    const QPen pen(cg.darker(130), std::max(1.0, unit * 0.06));
    p.setPen(pen);
    p.setBrush(cg);
    const qreal cw = unit * 0.55;
    const qreal ch = unit * 2.6;
    p.drawRoundedRect(QRectF(cx - cw / 2.0, cbase - ch, cw, ch + 2.0), cw / 2.0, cw / 2.0);
    const qreal sway = std::sin(now * 1.3) * unit * 0.05;
    QPainterPath arm;
    arm.addRoundedRect(QRectF(cx - cw * 1.6 + sway, cbase - ch * 0.78, cw * 0.5, ch * 0.42), cw * 0.25, cw * 0.25);
    arm.addRoundedRect(QRectF(cx - cw * 1.6 + sway, cbase - ch * 0.42, cw * 1.2, cw * 0.45), cw * 0.22, cw * 0.22);
    arm.addRoundedRect(QRectF(cx + cw * 1.1 - sway, cbase - ch * 0.62, cw * 0.5, ch * 0.32), cw * 0.25, cw * 0.25);
    arm.addRoundedRect(QRectF(cx + cw * 0.4 - sway, cbase - ch * 0.35, cw * 1.2, cw * 0.45), cw * 0.22, cw * 0.22);
    p.drawPath(arm.simplified());
    // Cactus face: bored.
    const QColor ink(QStringLiteral("#1D3B36"));
    p.setPen(QPen(ink, std::max(1.0, unit * 0.06), Qt::SolidLine, Qt::RoundCap));
    const qreal fy = cbase - ch * 0.75;
    for (qreal d : {-1.0, 1.0})
        p.drawLine(QPointF(cx + d * cw * 0.2 - cw * 0.08, fy), QPointF(cx + d * cw * 0.2 + cw * 0.08, fy));
    p.drawLine(QPointF(cx - cw * 0.12, fy + cw * 0.35), QPointF(cx + cw * 0.12, fy + cw * 0.35));
    // Chirp.
    const qreal cyc = pyMod(now, 3.2);
    if (cyc < 1.2) {
        QFont f(font());
        f.setItalic(true);
        f.setPointSizeF(std::max(6.5, unit * 0.32));
        p.setFont(f);
        QColor tc(palette().text().color());
        tc.setAlpha(int(170 * std::sin(cyc / 1.2 * PI)));
        p.setPen(tc);
        const QString word = (int(now / 3.2) % 3) ? QStringLiteral("chirp") : QStringLiteral("chirp chirp");
        p.drawText(QPointF(cx + cw * 1.3, cbase - unit * 0.2 - cyc * unit * 0.3), word);
    }
    // Tumbleweed.
    const qreal r = unit * 0.75;
    const qreal speed = unit * 4.5 * m_speed;
    const qreal span = w + 4.0 * r;
    const qreal period = span / speed + 1.5; // short pause off-screen between passes
    const qreal t = pyMod(now, period);
    const qreal x = -2.0 * r + t * speed;
    if (x < w + 2.0 * r) {
        const qreal ph = t * 3.2 * m_speed;
        const qreal hop = std::abs(std::sin(ph)) * unit * 0.9 * (0.6 + 0.4 * std::abs(std::sin(ph * 0.37)));
        const qreal y = ground + unit * 0.05 - r - hop;
        // Shadow.
        const QColor sh(0, 0, 0, int(50 * (1.0 - hop / (unit * 1.2))));
        p.setPen(Qt::NoPen);
        p.setBrush(sh);
        p.drawEllipse(QPointF(x, ground + unit * 0.08), r * (0.9 - hop / (unit * 4.0)), r * 0.15);
        // Wind streaks.
        QColor wc(palette().text().color());
        wc.setAlpha(60);
        p.setPen(QPen(wc, std::max(1.0, unit * 0.05), Qt::SolidLine, Qt::RoundCap));
        for (int k = 0; k < 3; ++k) {
            const qreal ly = y - r * 0.5 + k * r * 0.5;
            const qreal lx = x - r * 1.5 - k * r * 0.3;
            p.drawLine(QPointF(lx - r * 0.9, ly), QPointF(lx, ly));
        }
        p.save();
        p.translate(x, y);
        p.rotate((x / r) * 180.0 / PI);
        const QColor weed(QStringLiteral("#A0703A"));
        p.setPen(QPen(weed, std::max(1.0, unit * 0.05)));
        p.setBrush(Qt::NoBrush);
        for (const Loop &l : std::as_const(m_loops)) {
            p.save();
            p.translate(l.ox * r, l.oy * r);
            p.rotate(l.ang);
            p.drawEllipse(QPointF(0, 0), l.rx * r, l.ry * r);
            p.restore();
        }
        p.setPen(QPen(weed.darker(120), std::max(1.0, unit * 0.06)));
        p.drawEllipse(QPointF(0, 0), r * 0.95, r * 0.95);
        p.restore();
    }
}

void EmptyStateWidget::drawText(QPainter &p, qreal w, qreal ground, qreal unit)
{
    QFont tf(font());
    tf.setBold(true);
    tf.setPointSizeF(clamp(unit * 0.55, 11.0, 18.0));
    QFont sf(font());
    sf.setPointSizeF(clamp(unit * 0.36, 8.5, 11.0));
    const QFontMetricsF tfm(tf);
    const QFontMetricsF sfm(sf);
    const qreal tw = std::min(w - 30.0, 420.0);
    const QRectF sr = sfm.boundingRect(QRectF(0, 0, tw, 500), Qt::TextWordWrap | Qt::AlignHCenter, m_subtitle);
    const qreal btnH = !m_actionText.isEmpty() ? 34.0 : 0.0;
    const qreal blockH = tfm.height() + 6.0 + sr.height() + (btnH ? 16.0 + btnH : 0.0);
    qreal y = std::max(8.0, (ground - unit * 1.2 - blockH) / 2.0);
    const qreal x = (w - tw) / 2.0;
    const QColor tc(palette().text().color());
    p.setFont(tf);
    p.setPen(tc);
    p.drawText(QRectF(x, y, tw, tfm.height()), Qt::AlignCenter, m_title);
    y += tfm.height() + 6.0;
    QColor sub(tc);
    sub.setAlpha(165);
    p.setFont(sf);
    p.setPen(sub);
    p.drawText(QRectF(x, y, tw, sr.height()), Qt::TextWordWrap | Qt::AlignHCenter, m_subtitle);
    y += sr.height() + 16.0;
    if (!m_actionText.isEmpty()) {
        QFont bf(font());
        bf.setBold(true);
        bf.setPointSizeF(9.5);
        const QFontMetricsF bfm(bf);
        const qreal adv = bfm.horizontalAdvance(m_actionText);
        const qreal bw = adv + 52.0;
        QRectF rect((w - bw) / 2.0, y, bw, btnH);
        if (m_btnPress)
            rect.adjust(2, 2, -2, -2);
        m_btnRect = rect;
        const QColor ac = m_btnHover ? m_accentColor.lighter(112) : m_accentColor;
        p.setPen(Qt::NoPen);
        p.setBrush(ac);
        p.drawRoundedRect(rect, btnH / 2.0, btnH / 2.0);
        p.setFont(bf);
        p.setPen(QColor(QStringLiteral("#FFFFFF")));
        // "+" icon.
        const QPointF plus(rect.left() + 20.0, rect.center().y());
        p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), 2.0, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(plus - QPointF(5, 0), plus + QPointF(5, 0));
        p.drawLine(plus - QPointF(0, 5), plus + QPointF(0, 5));
        p.drawText(rect.adjusted(24, 0, -8, 0), Qt::AlignCenter, m_actionText);
    } else {
        m_btnRect = QRectF();
    }
}
