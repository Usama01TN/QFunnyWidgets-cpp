/**
 * HelpOwlWidget: a tiny "?" help icon shaped like a sleeping owl. Hover it (or click/press Space to pin it) and the
 * owl wakes up, swivels its head and shows a floating help bubble (HelpBubble); it dozes off again when you leave.
 */
#include "helpowl.h"

#include <QFontMetricsF>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace {

const qreal PI = 3.14159265358979323846;

inline qreal clamp(qreal x, qreal a, qreal b)
{
    return x < a ? a : (x > b ? b : x);
}

inline qreal easeOut(qreal t)
{
    t = clamp(t, 0.0, 1.0);
    return 1.0 - std::pow(1.0 - t, 3);
}

inline qreal pyMod(qreal a, qreal b)
{
    qreal r = std::fmod(a, b);
    if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
        r += b;
    return r;
}

} // namespace

// ======================================================================================================
// HelpBubble
// ======================================================================================================
HelpBubble::HelpBubble(QWidget *parent)
    : HelpBubble(Options(), parent)
{
}

HelpBubble::HelpBubble(QWidget *parent, const Options &options)
    : HelpBubble(options, parent)
{
}

HelpBubble::HelpBubble(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_title(options.title),
      m_text(options.text),
      m_accentColor(options.accentColor)
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    resizeToText();
}

// ---------- Helpers ----------
void HelpBubble::fonts(QFont &tf, QFont &bf) const
{
    tf = QFont(font());
    tf.setBold(true);
    tf.setPointSizeF(9.5);
    bf = QFont(font());
    bf.setPointSizeF(9.0);
}

void HelpBubble::resizeToText()
{
    QFont tf, bf;
    fonts(tf, bf);
    const qreal maxW = 240.0;
    const QRectF tr = !m_title.isEmpty()
                          ? QFontMetricsF(tf).boundingRect(QRectF(0, 0, maxW, 400), Qt::TextWordWrap, m_title)
                          : QRectF();
    const QRectF br = QFontMetricsF(bf).boundingRect(QRectF(0, 0, maxW, 1000), Qt::TextWordWrap, m_text);
    const qreal w = std::max(tr.width(), br.width()) + 28.0;
    const qreal h = tr.height() + (!m_title.isEmpty() ? 4.0 : 0.0) + br.height() + 22.0 + 10.0; // +10 tail
    resize(int(w) + 2, int(h) + 2);
}

// ---------- Public API ----------
QString HelpBubble::getTitle() const { return m_title; }
QString HelpBubble::getText() const { return m_text; }
QColor HelpBubble::getAccentColor() const { return m_accentColor; }
bool HelpBubble::isTailRight() const { return m_tailRight; }
qreal HelpBubble::getOpacity() const { return m_opacity; }

void HelpBubble::setTitle(const QString &title)
{
    m_title = title;
    resizeToText();
    update();
}

void HelpBubble::setText(const QString &text)
{
    m_text = text;
    resizeToText();
    update();
}

void HelpBubble::setAccentColor(const QColor &color)
{
    m_accentColor = color;
    update();
}

void HelpBubble::setTailRight(bool tailRight)
{
    m_tailRight = tailRight;
    update();
}

void HelpBubble::setOpacity(qreal opacity)
{
    m_opacity = clamp(opacity, 0.0, 1.0);
    update();
}

