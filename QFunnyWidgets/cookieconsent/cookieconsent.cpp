/**
 * CookieConsentWidget: a cookie banner starring an emotionally invested cookie. It looks at you with puppy eyes,
 * gets excited when you hover Accept, tears up when you hover Decline (which shyly shrinks), happily gets eaten
 * bite by bite when accepted and sobs when declined.
 */
#include "cookieconsent.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

const qreal PI = 3.14159265358979323846;

const qreal CHIPS[][2] = {{-0.35, -0.45}, {0.3, -0.5}, {0.55, 0.05}, {-0.55, 0.15}, {0.1, 0.6}, {-0.25, 0.55}, {0.45, 0.45}};
const qreal BITES[][3] = {{-25.0, 0.42, 0.25}, {20.0, 0.38, 0.55}, {-65.0, 0.34, 0.85}}; // angle deg, radius ratio, time s
const int BITE_COUNT = 3;

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

inline qreal radians(qreal deg)
{
    return deg * PI / 180.0;
}

inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

} // namespace

CookieConsentWidget::CookieConsentWidget(QWidget *parent)
    : CookieConsentWidget(Options(), parent)
{
}

CookieConsentWidget::CookieConsentWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_title(options.title),
      m_message(options.message),
      m_acceptText(options.acceptText),
      m_declineText(options.declineText),
      m_acceptedMessage(options.acceptedMessage),
      m_declinedMessage(options.declinedMessage),
      m_accentColor(options.accentColor),
      m_panelColor(options.panelColor),
      m_textColor(options.textColor),
      m_cookieColor(options.cookieColor),
      m_autoReset(std::max(0, options.autoReset))
{
    if (options.accepted)
        connect(this, &CookieConsentWidget::accepted, this, options.accepted);
    if (options.declined)
        connect(this, &CookieConsentWidget::declined, this, options.declined);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CookieConsentWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal CookieConsentWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void CookieConsentWidget::choose(int choice)
{
    if (m_choice != 0)
        return;
    m_choice = choice;
    m_choiceAt = now();
    m_bitesDone = 0;
    m_hoverBtn = 0;
    setCursor(Qt::ArrowCursor);
    if (choice > 0)
        emit accepted();
    else
        emit declined();
    update();
}

// ---------- Public API ----------
void CookieConsentWidget::accept()
{
    choose(1);
}

void CookieConsentWidget::decline()
{
    choose(-1);
}

void CookieConsentWidget::reset()
{
    m_choice = 0;
    m_bitesDone = 0;
    m_crumbs.clear();
    update();
}

int CookieConsentWidget::getChoice() const { return m_choice; }
QString CookieConsentWidget::getTitle() const { return m_title; }
QString CookieConsentWidget::getMessage() const { return m_message; }
QString CookieConsentWidget::getAcceptText() const { return m_acceptText; }
QString CookieConsentWidget::getDeclineText() const { return m_declineText; }
QString CookieConsentWidget::getAcceptedMessage() const { return m_acceptedMessage; }
QString CookieConsentWidget::getDeclinedMessage() const { return m_declinedMessage; }
QColor CookieConsentWidget::getAccentColor() const { return m_accentColor; }
QColor CookieConsentWidget::getPanelColor() const { return m_panelColor; }
QColor CookieConsentWidget::getTextColor() const { return m_textColor; }
QColor CookieConsentWidget::getCookieColor() const { return m_cookieColor; }
int CookieConsentWidget::getAutoReset() const { return m_autoReset; }

void CookieConsentWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void CookieConsentWidget::setMessage(const QString &message)
{
    m_message = message;
    update();
}

void CookieConsentWidget::setAcceptText(const QString &acceptText)
{
    m_acceptText = acceptText;
    update();
}

void CookieConsentWidget::setDeclineText(const QString &declineText)
{
    m_declineText = declineText;
    update();
}

void CookieConsentWidget::setAcceptedMessage(const QString &acceptedMessage)
{
    m_acceptedMessage = acceptedMessage;
    update();
}

void CookieConsentWidget::setDeclinedMessage(const QString &declinedMessage)
{
    m_declinedMessage = declinedMessage;
    update();
}

void CookieConsentWidget::setAccentColor(const QColor &color)
{
    if (m_accentColor != color) {
        m_accentColor = color;
        update();
    }
}

void CookieConsentWidget::setPanelColor(const QColor &color)
{
    if (m_panelColor != color) {
        m_panelColor = color;
        update();
    }
}

void CookieConsentWidget::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        update();
    }
}

