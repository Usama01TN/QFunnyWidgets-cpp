/**
 * AntBreadcrumbWidget: a breadcrumb bar made of actual bread crumbs, maintained by a colony of ants. Navigating
 * deeper makes ants carry new crumbs in from the right; going back makes them carry the dropped crumbs away. The
 * current crumb has a little "you are here" flag, and a few ants keep marching along the bottom.
 */
#include "antbreadcrumb.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <numeric>
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

QStringList AntBreadcrumbWidget::defaultPath()
{
    return {QStringLiteral("Home"), QStringLiteral("Projects"), QStringLiteral("QFunnyWidgets")};
}

AntBreadcrumbWidget::AntBreadcrumbWidget(QWidget *parent)
    : AntBreadcrumbWidget(Options(), parent)
{
}

AntBreadcrumbWidget::AntBreadcrumbWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_crumbColor(options.crumbColor),
      m_antColor(options.antColor),
      m_marching(options.marchingAnts)
{
    if (options.crumbClicked)
        connect(this, &AntBreadcrumbWidget::crumbClicked, this, options.crumbClicked);
    if (options.pathChanged)
        connect(this, &AntBreadcrumbWidget::pathChanged, this, options.pathChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    // State.
    for (const QString &t : options.path)
        m_crumbs.append(Crumb(t));
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AntBreadcrumbWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal AntBreadcrumbWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QFont AntBreadcrumbWidget::crumbFont(bool bold) const
{
    QFont f(font());
    f.setPointSizeF(9.5);
    f.setBold(bold);
    return f;
}

void AntBreadcrumbWidget::layoutCrumbs(bool snap)
{
    const int n = int(m_crumbs.size());
    if (!n)
        return;
    const qreal avail = width() - 12.0 - GAP * (n - 1);
    QList<qreal> natural;
    for (int i = 0; i < n; ++i) {
        const QFontMetricsF fm(crumbFont(i == n - 1));
        natural.append(fm.horizontalAdvance(m_crumbs.at(i).text) + 26.0);
    }
    // Too long for the space: short crumbs keep their size, long ones share what is left (water-filling).
    QList<qreal> widths = natural;
    if (std::accumulate(natural.cbegin(), natural.cend(), 0.0) > avail) {
        qreal remaining = avail;
        int left = n;
        QList<int> order;
        for (int i = 0; i < n; ++i)
            order.append(i);
        std::stable_sort(order.begin(), order.end(), [&natural](int a, int b) { return natural.at(a) < natural.at(b); });
        for (int i : std::as_const(order)) {
            const qreal share = std::max(36.0, remaining / left);
            widths[i] = std::min(natural.at(i), share);
            remaining -= widths.at(i);
            left -= 1;
        }
    }
    qreal x = 6.0;
    for (int i = 0; i < n; ++i) {
        Crumb &c = m_crumbs[i];
        c.width = widths.at(i);
        c.targetX = x;
        if (snap && c.state == Idle)
            c.x = x;
        x += c.width + GAP;
    }
}

QRectF AntBreadcrumbWidget::crumbRect(const Crumb &c) const
{
    const qreal h = height();
    const qreal ch = std::min(30.0, h - 16.0);
    return QRectF(c.x, (h - ch) / 2.0 - 4.0 - c.lift, c.width, ch);
}

// ---------- Public API ----------
QStringList AntBreadcrumbWidget::getPath() const
{
    QStringList path;
    for (const Crumb &c : m_crumbs)
        path.append(c.text);
    return path;
}

void AntBreadcrumbWidget::setPath(const QStringList &path)
{
    const QStringList old = getPath();
    if (path == old)
        return;
    int keep = 0;
    while (keep < std::min(path.size(), old.size()) && path.at(keep) == old.at(keep))
        ++keep;
    for (int k = 0; keep + k < m_crumbs.size(); ++k) {
        Crumb c = m_crumbs.at(keep + k);
        c.state = CarriedOut;
        c.delay = k * 0.08;
        m_leaving.append(c);
    }
    m_crumbs = m_crumbs.mid(0, keep);
    const qreal start = width() + 20.0;
    for (int k = 0; keep + k < path.size(); ++k)
        m_crumbs.append(Crumb(path.at(keep + k), start + k * 90.0, CarriedIn, 0.25 + k * 0.15));
    layoutCrumbs();
    emit pathChanged(getPath());
    update();
}

void AntBreadcrumbWidget::push(const QString &name)
{
    setPath(getPath() << name);
}

void AntBreadcrumbWidget::navigateTo(int index)
{
    index = clampInt(index, 0, int(m_crumbs.size()) - 1);
    setPath(getPath().mid(0, index + 1));
}

QColor AntBreadcrumbWidget::getCrumbColor() const { return m_crumbColor; }
QColor AntBreadcrumbWidget::getAntColor() const { return m_antColor; }
bool AntBreadcrumbWidget::isMarchingAnts() const { return m_marching; }

void AntBreadcrumbWidget::setCrumbColor(const QColor &color)
{
    m_crumbColor = color;
    update();
}

void AntBreadcrumbWidget::setAntColor(const QColor &color)
{
    m_antColor = color;
    update();
}

void AntBreadcrumbWidget::setMarchingAnts(bool marchingAnts)
{
    m_marching = marchingAnts;
}

// ---------- Qt overrides ----------
QSize AntBreadcrumbWidget::sizeHint() const
{
    return QSize(420, 56);
}

QSize AntBreadcrumbWidget::minimumSizeHint() const
{
    return QSize(160, 44);
}

void AntBreadcrumbWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void AntBreadcrumbWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void AntBreadcrumbWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutCrumbs(true);
}

void AntBreadcrumbWidget::mouseMoveEvent(QMouseEvent *e)
{
    int hover = -1;
    for (int i = 0; i < int(m_crumbs.size()) - 1; ++i) {
        const Crumb &c = m_crumbs.at(i);
        if (c.state == Idle && crumbRect(c).contains(eventPos(e)))
            hover = i;
    }
    if (hover != m_hover) {
        m_hover = hover;
        setCursor(hover >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
    QWidget::mouseMoveEvent(e);
}

void AntBreadcrumbWidget::leaveEvent(QEvent *e)
{
    m_hover = -1;
    QWidget::leaveEvent(e);
}

void AntBreadcrumbWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        for (int i = 0; i < int(m_crumbs.size()) - 1; ++i) {
            const Crumb &c = m_crumbs.at(i);
            if (c.state == Idle && crumbRect(c).contains(eventPos(e))) {
                emit crumbClicked(i);
                navigateTo(i);
                break;
            }
        }
    }
    QWidget::mouseReleaseEvent(e);
}

// ---------- Animation tick ----------
void AntBreadcrumbWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    if (!m_layoutDone && width() > 0) {
        layoutCrumbs(true);
        m_layoutDone = true;
    }
    for (Crumb &c : m_crumbs) {
        if (c.delay > 0) {
            c.delay -= dt;
            continue;
        }
        if (c.state == CarriedIn) {
            c.lift = 4.0;
            c.x = std::max(c.targetX, c.x - CARRY_SPEED * dt);
            if (c.x <= c.targetX + 0.5)
                c.state = Idle;
        } else {
            c.lift = std::max(0.0, c.lift - dt * 30.0);
            c.x += (c.targetX - c.x) * std::min(1.0, dt * 10.0); // re-flow after siblings change
        }
    }
    QList<Crumb> alive;
    for (Crumb c : std::as_const(m_leaving)) {
        if (c.delay > 0) {
            c.delay -= dt;
            c.lift = std::min(5.0, c.lift + dt * 40.0);
        } else {
            c.lift = 5.0;
            c.x += CARRY_SPEED * dt;
        }
        if (c.x < width() + 30)
            alive.append(c);
    }
    m_leaving = alive;
    update();
}