// ---------- Painting ----------
void HelpBubble::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    p.setOpacity(m_opacity);
    const QRectF rect(1, 1, width() - 2, height() - 12);
    QPainterPath path;
    path.addRoundedRect(rect, 10, 10);
    QPainterPath tail;
    if (m_tailRight) {
        tail.moveTo(rect.right() - 10, rect.bottom() - 2);
        tail.lineTo(rect.right() - 2, height() - 1.0);
        tail.lineTo(rect.right() - 24, rect.bottom() - 2);
    } else {
        tail.moveTo(rect.left() + 10, rect.bottom() - 2);
        tail.lineTo(rect.left() + 2, height() - 1.0);
        tail.lineTo(rect.left() + 24, rect.bottom() - 2);
    }
    tail.closeSubpath();
    path = path.united(tail);
    p.setPen(QPen(QColor(0, 0, 0, 50), 1.0));
    p.setBrush(QColor(QStringLiteral("#FFFDF7")));
    p.drawPath(path);
    p.fillRect(QRectF(rect.left() + 1, rect.top() + 10, 4, rect.height() - 20), m_accentColor);
    QFont tf, bf;
    fonts(tf, bf);
    qreal y = rect.top() + 10.0;
    if (!m_title.isEmpty()) {
        p.setFont(tf);
        p.setPen(QColor(QStringLiteral("#23232B")));
        const qreal th = QFontMetricsF(tf).boundingRect(QRectF(0, 0, rect.width() - 26, 400), Qt::TextWordWrap, m_title).height();
        p.drawText(QRectF(rect.left() + 14, y, rect.width() - 26, th), Qt::TextWordWrap, m_title);
        y += th + 4.0;
    }
    p.setFont(bf);
    p.setPen(QColor(QStringLiteral("#4A4A55")));
    p.drawText(QRectF(rect.left() + 14, y, rect.width() - 26, rect.bottom() - y - 6), Qt::TextWordWrap, m_text);
    p.end();
}

// ======================================================================================================
// HelpOwlWidget
// ======================================================================================================
HelpOwlWidget::HelpOwlWidget(QWidget *parent)
    : HelpOwlWidget(Options(), parent)
{
}

HelpOwlWidget::HelpOwlWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_owlColor(options.owlColor)
{
    if (options.shown)
        connect(this, &HelpOwlWidget::shown, this, options.shown);
    if (options.hidden)
        connect(this, &HelpOwlWidget::hidden, this, options.hidden);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::WhatsThisCursor);
    setFocusPolicy(Qt::StrongFocus);
    // Bubble window.
    // Parented to the owl so Qt destroys them together; the ToolTip flag keeps it a floating window.
    HelpBubble::Options bo;
    bo.title = options.title;
    bo.text = options.helpText;
    bo.accentColor = options.owlColor;
    m_bubble = new HelpBubble(this, bo);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HelpOwlWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal HelpOwlWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void HelpOwlWidget::setState(State state)
{
    if (state != m_state) {
        m_state = state;
        m_stateAt = now();
    }
}

bool HelpOwlWidget::wantOpen() const
{
    return m_hovered || m_pinned;
}

void HelpOwlWidget::placeBubble()
{
    HelpBubble *b = m_bubble;
    const QPoint g = mapToGlobal(QPoint(int(width() * 0.75), 0));
    QWidget *win = window();
    const int winRight = win->mapToGlobal(QPoint(win->width(), 0)).x();
    // Flip to the owl's left when the bubble would stick out of the window.
    const bool flip = g.x() + b->width() > winRight;
    b->setTailRight(flip);
    int x = flip ? (mapToGlobal(QPoint(int(width() * 0.25), 0)).x() - b->width()) : g.x();
    int y = g.y() - b->height() + 6;
    if (screen() != nullptr) {
        const QRect scr = screen()->availableGeometry();
        x = int(clamp(x, scr.left(), scr.right() - b->width()));
        y = int(clamp(y, scr.top(), scr.bottom() - b->height()));
    }
    b->move(x, y);
}

// ---------- Public API ----------
void HelpOwlWidget::showHelp()
{
    m_pinned = true;
    update();
}

void HelpOwlWidget::hideHelp()
{
    m_pinned = false;
    m_hovered = false;
}

bool HelpOwlWidget::isHelpVisible() const { return m_visible; }
HelpBubble *HelpOwlWidget::getBubble() const { return m_bubble; }
QString HelpOwlWidget::getHelpText() const { return m_bubble->getText(); }
QString HelpOwlWidget::getTitle() const { return m_bubble->getTitle(); }
QColor HelpOwlWidget::getOwlColor() const { return m_owlColor; }

void HelpOwlWidget::setHelpText(const QString &helpText)
{
    m_bubble->setText(helpText);
}

