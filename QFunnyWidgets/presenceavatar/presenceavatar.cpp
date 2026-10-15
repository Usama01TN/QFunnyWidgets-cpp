/**
 * PresenceAvatarWidget: an initials avatar whose online status is its mood: online (pulsing ring, eyes follow the
 * cursor, waves when clicked), away (asleep with a floating "z"), busy (headphones, frowning at the work) or
 * offline (a translucent ghost). Hovering shows a greeting bubble.
 */
#include "presenceavatar.h"

#include <QCursor>
#include <QFontMetricsF>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace {

const qreal PI = 3.14159265358979323846;

inline QPoint eventPos(const QMouseEvent *e)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position().toPoint();
#else
    return e->pos();
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

inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

} // namespace

QString initialsOf(const QString &name)
{
    QString n = name;
    n.replace(QLatin1Char('-'), QLatin1Char(' '));
    const QStringList words = n.split(QRegularExpression(QStringLiteral("\\s+"), QRegularExpression::UseUnicodePropertiesOption),
                                         Qt::SkipEmptyParts);
    if (words.isEmpty())
        return QStringLiteral("?");
    if (words.size() == 1)
        return words.first().left(2).toUpper();
    return (words.first().left(1) + words.last().left(1)).toUpper();
}

QColor colorOf(const QString &name)
{
    qint64 h = 0;
    for (auto ch : name.toUcs4())
        h = (h * 31 + qint64(ch)) & 0xFFFFFF;
    return QColor::fromHsv(int(h % 360), 120, 215);
}

QColor PresenceAvatarWidget::statusColor(int status)
{
    switch (status) {
    case Online:
        return QColor(QStringLiteral("#3BCEAC"));
    case Away:
        return QColor(QStringLiteral("#FFC93C"));
    case Busy:
        return QColor(QStringLiteral("#E63946"));
    default:
        return QColor(QStringLiteral("#8B8FA3"));
    }
}

QString PresenceAvatarWidget::greeting(int status)
{
    switch (status) {
    case Online:
        return QStringLiteral("Hi!");
    case Away:
        return QStringLiteral("zzz... 5 more min");
    case Busy:
        return QStringLiteral("Busy. Shoo.");
    default:
        return QStringLiteral("...boo.");
    }
}

PresenceAvatarWidget::PresenceAvatarWidget(QWidget *parent)
    : PresenceAvatarWidget(Options(), parent)
{
}

PresenceAvatarWidget::PresenceAvatarWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_name(options.name),
      m_status(Status(clampInt(options.status, 0, 3))),
      m_color(options.color),
      m_badgeBorderColor(options.badgeBorderColor)
{
    if (options.clicked)
        connect(this, &PresenceAvatarWidget::clicked, this, options.clicked);
    if (options.statusChanged)
        connect(this, &PresenceAvatarWidget::statusChanged, this, options.statusChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    updateToolTip();
    // State.
    m_blinkAt = 1.5 + uniform(0, 2);
    // Timing.
    m_clock.start();
    m_phase = uniform(0, 6.0); // so a row of avatars does not move in sync
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PresenceAvatarWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal PresenceAvatarWidget::now() const
{
    return m_clock.elapsed() / 1000.0 + m_phase;
}

void PresenceAvatarWidget::updateToolTip()
{
    const QMetaEnum me = QMetaEnum::fromType<Status>();
    setToolTip(QStringLiteral("%1 (%2)").arg(m_name, QString::fromLatin1(me.valueToKey(m_status)).toLower()));
}

QColor PresenceAvatarWidget::baseColor() const
{
    return m_color.isValid() ? QColor(m_color) : colorOf(m_name);
}

// ---------- Public API ----------
QString PresenceAvatarWidget::getInitials() const
{
    return initialsOf(m_name);
}

QString PresenceAvatarWidget::getName() const
{
    return m_name;
}

void PresenceAvatarWidget::setName(const QString &name)
{
    m_name = name;
    updateToolTip();
    update();
}

PresenceAvatarWidget::Status PresenceAvatarWidget::getStatus() const
{
    return m_status;
}

void PresenceAvatarWidget::setStatus(int status)
{
    const Status st = Status(clampInt(status, 0, 3));
    if (st != m_status) {
        m_status = st;
        updateToolTip();
        emit statusChanged(int(st));
        update();
    }
}

QColor PresenceAvatarWidget::getColor() const
{
    return baseColor();
}

void PresenceAvatarWidget::setColor(const QColor &color)
{
    m_color = color;
    update();
}

QColor PresenceAvatarWidget::getBadgeBorderColor() const
{
    return m_badgeBorderColor;
}

void PresenceAvatarWidget::setBadgeBorderColor(const QColor &color)
{
    m_badgeBorderColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize PresenceAvatarWidget::sizeHint() const
{
    return QSize(130, 150);
}

QSize PresenceAvatarWidget::minimumSizeHint() const
{
    return QSize(48, 56);
}

void PresenceAvatarWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void PresenceAvatarWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void PresenceAvatarWidget::enterEvent(QEnterEvent *e)
#else
void PresenceAvatarWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void PresenceAvatarWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void PresenceAvatarWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(eventPos(e))) {
        m_clickAt = now();
        emit clicked();
    }
    QWidget::mouseReleaseEvent(e);
}

// ---------- Animation tick ----------
void PresenceAvatarWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    m_hover += ((m_hovered ? 1.0 : 0.0) - m_hover) * std::min(1.0, dt * 10.0);
    if (t >= m_blinkAt + 0.14)
        m_blinkAt = t + uniform(2.0, 5.0);
    update();
}