// ---------- Painting ----------
void AntBreadcrumbWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal h = height();
    const qreal ground = h - 7.0;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Marching ants underneath (crumbs are painted over them).
    if (m_marching) {
        const qreal span = width() + 40.0;
        for (int k = 0; k < 7; ++k) {
            const qreal x = pyMod(tNow * 28.0 + k * span / 7.0, span) - 20.0;
            drawAnt(p, QPointF(x, ground), tNow * 12.0 + k, 1.0);
        }
    }
    const int n = int(m_crumbs.size());
    // Separators.
    QColor sep(palette().text().color());
    sep.setAlpha(110);
    p.setPen(QPen(sep, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < n - 1; ++i) {
        const Crumb &c = m_crumbs.at(i);
        if (c.state != Idle)
            continue;
        const QRectF r = crumbRect(c);
        const qreal sx = r.right() + GAP / 2.0;
        const qreal cy = r.center().y();
        p.drawLine(QPointF(sx - 3, cy - 5), QPointF(sx + 2, cy));
        p.drawLine(QPointF(sx + 2, cy), QPointF(sx - 3, cy + 5));
    }
    for (int i = 0; i < n; ++i)
        drawCrumb(p, m_crumbs.at(i), i == n - 1, i == m_hover, tNow);
    for (const Crumb &c : std::as_const(m_leaving))
        drawCrumb(p, c, false, false, tNow);
    p.end();
}