void HelpOwlWidget::setTitle(const QString &title)
{
    m_bubble->setTitle(title);
}

void HelpOwlWidget::setOwlColor(const QColor &color)
{
    m_owlColor = color;
    m_bubble->setAccentColor(color);
    update();
}

// ---------- Qt overrides ----------
QSize HelpOwlWidget::sizeHint() const
{
    return QSize(34, 40);
}

QSize HelpOwlWidget::minimumSizeHint() const
{
    return QSize(22, 26);
}

void HelpOwlWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void HelpOwlWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    if (m_bubble != nullptr)
        m_bubble->hide();
    QWidget::hideEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void HelpOwlWidget::enterEvent(QEnterEvent *e)
#else
void HelpOwlWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void HelpOwlWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void HelpOwlWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        m_pinned = !m_pinned;
    QWidget::mouseReleaseEvent(e);
}

void HelpOwlWidget::keyPressEvent(QKeyEvent *e)
{
    const int k = e->key();
    if (k == Qt::Key_Space || k == Qt::Key_Return || k == Qt::Key_Enter || k == Qt::Key_F1) {
        m_pinned = !m_pinned;
        return;
    }
    if (k == Qt::Key_Escape) {
        hideHelp();
        return;
    }
    QWidget::keyPressEvent(e);
}

void HelpOwlWidget::focusOutEvent(QFocusEvent *e)
{
    m_pinned = false;
    QWidget::focusOutEvent(e);
}

// ---------- Animation tick ----------
void HelpOwlWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal since = t - m_stateAt;
    const bool want = wantOpen();
    const State st = m_state;
    if (want && (st == SLEEP || st == DOZING))
        setState(WAKING);
    else if (st == WAKING && since >= WAKE_TIME)
        setState(AWAKE);
    else if (!want && (st == AWAKE || st == WAKING))
        setState(DOZING);
    else if (st == DOZING && since >= DOZE_TIME)
        setState(SLEEP);
    // Bubble fades in once the head has finished turning.
    const qreal target = (want && m_state == AWAKE) ? 1.0 : 0.0;
    m_alpha += (target - m_alpha) * std::min(1.0, dt * 12.0);
    HelpBubble *b = m_bubble;
    if (m_alpha > 0.02) {
        if (!b->isVisible()) {
            placeBubble();
            b->show();
        }
        b->setOpacity(m_alpha);
        if (!m_visible) {
            m_visible = true;
            emit shown();
        }
    } else if (b->isVisible()) {
        b->hide();
        if (m_visible) {
            m_visible = false;
            emit hidden();
        }
    }
    update();
}