void CookieConsentWidget::setCookieColor(const QColor &color)
{
    if (m_cookieColor != color) {
        m_cookieColor = color;
        update();
    }
}

void CookieConsentWidget::setAutoReset(int autoReset)
{
    m_autoReset = std::max(0, autoReset);
}

// ---------- Qt overrides ----------
QSize CookieConsentWidget::sizeHint() const
{
    return QSize(520, 160);
}

QSize CookieConsentWidget::minimumSizeHint() const
{
    return QSize(320, 120);
}

void CookieConsentWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void CookieConsentWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void CookieConsentWidget::mouseMoveEvent(QMouseEvent *e)
{
    int hover = 0;
    if (m_choice == 0) {
        if (m_acceptRect.contains(eventPos(e)))
            hover = 1;
        else if (m_declineRect.contains(eventPos(e)))
            hover = -1;
    }
    if (hover != m_hoverBtn) {
        m_hoverBtn = hover;
        setCursor(hover ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
    QWidget::mouseMoveEvent(e);
}

void CookieConsentWidget::leaveEvent(QEvent *e)
{
    m_hoverBtn = 0;
    QWidget::leaveEvent(e);
}

void CookieConsentWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_choice == 0) {
        if (m_acceptRect.contains(eventPos(e)))
            accept();
        else if (m_declineRect.contains(eventPos(e)))
            decline();
    }
    QWidget::mouseReleaseEvent(e);
}

// ---------- Animation tick ----------
void CookieConsentWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal since = t - m_choiceAt;
    const QPointF c = m_cookieCentre;
    const qreal R = m_cookieR;
    if (m_choice == 1) {
        while (m_bitesDone < BITE_COUNT && since >= BITES[m_bitesDone][2]) {
            const qreal ang = BITES[m_bitesDone][0];
            const qreal bx = c.x() + std::cos(radians(ang)) * R;
            const qreal by = c.y() + std::sin(radians(ang)) * R;
            for (int i = 0; i < 14; ++i) {
                const qreal a = uniform(-PI, PI);
                const qreal sp = uniform(40, 140);
                Crumb cr;
                cr.x = bx;
                cr.y = by;
                cr.vx = std::cos(a) * sp + 40;
                cr.vy = std::sin(a) * sp - 80;
                cr.size = uniform(1.5, 3.5) * R / 40.0;
                cr.life = uniform(0.6, 1.1);
                m_crumbs.append(cr);
            }
            m_bitesDone += 1;
        }
    }
    if (m_choice != 0 && m_autoReset && since > m_autoReset / 1000.0)
        reset();
    if (t >= m_blinkAt + 0.14)
        m_blinkAt = t + uniform(2.0, 4.5);
    QList<Crumb> alive;
    for (Crumb cr : std::as_const(m_crumbs)) {
        cr.vy += 500.0 * dt;
        cr.x += cr.vx * dt;
        cr.y += cr.vy * dt;
        cr.life -= dt;
        if (cr.life > 0)
            alive.append(cr);
    }
    m_crumbs = alive;
    update();
}