void AntBreadcrumbWidget::drawCrumb(QPainter &p, const Crumb &c, bool current, bool hovered, qreal now)
{
    const QRectF r = crumbRect(c);
    if (r.right() < -10 || r.left() > width() + 10)
        return;
    const bool carried = (c.state == CarriedIn || c.state == CarriedOut) && c.delay <= 0;
    // Ants carrying it: under the crumb, legs going.
    if (carried) {
        for (int k = 0; k < 3; ++k) {
            const qreal ax = r.left() + r.width() * (0.2 + 0.3 * k);
            drawAnt(p, QPointF(ax, r.bottom() + 5.0), now * 18.0 + k, 1.0);
        }
    }
    PyRandom rnd(c.seed);
    QColor col = hovered ? m_crumbColor.lighter(110) : m_crumbColor;
    if (current)
        col = col.lighter(106);
    // Slightly lumpy crumb outline.
    QPainterPath path;
    const int steps = 22;
    const qreal cx = r.center().x();
    const qreal cy = r.center().y();
    const qreal rx = r.width() / 2.0;
    const qreal ry = r.height() / 2.0;
    for (int k = 0; k <= steps; ++k) {
        const qreal a = k / qreal(steps) * 2.0 * PI;
        const qreal bump = 1.0 + rnd.uniform(-0.04, 0.04);
        // Superellipse-ish so long crumbs stay pill shaped.
        const qreal sx = std::pow(std::abs(std::cos(a)), 0.45) * (std::cos(a) >= 0 ? 1 : -1);
        const qreal sy = std::pow(std::abs(std::sin(a)), 0.45) * (std::sin(a) >= 0 ? 1 : -1);
        const QPointF pt(cx + sx * rx * bump, cy + sy * ry * bump);
        if (k == 0)
            path.moveTo(pt);
        else
            path.lineTo(pt);
    }
    path.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 40));
    p.drawPath(path.translated(0, 2));
    p.setPen(QPen(QColor(QStringLiteral("#A0682C")), 1.4));
    p.setBrush(col);
    p.drawPath(path);
    // Sesame / texture dots.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 245, 220, 150));
    const int dots = std::max(2, int(r.width() / 22));
    for (int k = 0; k < dots; ++k) {
        const qreal dx = rnd.uniform(8, r.width() - 8);
        const qreal dy = rnd.uniform(4, 8);
        p.drawEllipse(QPointF(r.left() + dx, r.top() + dy), 1.6, 1.0);
    }
    const QFont f = crumbFont(current);
    p.setFont(f);
    p.setPen(QColor(QStringLiteral("#4A2A10")));
    const QFontMetricsF fm(f);
    p.drawText(r.adjusted(10, 0, -10, 0), Qt::AlignCenter, fm.elidedText(c.text, Qt::ElideMiddle, r.width() - 20));
    if (current && !carried) {
        // "You are here" flag planted by the ants.
        const qreal fx = r.right() - 6.0;
        p.setPen(QPen(QColor(QStringLiteral("#5A3A20")), 1.2));
        p.drawLine(QPointF(fx, r.top() + 2), QPointF(fx, r.top() - 9));
        QPainterPath flag;
        const qreal wave = std::sin(now * 6.0) * 1.2;
        flag.moveTo(fx, r.top() - 9);
        flag.lineTo(fx + 8, r.top() - 7 + wave);
        flag.lineTo(fx, r.top() - 4);
        flag.closeSubpath();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#E63946")));
        p.drawPath(flag);
    }
}

void AntBreadcrumbWidget::drawAnt(QPainter &p, const QPointF &c, qreal phase, qreal scale)
{
    const QColor ac = m_antColor;
    const qreal s = 4.0 * scale;
    const qreal y = c.y() - s * 0.9;
    p.setPen(QPen(ac, 0.9, Qt::SolidLine, Qt::RoundCap));
    const qreal legs[3] = {-s * 0.4, 0.0, s * 0.4};
    for (int k = 0; k < 3; ++k) {
        const qreal sw = std::sin(phase + k * 2.1) * s * 0.35;
        p.drawLine(QPointF(c.x() + legs[k], y), QPointF(c.x() + legs[k] + sw, c.y()));
    }
    p.drawLine(QPointF(c.x() + s * 1.25, y - s * 0.3), QPointF(c.x() + s * 1.7, y - s * 0.9));
    p.setPen(Qt::NoPen);
    p.setBrush(ac);
    p.drawEllipse(QPointF(c.x() - s * 0.75, y), s * 0.6, s * 0.45);
    p.drawEllipse(QPointF(c.x(), y), s * 0.35, s * 0.3);
    p.drawEllipse(QPointF(c.x() + s * 0.8, y - s * 0.1), s * 0.4, s * 0.35);
}