// ---------- Painting ----------
void PresenceAvatarWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal w = width();
    const qreal h = height();
    const int st = int(m_status);
    const qreal R = std::max(10.0, std::min(w * 0.38, h * 0.33));
    QPointF c(w / 2.0, h - R * 1.2 - 4.0);
    qreal hop = st != Offline ? -std::abs(std::sin(tNow * 9.0)) * R * 0.08 * m_hover : 0.0;
    if (st == Offline)
        hop = std::sin(tNow * 1.8) * R * 0.08; // ghostly float
    c = QPointF(c.x(), c.y() + hop);
    const QColor sc = statusColor(st);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Status ring (pulses when online).
    if (st != Offline) {
        QColor ring(sc);
        if (st == Online)
            ring.setAlpha(int(140 + 115 * (0.5 + 0.5 * std::sin(tNow * 3.0))));
        p.setPen(QPen(ring, std::max(2.0, R * 0.08)));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(c, R * 1.12, R * 1.12);
    }
    QColor base = baseColor();
    p.save();
    if (st == Offline) {
        p.setOpacity(0.6);
        const int g = int(base.red() * 0.3 + base.green() * 0.59 + base.blue() * 0.11);
        base = QColor(g, g, g < 243 ? int(g * 1.05) : g);
    }
    // Head (a ghost when offline).
    QPainterPath head;
    if (st == Offline) {
        head.moveTo(-R, 0);
        head.arcTo(QRectF(-R, -R, 2 * R, 2 * R), 180, -180);
        head.lineTo(R, R * 0.75);
        const int n = 4;
        for (int k = 0; k < n; ++k) {
            const qreal x0 = R - k * (2 * R / n);
            const qreal wob = std::sin(tNow * 4.0 + k) * R * 0.06;
            head.quadTo(x0 - R / n * 0.5, R * 1.05 + wob, x0 - R / n, R * 0.75);
        }
        head.closeSubpath();
    } else {
        head.addEllipse(QPointF(0, 0), R, R);
    }
    head.translate(c);
    QRadialGradient grad(QPointF(c.x() - R * 0.35, c.y() - R * 0.4), R * 1.7);
    grad.setColorAt(0.0, base.lighter(118));
    grad.setColorAt(1.0, base.darker(118));
    p.setPen(QPen(base.darker(135), std::max(1.0, R * 0.03)));
    p.setBrush(grad);
    p.drawPath(head);
    drawFace(p, c, R, st, tNow);
    // Initials.
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(std::max(6.0, R * 0.36));
    p.setFont(f);
    p.setPen(QColor(255, 255, 255, 235));
    p.drawText(QRectF(c.x() - R, c.y() + R * 0.05, 2 * R, R * 0.6), Qt::AlignCenter, initialsOf(m_name));
    if (st == Busy)
        drawHeadphones(p, c, R);
    p.restore();
    // Status badge.
    drawBadge(p, QPointF(c.x() + R * 0.78, c.y() + R * 0.78), R * 0.27, st, sc);
    // Hover bubble.
    if (m_hover > 0.05)
        drawBubble(p, QPointF(c.x() + R * 0.35, c.y() - R * 1.2), greeting(st));
    p.end();
}