// ---------- Painting ----------
void CookieConsentWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const QRectF panel = QRectF(rect()).adjusted(3, 3, -3, -3);
    if (m_panelColor.alpha() > 0) {
        p.setPen(QPen(QColor(0, 0, 0, 30), 1.0));
        p.setBrush(m_panelColor);
        p.drawRoundedRect(panel, 16, 16);
    }
    const qreal R = clamp(std::min(panel.height() * 0.33, panel.width() * 0.13), 14.0, 70.0);
    const QPointF c(panel.left() + 22.0 + R, panel.center().y());
    m_cookieCentre = c;
    m_cookieR = R;
    drawCookie(p, c, R, tNow);
    // Crumbs.
    p.setPen(Qt::NoPen);
    p.setBrush(m_cookieColor.darker(115));
    for (const Crumb &cr : std::as_const(m_crumbs))
        p.drawEllipse(QPointF(cr.x, cr.y), cr.size, cr.size * 0.8);
    // Text block.
    const qreal x0 = c.x() + R + 24.0;
    const qreal tw = panel.right() - 20.0 - x0;
    if (tw < 60) {
        p.end();
        return;
    }
    QFont tf(font());
    tf.setBold(true);
    tf.setPointSizeF(12.5);
    QFont mf(font());
    mf.setPointSizeF(9.5);
    const QFontMetricsF tfm(tf);
    const QFontMetricsF mfm(mf);
    QString message;
    if (m_choice == 0)
        message = m_message;
    else
        message = m_choice > 0 ? m_acceptedMessage : m_declinedMessage;
    const QRectF mr = mfm.boundingRect(QRectF(0, 0, tw, 1000), Qt::TextWordWrap, message);
    const qreal btnH = m_choice == 0 ? 34.0 : 0.0;
    const qreal blockH = tfm.height() + 6.0 + mr.height() + (btnH ? 14.0 + btnH : 0.0);
    qreal y = panel.center().y() - blockH / 2.0;
    p.setFont(tf);
    p.setPen(m_textColor);
    p.drawText(QRectF(x0, y, tw, tfm.height()), Qt::AlignLeft | Qt::AlignVCenter, m_title);
    y += tfm.height() + 6.0;
    p.setFont(mf);
    QColor sub(m_textColor);
    sub.setAlpha(190);
    p.setPen(sub);
    p.drawText(QRectF(x0, y, tw, mr.height()), Qt::TextWordWrap, message);
    y += mr.height() + 14.0;
    if (m_choice == 0) {
        drawButtons(p, x0, y, btnH, tNow);
    } else {
        m_acceptRect = QRectF();
        m_declineRect = QRectF();
    }
    p.end();
}

void CookieConsentWidget::drawButtons(QPainter &p, qreal x0, qreal y, qreal h, qreal now)
{
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(9.5);
    const QFontMetricsF fm(f);
    const qreal aw = fm.horizontalAdvance(m_acceptText) + 36.0;
    const qreal dw = fm.horizontalAdvance(m_declineText) + 30.0;
    m_acceptRect = QRectF(x0, y, aw, h);
    // Decline button shrinks (a little) while you look at it. Dark pattern? Comedy pattern.
    const qreal shy = m_hoverBtn == -1 ? 0.92 : 1.0;
    m_declineRect = QRectF(x0 + aw + 10.0, y + h * (1.0 - shy) / 2.0, dw * shy, h * shy);
    p.setFont(f);
    const QColor ac = m_hoverBtn == 1 ? m_accentColor.lighter(112) : m_accentColor;
    QRectF r = m_acceptRect;
    if (m_hoverBtn == 1) {
        const qreal pulse = 1.0 + 0.04 * std::sin(now * 12.0);
        const QPointF cx = r.center();
        r = QRectF(cx.x() - r.width() * pulse / 2.0, cx.y() - r.height() * pulse / 2.0, r.width() * pulse, r.height() * pulse);
    }
    p.setPen(Qt::NoPen);
    p.setBrush(ac);
    p.drawRoundedRect(r, h / 2.0, h / 2.0);
    p.setPen(QColor(QStringLiteral("#FFFFFF")));
    p.drawText(r, Qt::AlignCenter, m_acceptText);
    const QRectF d = m_declineRect;
    QColor border(m_textColor);
    border.setAlpha(m_hoverBtn == -1 ? 110 : 70);
    p.setPen(QPen(border, 1.3));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(d, d.height() / 2.0, d.height() / 2.0);
    QColor tc(m_textColor);
    tc.setAlpha(200);
    p.setPen(tc);
    p.drawText(d, Qt::AlignCenter, m_declineText);
}