// ---------- Painting ----------
void HelpOwlWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal w = width();
    const qreal h = height();
    const qreal s = std::min(w / 0.85, h);
    const QPointF c(w / 2.0, h / 2.0);
    const State st = m_state;
    const qreal since = tNow - m_stateAt;
    const QColor oc = m_owlColor;
    const QColor ink(QStringLiteral("#2B1D14"));
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const qreal breathe = 1.0 + (st == SLEEP ? 0.03 * std::sin(tNow * 2.0) : 0.0);
    // Feet.
    p.setPen(QPen(QColor(QStringLiteral("#E09F3E")), std::max(1.2, s * 0.05), Qt::SolidLine, Qt::RoundCap));
    for (qreal d : {-1.0, 1.0})
        p.drawLine(QPointF(c.x() + d * s * 0.1, c.y() + s * 0.4), QPointF(c.x() + d * s * 0.14, c.y() + s * 0.47));
    // Body.
    p.setPen(QPen(oc.darker(140), std::max(1.0, s * 0.03)));
    p.setBrush(oc);
    p.drawEllipse(QPointF(c.x(), c.y() + s * 0.12), s * 0.33 * breathe, s * 0.33);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#EAD7C3")));
    p.drawEllipse(QPointF(c.x(), c.y() + s * 0.18), s * 0.2, s * 0.23);
    // The "?" on the belly.
    QFont f(font());
    f.setBold(true);
    f.setPixelSize(std::max(7, int(s * 0.3)));
    p.setFont(f);
    p.setPen(oc.darker(130));
    p.drawText(QRectF(c.x() - s * 0.2, c.y() - s * 0.03, s * 0.4, s * 0.42), Qt::AlignCenter, QStringLiteral("?"));
    // Wings.
    p.setPen(QPen(oc.darker(140), std::max(1.0, s * 0.03)));
    p.setBrush(oc.darker(112));
    const qreal flap = st == WAKING ? std::sin(since * 30.0) * s * 0.03 : 0.0;
    for (qreal d : {-1.0, 1.0})
        p.drawEllipse(QPointF(c.x() + d * s * 0.3, c.y() + s * 0.15 - flap), s * 0.09, s * 0.2);
    // Head (swivels when waking).
    const QPointF hc(c.x(), c.y() - s * 0.2);
    p.save();
    p.translate(hc);
    if (st == WAKING)
        p.rotate(300.0 * (1.0 - easeOut(since / WAKE_TIME)));
    else if (st == SLEEP)
        p.rotate(-10.0);
    p.setPen(QPen(oc.darker(140), std::max(1.0, s * 0.03)));
    p.setBrush(oc);
    for (qreal d : {-1.0, 1.0}) {
        QPainterPath tuft;
        tuft.moveTo(d * s * 0.12, -s * 0.18);
        tuft.lineTo(d * s * 0.26, -s * 0.32);
        tuft.lineTo(d * s * 0.27, -s * 0.12);
        tuft.closeSubpath();
        p.drawPath(tuft);
    }
    p.drawEllipse(QPointF(0, 0), s * 0.3, s * 0.24);
    // Face disc + eyes.
    const qreal er = s * 0.1;
    for (qreal d : {-1.0, 1.0}) {
        const QPointF ec(d * s * 0.13, -s * 0.01);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#EAD7C3")));
        p.drawEllipse(ec, er * 1.25, er * 1.25);
        if (st == SLEEP || st == DOZING) {
            p.setPen(QPen(ink, std::max(1.0, s * 0.035), Qt::SolidLine, Qt::RoundCap));
            p.setBrush(Qt::NoBrush);
            p.drawArc(QRectF(ec.x() - er * 0.8, ec.y() - er * 0.4, er * 1.6, er * 1.0), 180 * 16, 180 * 16);
        } else {
            p.setBrush(QColor(QStringLiteral("#FFD23F")));
            p.drawEllipse(ec, er, er);
            p.setBrush(ink);
            p.drawEllipse(QPointF(ec.x() + er * 0.25, ec.y() - er * 0.2), er * 0.5, er * 0.5);
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(ec.x() + er * 0.1, ec.y() - er * 0.4), er * 0.18, er * 0.18);
        }
    }
    // Beak.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#E09F3E")));
    QPainterPath beak;
    beak.moveTo(-s * 0.04, s * 0.06);
    beak.lineTo(s * 0.04, s * 0.06);
    beak.lineTo(0, s * 0.14);
    beak.closeSubpath();
    p.drawPath(beak);
    p.restore();
    // Sleepy z (kept inside the icon).
    if (st == SLEEP) {
        const qreal ph = pyMod(tNow * 0.5, 1.0);
        QFont zf(font());
        zf.setBold(true);
        zf.setPixelSize(std::max(6, int(s * (0.14 + 0.08 * ph))));
        p.setFont(zf);
        QColor zc(palette().text().color());
        zc.setAlpha(int(200 * std::sin(ph * PI)));
        p.setPen(zc);
        p.drawText(QPointF(std::min(w - s * 0.2, c.x() + s * 0.28 + ph * s * 0.08),
                           std::max(s * 0.2, c.y() - s * 0.38 - ph * s * 0.1)),
                   QStringLiteral("z"));
    }
    if (hasFocus()) {
        QColor fc(palette().text().color());
        fc.setAlpha(60);
        p.setPen(QPen(fc, 1.0, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 6, 6);
    }
    p.end();
}