void PresenceAvatarWidget::drawFace(QPainter &p, const QPointF &c, qreal R, int st, qreal now)
{
    const QColor ink(QStringLiteral("#1E1E28"));
    const qreal ex = R * 0.33;
    const qreal ey = c.y() - R * 0.3;
    const qreal er = R * 0.17;
    const QPen pen(ink, std::max(1.2, R * 0.06), Qt::SolidLine, Qt::RoundCap);
    const bool waving = now - m_clickAt < 1.0;
    if (st == Away || (st == Online && waving)) {
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const int a0 = st == Away ? 180 * 16 : 20 * 16;
        const int a1 = st == Away ? 180 * 16 : 140 * 16;
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(c.x() + d * ex - er, ey - er * 0.5, er * 2, er * 1.2), a0, a1);
        if (st == Away) {
            QFont f(font());
            f.setBold(true);
            const qreal ph = pyMod(now * 0.6, 1.0);
            f.setPointSizeF(std::max(5.0, R * (0.18 + 0.12 * ph)));
            p.setFont(f);
            QColor zc(palette().text().color());
            zc.setAlpha(int(220 * std::sin(ph * PI)));
            p.setPen(zc);
            p.drawText(QPointF(c.x() - R * 0.9 - ph * R * 0.3, c.y() - R * 0.7 - ph * R * 0.5), QStringLiteral("z"));
        }
        return;
    }
    if (st == Offline) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(40, 40, 50, 200));
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(c.x() + d * ex, ey), er * 0.7, er * 1.1);
        return;
    }
    const bool blinking = m_blinkAt <= now && now < m_blinkAt + 0.14;
    const QPoint cur = mapFromGlobal(QCursor::pos());
    qreal lx = clamp((cur.x() - c.x()) / 150.0, -1.0, 1.0);
    qreal ly = clamp((cur.y() - ey) / 150.0, -1.0, 1.0);
    if (st == Busy) {
        lx = 0.0; // eyes on the work
        ly = 0.6;
    }
    for (qreal d : {-1.0, 1.0}) {
        const qreal x = c.x() + d * ex;
        if (blinking) {
            p.setPen(pen);
            p.drawLine(QPointF(x - er, ey), QPointF(x + er, ey));
            continue;
        }
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(x, ey), er, er * 1.1);
        p.setBrush(ink);
        p.drawEllipse(QPointF(x + lx * er * 0.45, ey + ly * er * 0.45), er * 0.52, er * 0.56);
    }
    if (st == Busy) {
        p.setPen(pen);
        for (qreal d : {-1.0, 1.0}) {
            const qreal x = c.x() + d * ex;
            p.drawLine(QPointF(x + d * er * 1.1, ey - er * 1.7), QPointF(x - d * er * 0.9, ey - er * 1.2));
        }
    }
}

void PresenceAvatarWidget::drawHeadphones(QPainter &p, const QPointF &c, qreal R)
{
    const QColor dark(QStringLiteral("#2B2D36"));
    p.setPen(QPen(dark, std::max(2.0, R * 0.12), Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(c.x() - R * 1.02, c.y() - R * 1.12, R * 2.04, R * 1.9), 10 * 16, 160 * 16);
    p.setPen(Qt::NoPen);
    p.setBrush(dark);
    for (qreal d : {-1.0, 1.0})
        p.drawRoundedRect(QRectF(c.x() + d * R * 0.98 - R * 0.17, c.y() - R * 0.42, R * 0.34, R * 0.55), R * 0.12, R * 0.12);
    p.setBrush(QColor(QStringLiteral("#E63946")));
    for (qreal d : {-1.0, 1.0})
        p.drawEllipse(QPointF(c.x() + d * R * 0.98, c.y() - R * 0.15), R * 0.06, R * 0.06);
}

void PresenceAvatarWidget::drawBadge(QPainter &p, const QPointF &c, qreal r, int st, const QColor &sc)
{
    p.setPen(QPen(m_badgeBorderColor, std::max(1.5, r * 0.3)));
    p.setBrush(st != Offline ? sc : m_badgeBorderColor);
    p.drawEllipse(c, r, r);
    if (st == Away) {
        // Crescent moon cut-out.
        p.setPen(Qt::NoPen);
        p.setBrush(m_badgeBorderColor);
        p.drawEllipse(QPointF(c.x() - r * 0.35, c.y() - r * 0.3), r * 0.62, r * 0.62);
    } else if (st == Busy) {
        p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), std::max(1.5, r * 0.3), Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(c.x() - r * 0.45, c.y()), QPointF(c.x() + r * 0.45, c.y()));
    } else if (st == Offline) {
        p.setPen(QPen(sc, std::max(1.5, r * 0.28)));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(c, r * 0.5, r * 0.5);
    }
}

void PresenceAvatarWidget::drawBubble(QPainter &p, const QPointF &anchor, const QString &text)
{
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(8.0);
    const QFontMetricsF fm(f);
    const qreal tw = fm.horizontalAdvance(text);
    const qreal bw = tw + 16.0;
    const qreal bh = fm.height() + 8.0;
    const qreal bx = clamp(anchor.x() - bw * 0.3, 2.0, width() - bw - 2.0);
    const qreal by = std::max(2.0, anchor.y() - bh - 6.0);
    p.save();
    p.setOpacity(m_hover);
    QPainterPath path;
    path.addRoundedRect(QRectF(bx, by, bw, bh), bh / 2.0, bh / 2.0);
    QPainterPath tail;
    const qreal tx = clamp(anchor.x(), bx + 8.0, bx + bw - 8.0);
    tail.moveTo(tx - 5.0, by + bh - 1.0);
    tail.lineTo(tx - 2.0, by + bh + 6.0);
    tail.lineTo(tx + 5.0, by + bh - 1.0);
    tail.closeSubpath();
    p.setPen(QPen(QColor(20, 20, 30, 60), 1.0));
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    p.drawPath(path.united(tail));
    p.setFont(f);
    p.setPen(QColor(QStringLiteral("#22222A")));
    p.drawText(QRectF(bx, by, bw, bh), Qt::AlignCenter, text);
    p.restore();
}
