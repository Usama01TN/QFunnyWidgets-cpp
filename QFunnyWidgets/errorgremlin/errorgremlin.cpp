/**
 * ErrorGremlinWidget: an inline alert banner with a severity-matched gremlin: sleeping for info, peeking and waving
 * a wrench for warnings, chewing a sparking cable for errors and bouncing in flames for critical alerts. Retry
 * scares it off; the close button fades the banner out.
 */
#include "errorgremlin.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
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

QColor ErrorGremlinWidget::severityColor(int severity)
{
    switch (severity) {
    case Info:
        return QColor(QStringLiteral("#4C9AFF"));
    case Warning:
        return QColor(QStringLiteral("#FFB020"));
    case Error:
        return QColor(QStringLiteral("#E63946"));
    default:
        return QColor(QStringLiteral("#C9184A"));
    }
}

ErrorGremlinWidget::ErrorGremlinWidget(QWidget *parent)
    : ErrorGremlinWidget(Options(), parent)
{
}

ErrorGremlinWidget::ErrorGremlinWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_severity(Severity(clampInt(options.severity, 0, 3))),
      m_title(options.title),
      m_message(options.message),
      m_retryText(options.retryText),
      m_closable(options.closable),
      m_panelColor(options.panelColor),
      m_textColor(options.textColor)
{
    if (options.retryClicked)
        connect(this, &ErrorGremlinWidget::retryClicked, this, options.retryClicked);
    if (options.dismissed)
        connect(this, &ErrorGremlinWidget::dismissed, this, options.dismissed);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ErrorGremlinWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal ErrorGremlinWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QRectF ErrorGremlinWidget::iconRect() const
{
    const QRectF panel = QRectF(rect()).adjusted(3, 3, -3, -3);
    const qreal size = std::min(panel.height() - 12.0, 96.0);
    return QRectF(panel.left() + 14.0, panel.center().y() - size / 2.0, size, size);
}

qreal ErrorGremlinWidget::fleeing() const
{
    const qreal since = now() - m_retryAt;
    if (since < 1.6)
        return clamp(since / 0.5, 0.0, 1.0);
    return 0.0;
}

// ---------- Public API ----------
void ErrorGremlinWidget::showAlert(int severity, const QString &title, const QString &message)
{
    if (severity >= 0)
        setSeverity(severity);
    if (!title.isEmpty())
        m_title = title;
    if (!message.isEmpty())
        m_message = message;
    m_closeAt = -1.0;
    m_retryAt = -10.0;
    show();
    update();
}

void ErrorGremlinWidget::dismiss()
{
    if (m_closeAt < 0)
        m_closeAt = now();
}

ErrorGremlinWidget::Severity ErrorGremlinWidget::getSeverity() const
{
    return m_severity;
}

void ErrorGremlinWidget::setSeverity(int severity)
{
    const Severity sev = Severity(clampInt(severity, 0, 3));
    if (sev != m_severity) {
        m_severity = sev;
        m_changedAt = now();
        m_sparks.clear();
        update();
    }
}

QString ErrorGremlinWidget::getTitle() const { return m_title; }
QString ErrorGremlinWidget::getMessage() const { return m_message; }
QString ErrorGremlinWidget::getRetryText() const { return m_retryText; }
bool ErrorGremlinWidget::isClosable() const { return m_closable; }
QColor ErrorGremlinWidget::getPanelColor() const { return m_panelColor; }
QColor ErrorGremlinWidget::getTextColor() const { return m_textColor; }

void ErrorGremlinWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void ErrorGremlinWidget::setMessage(const QString &message)
{
    m_message = message;
    update();
}

void ErrorGremlinWidget::setRetryText(const QString &retryText)
{
    m_retryText = retryText;
    update();
}

void ErrorGremlinWidget::setClosable(bool closable)
{
    m_closable = closable;
    update();
}

void ErrorGremlinWidget::setPanelColor(const QColor &color)
{
    m_panelColor = color;
    update();
}

void ErrorGremlinWidget::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize ErrorGremlinWidget::sizeHint() const
{
    return QSize(520, 110);
}

QSize ErrorGremlinWidget::minimumSizeHint() const
{
    return QSize(300, 90);
}

void ErrorGremlinWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void ErrorGremlinWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void ErrorGremlinWidget::mouseMoveEvent(QMouseEvent *e)
{
    const int hover = m_retryRect.contains(eventPos(e)) ? 1 : (m_closeRect.contains(eventPos(e)) ? -1 : 0);
    if (hover != m_hoverBtn) {
        m_hoverBtn = hover;
        setCursor(hover ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
    QWidget::mouseMoveEvent(e);
}

void ErrorGremlinWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_closeAt < 0) {
        if (m_retryRect.contains(eventPos(e))) {
            m_retryAt = now();
            emit retryClicked();
        } else if (m_closeRect.contains(eventPos(e))) {
            dismiss();
        }
    }
    QWidget::mouseReleaseEvent(e);
}

// ---------- Animation tick ----------
void ErrorGremlinWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    if (m_closeAt >= 0 && t - m_closeAt > 0.35) {
        m_closeAt = -1.0;
        hide();
        emit dismissed();
        return;
    }
    const QRectF icon = iconRect();
    const int sev = int(m_severity);
    if (fleeing() == 0.0) {
        if (sev == Error) {
            m_sparkAcc += dt * 22.0;
            while (m_sparkAcc > 1.0) {
                m_sparkAcc -= 1.0;
                const qreal a = uniform(-PI, 0);
                const qreal sp = uniform(50, 140);
                const qreal life = uniform(0.15, 0.35);
                m_sparks.append(Spark(icon.center().x() + icon.width() * 0.12, icon.top() + icon.height() * 0.62,
                                      std::cos(a) * sp, std::sin(a) * sp, life, 0));
            }
        } else if (sev == Critical) {
            m_sparkAcc += dt * 30.0;
            while (m_sparkAcc > 1.0) {
                m_sparkAcc -= 1.0;
                const qreal x = uniform(icon.left() + 6, icon.right() - 6);
                const qreal vy = -uniform(40, 80);
                const qreal life = uniform(0.4, 0.7);
                m_sparks.append(Spark(x, icon.bottom() - 4, 0, vy, life, 1));
            }
        }
    }
    QList<Spark> alive;
    for (Spark sp : std::as_const(m_sparks)) {
        if (sp.kind == 0)
            sp.vy += 400.0 * dt;
        sp.x += sp.vx * dt + (sp.kind ? std::sin(t * 9.0 + sp.y * 0.2) * 10.0 * dt : 0.0);
        sp.y += sp.vy * dt;
        sp.life -= dt;
        if (sp.life > 0)
            alive.append(sp);
    }
    m_sparks = alive;
    update();
}