void CookieConsentWidget::drawCookie(QPainter &p, const QPointF &c, qreal R, qreal now)
{
    const int choice = m_choice;
    const int hover = m_hoverBtn;
    p.save();
    qreal bob = std::sin(now * (hover == 1 ? 7.0 : 2.0)) * R * (hover == 1 ? 0.06 : 0.03);
    if (choice == -1)
        bob = std::sin(now * 14.0) * R * 0.03; // sobbing
    p.translate(c.x(), c.y() + bob);
    const QColor cc = m_cookieColor;
    // Cookie shape with bites taken out.
    QPainterPath shape;
    shape.addEllipse(QPointF(0, 0), R, R);
    if (choice == 1) {
        for (int k = 0; k < m_bitesDone; ++k) {
            const qreal ang = BITES[k][0];
            const qreal rr = BITES[k][1];
            QPainterPath bite;
            bite.addEllipse(QPointF(std::cos(radians(ang)) * R * 1.05, std::sin(radians(ang)) * R * 1.05), R * rr, R * rr);
            shape = shape.subtracted(bite);
        }
    }
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 40));
    p.drawPath(shape.translated(0, R * 0.1));
    QRadialGradient grad(QPointF(-R * 0.3, -R * 0.35), R * 1.6);
    grad.setColorAt(0.0, cc.lighter(118));
    grad.setColorAt(0.8, cc);
    grad.setColorAt(1.0, cc.darker(125));
    p.setBrush(grad);
    p.setPen(QPen(cc.darker(145), std::max(1.2, R * 0.04)));
    p.drawPath(shape);
    // Chocolate chips (only where the cookie still exists).
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#5B3A1E")));
    for (const auto &chip : CHIPS) {
        const qreal cx = chip[0];
        const qreal cy = chip[1];
        const QPointF pt(cx * R, cy * R);
        if (shape.contains(pt) && !(std::abs(cx) < 0.5 && -0.3 < cy && cy < 0.45))
            p.drawEllipse(pt, R * 0.09, R * 0.075);
    }
    // Face.
    const QColor ink(QStringLiteral("#2E1E10"));
    const qreal ex = R * 0.25;
    const qreal ey = -R * 0.08;
    const qreal er = R * 0.14;
    const QPen pen(ink, std::max(1.2, R * 0.06), Qt::SolidLine, Qt::RoundCap);
    const bool blinking = m_blinkAt <= now && now < m_blinkAt + 0.14;
    if (choice == 1) {
        // Blissful ^ ^ eyes and a blush.
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(d * ex - er, ey - er * 0.6, er * 2.0, er * 1.4), 20 * 16, 140 * 16);
        QColor bl(QStringLiteral("#FF8FA3"));
        bl.setAlpha(150);
        p.setPen(Qt::NoPen);
        p.setBrush(bl);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(d * R * 0.45, R * 0.18), R * 0.12, R * 0.07);
        p.setPen(pen);
        p.setBrush(QColor(QStringLiteral("#7A2E1E")));
        QPainterPath mouth;
        mouth.moveTo(-R * 0.18, R * 0.22);
        mouth.quadTo(0, R * 0.5, R * 0.18, R * 0.22);
        mouth.closeSubpath();
        p.drawPath(mouth);
    } else if (choice == -1) {
        // Squeezed shut eyes, tear streams, wailing mouth.
        p.setPen(pen);
        for (qreal d : {-1.0, 1.0}) {
            p.drawLine(QPointF(d * ex - er, ey - er * 0.5), QPointF(d * ex + er * 0.2 * d, ey));
            p.drawLine(QPointF(d * ex + er * 0.2 * d, ey), QPointF(d * ex - er, ey + er * 0.4));
        }
        QColor tear(QStringLiteral("#5AB9FF"));
        tear.setAlpha(210);
        for (qreal d : {-1.0, 1.0}) {
            QPainterPath stream;
            stream.moveTo(d * ex, ey + er * 0.5);
            for (int k = 1; k < 7; ++k) {
                const qreal t = k / 6.0;
                stream.lineTo(d * (ex + R * 0.08 * std::sin(now * 10.0 + k)), ey + er * 0.5 + t * R * 0.8);
            }
            const QPen tp(tear, std::max(2.0, R * 0.1), Qt::SolidLine, Qt::RoundCap);
            p.setPen(tp);
            p.setBrush(Qt::NoBrush);
            p.drawPath(stream);
        }
        p.setPen(pen);
        p.setBrush(QColor(QStringLiteral("#7A2E1E")));
        const qreal o = 1.0 + 0.2 * std::sin(now * 16.0);
        p.drawEllipse(QPointF(0, R * 0.36), R * 0.16, R * 0.12 * o);
    } else {
        // Hopeful puppy eyes.
        for (qreal d : {-1.0, 1.0}) {
            if (blinking) {
                p.setPen(pen);
                p.setBrush(Qt::NoBrush);
                p.drawArc(QRectF(d * ex - er, ey - er * 0.5, er * 2.0, er), 180 * 16, 180 * 16);
                continue;
            }
            const qreal big = er * (hover == 1 ? 1.15 : 1.0);
            p.setPen(Qt::NoPen);
            p.setBrush(ink);
            p.drawEllipse(QPointF(d * ex, ey), big, big * 1.1);
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(d * ex - big * 0.3, ey - big * 0.35), big * 0.35, big * 0.35);
            p.drawEllipse(QPointF(d * ex + big * 0.3, ey + big * 0.35), big * 0.15, big * 0.15);
            if (hover == -1) {
                // Welling tear.
                const qreal grow = clamp(pyMod(now * 1.0, 1.5), 0.0, 1.0);
                QColor tc(QStringLiteral("#5AB9FF"));
                tc.setAlpha(200);
                p.setBrush(tc);
                p.drawEllipse(QPointF(d * ex + d * big * 0.4, ey + big * 1.1 + grow * R * 0.15), big * 0.3, big * 0.38);
            }
        }
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        if (hover == -1) {
            // Quivering lip + worried brows.
            QPainterPath mouth;
            mouth.moveTo(-R * 0.16, R * 0.33);
            for (int k = 1; k < 7; ++k)
                mouth.lineTo(-R * 0.16 + k * R * 0.32 / 6.0, R * 0.3 + ((k % 2) ? R * 0.025 : -R * 0.025) * std::sin(now * 40.0));
            p.drawPath(mouth);
            for (qreal d : {-1.0, 1.0})
                p.drawLine(QPointF(d * (ex + er), ey - er * 1.5), QPointF(d * (ex - er * 0.8), ey - er * 2.1));
        } else if (hover == 1) {
            p.setBrush(QColor(QStringLiteral("#7A2E1E")));
            QPainterPath mouth;
            mouth.moveTo(-R * 0.2, R * 0.2);
            mouth.quadTo(0, R * 0.52, R * 0.2, R * 0.2);
            mouth.closeSubpath();
            p.drawPath(mouth);
        } else {
            p.drawArc(QRectF(-R * 0.14, R * 0.12, R * 0.28, R * 0.2), 200 * 16, 140 * 16);
        }
    }
    p.restore();
}