// ---------- Painting ----------
void ErrorGremlinWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const int sev = int(m_severity);
    const QColor sc = severityColor(sev);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    if (m_closeAt >= 0)
        p.setOpacity(clamp(1.0 - (tNow - m_closeAt) / 0.35, 0.0, 1.0));
    const QRectF panel = QRectF(rect()).adjusted(3, 3, -3, -3);
    QPainterPath path;
    path.addRoundedRect(panel, 12, 12);
    p.fillPath(path, m_panelColor);
    QColor tint(sc);
    tint.setAlpha(22);
    p.fillPath(path, tint);
    QColor border(sc);
    if (sev == Critical)
        border.setAlpha(int(120 + 135 * (0.5 + 0.5 * std::sin(tNow * 10.0))));
    else
        border.setAlpha(110);
    p.setPen(QPen(border, sev < 3 ? 1.5 : 2.5));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    // Severity stripe.
    p.save();
    p.setClipPath(path);
    p.fillRect(QRectF(panel.left(), panel.top(), 6, panel.height()), sc);
    p.restore();
    // Gremlin scene (clipped to its icon box).
    const QRectF icon = iconRect();
    QPainterPath ip;
    ip.addRoundedRect(icon, 12, 12);
    QColor bg(sc);
    bg.setAlpha(30);
    p.fillPath(ip, bg);
    p.save();
    p.setClipPath(ip);
    drawScene(p, icon, sev, tNow);
    p.restore();
    // Buttons.
    const QColor tc = m_textColor;
    qreal right = panel.right() - 12.0;
    if (m_closable) {
        const QRectF cr(panel.right() - 30.0, panel.top() + 8.0, 22.0, 22.0);
        m_closeRect = cr;
        QColor cc(tc);
        cc.setAlpha(m_hoverBtn == -1 ? 200 : 120);
        p.setPen(QPen(cc, 1.8, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(cr.center() + QPointF(-5, -5), cr.center() + QPointF(5, 5));
        p.drawLine(cr.center() + QPointF(5, -5), cr.center() + QPointF(-5, 5));
        right = cr.left() - 6.0;
    } else {
        m_closeRect = QRectF();
    }
    QFont bf(font());
    bf.setBold(true);
    bf.setPointSizeF(9.0);
    const QFontMetricsF bfm(bf);
    qreal textRight;
    if (!m_retryText.isEmpty()) {
        const qreal adv = bfm.horizontalAdvance(m_retryText);
        const QRectF rr(right - adv - 28.0, panel.bottom() - 40.0, adv + 28.0, 30.0);
        m_retryRect = rr;
        p.setPen(QPen(sc, 1.5));
        p.setBrush(QColor(sc.red(), sc.green(), sc.blue(), m_hoverBtn == 1 ? 50 : 0));
        p.drawRoundedRect(rr, 15, 15);
        p.setFont(bf);
        p.setPen(m_panelColor.lightness() > 128 ? sc.darker(110) : sc.lighter(130));
        p.drawText(rr, Qt::AlignCenter, m_retryText);
        textRight = rr.left() - 10.0;
    } else {
        m_retryRect = QRectF();
        textRight = right;
    }
    // Text.
    const qreal x0 = icon.right() + 16.0;
    const qreal tw = std::max(40.0, textRight - x0);
    const bool retrying = 0.0 < fleeing() || tNow - m_retryAt < 1.6;
    QFont tf(font());
    tf.setBold(true);
    tf.setPointSizeF(11.0);
    p.setFont(tf);
    p.setPen(tc);
    const QString title = retrying ? tr("Retrying... (the gremlin fled)") : m_title;
    p.drawText(QRectF(x0, panel.top() + 12.0, tw, 20.0), Qt::AlignLeft | Qt::AlignVCenter, title);
    QFont mf(font());
    mf.setPointSizeF(9.0);
    p.setFont(mf);
    QColor sub(tc);
    sub.setAlpha(180);
    p.setPen(sub);
    p.drawText(QRectF(x0, panel.top() + 34.0, tw, panel.height() - 42.0), Qt::TextWordWrap | Qt::AlignTop, m_message);
    p.end();
}

void ErrorGremlinWidget::drawScene(QPainter &p, const QRectF &icon, int sev, qreal now)
{
    const qreal g = icon.width() * 0.5;
    qreal cx = icon.center().x();
    qreal cy = icon.center().y() + g * 0.15;
    const qreal flee = fleeing();
    // Critical: flames behind.
    if (sev == Critical) {
        for (const Spark &sp : std::as_const(m_sparks)) {
            if (sp.kind != 1)
                continue;
            const qreal k = clamp(sp.life / sp.maxLife, 0.0, 1.0);
            QColor fc(255, int(80 + 150 * k), 30);
            fc.setAlpha(int(200 * k));
            const qreal r = g * 0.12 * (0.5 + k);
            QPainterPath fl;
            fl.moveTo(sp.x, sp.y - r * 2.2);
            fl.cubicTo(sp.x + r, sp.y - r * 0.6, sp.x + r, sp.y + r, sp.x, sp.y + r);
            fl.cubicTo(sp.x - r, sp.y + r, sp.x - r, sp.y - r * 0.6, sp.x, sp.y - r * 2.2);
            p.fillPath(fl, fc);
        }
    }
    // Error: the cable being chewed.
    if (sev == Error) {
        QPainterPath cable;
        const qreal cy2 = icon.top() + icon.height() * 0.62;
        cable.moveTo(icon.left() - 4, cy2 + g * 0.1);
        cable.cubicTo(cx - g * 0.4, cy2 - g * 0.1, cx, cy2 + g * 0.1, icon.right() + 4, cy2 - g * 0.05);
        p.setPen(QPen(QColor(QStringLiteral("#2B2D36")), std::max(3.0, g * 0.12), Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        p.drawPath(cable);
    }
    if (flee > 0.0) {
        // Scurrying off to the left.
        cx -= flee * icon.width() * 1.1;
        cy += -std::abs(std::sin(now * 25.0)) * g * 0.08;
    } else if (sev == Info) {
        cy += g * 0.25;
    } else if (sev == Warning) {
        cy += g * 0.55 + std::sin(now * 2.0) * g * 0.05; // only peeking over the bottom edge
    } else if (sev == Critical) {
        cy -= std::abs(std::sin(now * 9.0)) * g * 0.25;
    }
    drawGremlin(p, QPointF(cx, cy), g, sev, flee, now);
    // Sparks on top.
    for (const Spark &sp : std::as_const(m_sparks)) {
        if (sp.kind != 0)
            continue;
        const qreal k = clamp(sp.life / sp.maxLife, 0.0, 1.0);
        QColor yc(QStringLiteral("#FFD23F"));
        yc.setAlpha(int(255 * k));
        p.setPen(QPen(yc, std::max(1.2, g * 0.04), Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(sp.x, sp.y), QPointF(sp.x - sp.vx * 0.03, sp.y - sp.vy * 0.03));
    }
    // Info: z.
    if (sev == Info && flee == 0.0) {
        QFont f(font());
        f.setBold(true);
        const qreal ph = pyMod(now * 0.5, 1.0);
        f.setPointSizeF(std::max(6.0, g * (0.18 + 0.12 * ph)));
        p.setFont(f);
        QColor zc = severityColor(0);
        zc.setAlpha(int(230 * std::sin(ph * PI)));
        p.setPen(zc);
        p.drawText(QPointF(cx + g * 0.45 + ph * g * 0.2, icon.top() + g * 0.55 - ph * g * 0.3), QStringLiteral("z"));
    }
    if (flee > 0.0 && flee < 1.0) {
        QFont f(font());
        f.setBold(true);
        f.setPointSizeF(std::max(7.0, g * 0.22));
        p.setFont(f);
        p.setPen(severityColor(sev));
        p.drawText(QPointF(icon.center().x() - g * 0.2, icon.top() + g * 0.45), tr("eep!"));
    }
}

void ErrorGremlinWidget::drawGremlin(QPainter &p, const QPointF &c, qreal g, int sev, qreal flee, qreal now)
{
    const QColor skin(QStringLiteral("#7FA37A"));
    const QColor ink(QStringLiteral("#1E2A1C"));
    const QPen outline(skin.darker(150), std::max(1.0, g * 0.04));
    const qreal breathe = sev == Info ? 1.0 + 0.04 * std::sin(now * 2.0) : 1.0;
    const bool fleeingNow = flee != 0.0;
    // Ears.
    const qreal droop = sev == Info ? 0.35 : 0.0;
    p.setPen(outline);
    p.setBrush(skin);
    for (qreal d : {-1.0, 1.0}) {
        QPainterPath ear;
        ear.moveTo(c.x() + d * g * 0.25, c.y() - g * 0.3);
        ear.lineTo(c.x() + d * g * 0.95, c.y() - g * (0.6 - droop));
        ear.lineTo(c.x() + d * g * 0.42, c.y() - g * 0.05);
        ear.closeSubpath();
        p.drawPath(ear);
        p.setBrush(QColor(QStringLiteral("#C98B8B")));
        QPainterPath inner;
        inner.moveTo(c.x() + d * g * 0.32, c.y() - g * 0.24);
        inner.lineTo(c.x() + d * g * 0.78, c.y() - g * (0.5 - droop));
        inner.lineTo(c.x() + d * g * 0.42, c.y() - g * 0.1);
        inner.closeSubpath();
        p.setPen(Qt::NoPen);
        p.drawPath(inner);
        p.setPen(outline);
        p.setBrush(skin);
    }
    // Head/body blob.
    p.drawEllipse(c, g * 0.48 * breathe, g * 0.42 * breathe);
    // Eyes.
    const qreal ex = g * 0.19;
    const qreal ey = c.y() - g * 0.08;
    if (sev == Info && flee == 0.0) {
        p.setPen(QPen(ink, std::max(1.0, g * 0.05), Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(c.x() + d * ex - g * 0.1, ey - g * 0.04, g * 0.2, g * 0.12), 180 * 16, 180 * 16);
    } else {
        const qreal dart = sev == Warning ? std::sin(now * 8.0) * g * 0.03 : 0.0;
        for (qreal d : {-1.0, 1.0}) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(QStringLiteral("#FFE066")));
            p.drawEllipse(QPointF(c.x() + d * ex, ey), g * 0.13, g * 0.12);
            p.setBrush(ink);
            p.drawEllipse(QPointF(c.x() + d * ex + dart - (fleeingNow ? g * 0.04 : 0.0), ey), g * 0.03, g * 0.09);
        }
        if (sev >= Error || fleeingNow) {
            // Mischievous brows.
            p.setPen(QPen(ink, std::max(1.0, g * 0.05), Qt::SolidLine, Qt::RoundCap));
            for (qreal d : {-1.0, 1.0})
                p.drawLine(QPointF(c.x() + d * (ex + g * 0.12), ey - g * 0.18), QPointF(c.x() + d * (ex - g * 0.1), ey - g * 0.12));
        }
    }
    // Mouth.
    const qreal my = c.y() + g * 0.16;
    if (sev == Info && flee == 0.0) {
        p.setPen(QPen(ink, std::max(1.0, g * 0.04)));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(c.x(), my), g * 0.04, g * 0.05);
    } else {
        const qreal open = (sev == Error && !fleeingNow)
                               ? g * (0.06 + 0.06 * std::abs(std::sin(now * 14.0)))
                               : g * ((sev == Critical || fleeingNow) ? 0.14 : 0.07);
        QPainterPath mouth;
        mouth.moveTo(c.x() - g * 0.24, my - g * 0.03);
        mouth.quadTo(c.x(), my + open * 2.2, c.x() + g * 0.24, my - g * 0.03);
        mouth.closeSubpath();
        p.setPen(QPen(ink, std::max(1.0, g * 0.03)));
        p.setBrush(QColor(QStringLiteral("#3B1020")));
        p.drawPath(mouth);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        for (int k = 0; k < 4; ++k) {
            const qreal x = c.x() - g * 0.17 + k * g * 0.11;
            QPainterPath tooth;
            tooth.moveTo(x - g * 0.04, my - g * 0.02);
            tooth.lineTo(x + g * 0.04, my - g * 0.02);
            tooth.lineTo(x, my + g * 0.06);
            tooth.closeSubpath();
            p.drawPath(tooth);
        }
    }
    // Warning: waving a wrench.
    if (sev == Warning && !fleeingNow) {
        p.save();
        p.translate(c.x() + g * 0.55, c.y() - g * 0.3);
        p.rotate(-30.0 + std::sin(now * 6.0) * 25.0);
        p.setPen(QPen(QColor(QStringLiteral("#8D99AE")), std::max(2.0, g * 0.07), Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(0, 0), QPointF(0, -g * 0.45));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#8D99AE")));
        QPainterPath jaw;
        jaw.addEllipse(QPointF(0, -g * 0.5), g * 0.11, g * 0.11);
        QPainterPath cut;
        cut.addRect(QRectF(-g * 0.04, -g * 0.65, g * 0.08, g * 0.12));
        p.drawPath(jaw.subtracted(cut));
        p.restore();
    }
}
