/**
 * SleepyCatSectionWidget: a collapsible section whose header bar (CatHeaderWidget) has a cat napping on it. The
 * cat sleeps while the section is collapsed (peeking when you hover), jumps up startled when it is expanded and
 * then sits watching the content, and yawns before going back to sleep when it collapses.
 */
#include "catsection.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
#include <QVBoxLayout>
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

// ======================================================================================================
// CatHeaderWidget
// ======================================================================================================
CatHeaderWidget::CatHeaderWidget(QWidget *parent)
    : CatHeaderWidget(Options(), parent)
{
}

CatHeaderWidget::CatHeaderWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_title(options.title),
      m_expanded(options.expanded),
      m_catColor(options.catColor),
      m_barColor(options.barColor),
      m_textColor(options.textColor)
{
    if (options.clicked)
        connect(this, &CatHeaderWidget::clicked, this, options.clicked);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setFixedHeight(int(CAT_ROOM + BAR_HEIGHT));
    // State.
    m_cat = m_expanded ? SITTING : SLEEPING;
    m_chevron = m_expanded ? 90.0 : 0.0;
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CatHeaderWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal CatHeaderWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

// ---------- Public API ----------
bool CatHeaderWidget::isExpanded() const
{
    return m_expanded;
}

void CatHeaderWidget::setExpanded(bool expanded)
{
    if (expanded != m_expanded) {
        m_expanded = expanded;
        m_cat = expanded ? STARTLED : YAWNING;
        m_catAt = now();
        update();
    }
}

QString CatHeaderWidget::getTitle() const { return m_title; }
QColor CatHeaderWidget::getCatColor() const { return m_catColor; }
QColor CatHeaderWidget::getBarColor() const { return m_barColor; }
QColor CatHeaderWidget::getTextColor() const { return m_textColor; }

void CatHeaderWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void CatHeaderWidget::setCatColor(const QColor &color)
{
    m_catColor = color;
    update();
}

void CatHeaderWidget::setBarColor(const QColor &color)
{
    m_barColor = color;
    update();
}

void CatHeaderWidget::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize CatHeaderWidget::sizeHint() const
{
    return QSize(360, int(CAT_ROOM + BAR_HEIGHT));
}

void CatHeaderWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void CatHeaderWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void CatHeaderWidget::enterEvent(QEnterEvent *e)
#else
void CatHeaderWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void CatHeaderWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void CatHeaderWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(eventPos(e)))
        emit clicked();
    QWidget::mouseReleaseEvent(e);
}

void CatHeaderWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        emit clicked();
        return;
    }
    QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void CatHeaderWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal since = t - m_catAt;
    if (m_cat == STARTLED && since > STARTLE_TIME)
        m_cat = SITTING;
    else if (m_cat == YAWNING && since > YAWN_TIME)
        m_cat = SLEEPING;
    const qreal target = m_expanded ? 90.0 : 0.0;
    m_chevron += (target - m_chevron) * std::min(1.0, dt * 12.0);
    if (t >= m_blinkAt + 0.14)
        m_blinkAt = t + uniform(2.0, 5.0);
    update();
}

// ---------- Painting ----------
void CatHeaderWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal w = width();
    const QRectF bar(1.0, CAT_ROOM, w - 2.0, height() - CAT_ROOM - 1.0);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const QColor bc = m_hovered ? m_barColor.lighter(108) : m_barColor;
    p.setPen(QPen(QColor(0, 0, 0, 30), 1.0));
    p.setBrush(bc);
    p.drawRoundedRect(bar, 12, 12);
    if (hasFocus()) {
        QColor fc(m_textColor);
        fc.setAlpha(60);
        p.setPen(QPen(fc, 1.0, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(bar.adjusted(3, 3, -3, -3), 9, 9);
    }
    // Chevron.
    const QPointF cc(bar.left() + 22.0, bar.center().y());
    p.save();
    p.translate(cc);
    p.rotate(m_chevron);
    p.setPen(QPen(m_textColor, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(QPointF(-3, -6), QPointF(3, 0));
    p.drawLine(QPointF(3, 0), QPointF(-3, 6));
    p.restore();
    // Title.
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(10.5);
    p.setFont(f);
    p.setPen(m_textColor);
    p.drawText(QRectF(bar.left() + 42.0, bar.top(), bar.width() - 140.0, bar.height()), Qt::AlignLeft | Qt::AlignVCenter,
               m_title);
    // Cat on the right end of the bar.
    drawCat(p, QPointF(bar.right() - 52.0, bar.top() + 4.0), tNow);
    p.end();
}

void CatHeaderWidget::drawCat(QPainter &p, const QPointF &base, qreal now)
{
    const CatState cat = m_cat;
    const qreal since = now - m_catAt;
    const QColor col = m_catColor;
    const QColor ink(QStringLiteral("#3A2416"));
    const QPen outline(col.darker(150), 1.2);
    const QPen pen(ink, 1.6, Qt::SolidLine, Qt::RoundCap);
    p.save();
    p.translate(base);
    if (cat == STARTLED) {
        const qreal t = clamp(since / STARTLE_TIME, 0.0, 1.0);
        p.translate(0, -std::sin(t * PI) * 8.0);
    }
    const bool lying = cat == SLEEPING || cat == YAWNING;
    QPointF hc;
    if (lying) {
        // Loaf: wide body, head on the left, tail wrapped in front.
        const qreal breathe = 1.0 + 0.04 * std::sin(now * 2.2);
        p.setPen(outline);
        p.setBrush(col);
        p.drawEllipse(QPointF(6.0, -9.0), 24.0, 11.0 * breathe);
        p.setBrush(col.darker(112));
        for (int k = 0; k < 3; ++k)
            p.drawRect(QRectF(4.0 + k * 7.0, -19.0 * breathe, 3.0, 6.0));
        p.setBrush(col);
        QPainterPath tail;
        tail.moveTo(28.0, -4.0);
        tail.quadTo(20.0, 3.0, -4.0, 0.0);
        p.setPen(QPen(col.darker(120), 5.0, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        p.drawPath(tail);
        hc = QPointF(-16.0, -11.0);
    } else {
        // Sitting upright, tail swishing.
        const qreal swish = std::sin(now * 3.0) * 6.0;
        p.setPen(QPen(col.darker(120), 5.0, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        QPainterPath tail;
        tail.moveTo(10.0, -4.0);
        tail.cubicTo(26.0, -6.0, 24.0 + swish, -22.0, 18.0 + swish, -30.0);
        p.drawPath(tail);
        p.setPen(outline);
        p.setBrush(col);
        p.drawEllipse(QPointF(0.0, -12.0), 13.0, 13.0);
        p.setBrush(col.lighter(120));
        p.drawEllipse(QPointF(0.0, -9.0), 7.0, 8.0);
        hc = QPointF(0.0, -30.0);
    }
    // Head + ears.
    p.setPen(outline);
    p.setBrush(col);
    const qreal twitch = (m_hovered && lying) ? std::sin(now * 30.0) * 1.5 : 0.0;
    for (qreal d : {-1.0, 1.0}) {
        QPainterPath ear;
        ear.moveTo(hc.x() + d * 4.0, hc.y() - 7.0);
        ear.lineTo(hc.x() + d * 9.5, hc.y() - 15.0 + (d > 0 ? twitch : 0.0));
        ear.lineTo(hc.x() + d * 10.0, hc.y() - 4.0);
        ear.closeSubpath();
        p.drawPath(ear);
    }
    p.drawEllipse(hc, 11.0, 9.5);
    p.setPen(QPen(col.darker(125), 1.2));
    for (qreal dx : {-3.0, 0.0, 3.0})
        p.drawLine(QPointF(hc.x() + dx, hc.y() - 9.0), QPointF(hc.x() + dx, hc.y() - 5.5));
    // Face.
    const qreal ey = hc.y() - 1.0;
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    const bool blinking = m_blinkAt <= now && now < m_blinkAt + 0.14;
    if (cat == SLEEPING) {
        const bool peek = m_hovered && pyMod(now, 3.0) < 1.6;
        for (qreal d : {-1.0, 1.0}) {
            if (peek && d > 0) {
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(QStringLiteral("#9BDB4D")));
                p.drawEllipse(QPointF(hc.x() + d * 4.5, ey), 2.2, 1.4);
                p.setBrush(ink);
                p.drawEllipse(QPointF(hc.x() + d * 4.5, ey), 0.7, 1.3);
                p.setPen(pen);
                p.setBrush(Qt::NoBrush);
            } else {
                p.drawArc(QRectF(hc.x() + d * 4.5 - 2.5, ey - 1.0, 5.0, 3.0), 180 * 16, 180 * 16);
            }
        }
    } else if (cat == YAWNING) {
        for (qreal d : {-1.0, 1.0})
            p.drawLine(QPointF(hc.x() + d * 4.5 - 2.5, ey - 1.0), QPointF(hc.x() + d * 4.5 + 2.5, ey + 0.5));
    } else if (cat == STARTLED) {
        for (qreal d : {-1.0, 1.0}) {
            p.setPen(QPen(ink, 1.0));
            p.setBrush(QColor(QStringLiteral("#FFFFFF")));
            p.drawEllipse(QPointF(hc.x() + d * 4.5, ey), 3.2, 3.6);
            p.setPen(Qt::NoPen);
            p.setBrush(ink);
            p.drawEllipse(QPointF(hc.x() + d * 4.5, ey), 1.0, 1.0);
        }
    } else {
        for (qreal d : {-1.0, 1.0}) {
            if (blinking) {
                p.drawLine(QPointF(hc.x() + d * 4.5 - 2.2, ey), QPointF(hc.x() + d * 4.5 + 2.2, ey));
                continue;
            }
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(QStringLiteral("#9BDB4D")));
            p.drawEllipse(QPointF(hc.x() + d * 4.5, ey), 2.6, 2.8);
            p.setBrush(ink);
            p.drawEllipse(QPointF(hc.x() + d * 4.5, ey + 0.8), 0.9, 2.0); // watching the content below
            p.setPen(pen);
        }
    }
    // Nose / mouth.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#E58C8A")));
    p.drawEllipse(QPointF(hc.x(), hc.y() + 2.5), 1.4, 1.0);
    if (cat == YAWNING || cat == STARTLED) {
        const qreal open = cat == YAWNING ? std::sin(clamp(since / YAWN_TIME, 0.0, 1.0) * PI) * 3.5 : 2.0;
        p.setBrush(QColor(QStringLiteral("#7A2E2E")));
        p.drawEllipse(QPointF(hc.x(), hc.y() + 5.5), 2.2, std::max(0.6, open));
    } else {
        p.setPen(QPen(ink, 1.0, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(hc.x() - 3.0, hc.y() + 2.5, 3.0, 2.5), 180 * 16, 180 * 16);
        p.drawArc(QRectF(hc.x(), hc.y() + 2.5, 3.0, 2.5), 180 * 16, 180 * 16);
    }
    // Whiskers.
    QColor wc(ink);
    wc.setAlpha(120);
    p.setPen(QPen(wc, 0.8));
    for (qreal d : {-1.0, 1.0}) {
        p.drawLine(QPointF(hc.x() + d * 6.0, hc.y() + 3.0), QPointF(hc.x() + d * 14.0, hc.y() + 1.5));
        p.drawLine(QPointF(hc.x() + d * 6.0, hc.y() + 4.0), QPointF(hc.x() + d * 14.0, hc.y() + 5.0));
    }
    p.restore();
    // Sleeping z / startled !
    QFont f(font());
    f.setBold(true);
    if (cat == SLEEPING) {
        const qreal ph = pyMod(now * 0.5, 1.0);
        f.setPointSizeF(6.5 + 3.0 * ph);
        p.setFont(f);
        QColor zc(m_textColor);
        zc.setAlpha(int(200 * std::sin(ph * PI)));
        p.setPen(zc);
        p.drawText(QPointF(base.x() - 28.0 - ph * 6.0, base.y() - 16.0 - ph * 10.0), QStringLiteral("z"));
    } else if (cat == STARTLED) {
        f.setPointSizeF(11.0);
        p.setFont(f);
        p.setPen(QColor(QStringLiteral("#E63946")));
        p.drawText(QPointF(base.x() + 14.0, base.y() - 30.0 - std::sin(clamp(since / STARTLE_TIME, 0, 1) * PI) * 8.0),
                   QStringLiteral("!"));
    }
}

// ======================================================================================================
// SleepyCatSectionWidget
// ======================================================================================================
SleepyCatSectionWidget::SleepyCatSectionWidget(QWidget *parent)
    : SleepyCatSectionWidget(Options(), parent)
{
}

SleepyCatSectionWidget::SleepyCatSectionWidget(const Options &options, QWidget *parent)
    : QWidget(parent)
{
    if (options.toggled)
        connect(this, &SleepyCatSectionWidget::toggled, this, options.toggled);
    // Children.
    CatHeaderWidget::Options ho;
    ho.title = options.title;
    ho.expanded = options.expanded;
    ho.catColor = options.catColor;
    ho.barColor = options.barColor;
    ho.textColor = options.textColor;
    ho.clicked = [this]() { toggle(); };
    m_header = new CatHeaderWidget(ho);
    m_body = new QWidget();
    m_bodyLayout = new QVBoxLayout(m_body);
    m_bodyLayout->setContentsMargins(12, 8, 12, 8);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_header);
    layout->addWidget(m_body);
    // Height animation.
    m_expanded = options.expanded;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SleepyCatSectionWidget::tick);
    if (options.content != nullptr)
        setContentWidget(options.content);
    snap();
}

// ---------- Helpers ----------
qreal SleepyCatSectionWidget::targetHeight() const
{
    return m_expanded ? qreal(m_body->sizeHint().height()) : 0.0;
}

void SleepyCatSectionWidget::snap()
{
    m_height = targetHeight();
    m_body->setMaximumHeight(!m_expanded ? int(m_height) : 16777215);
    m_body->setVisible(m_expanded);
    // Lifting the height cap does not by itself make the parent re-layout, so ask explicitly.
    m_body->updateGeometry();
    updateGeometry();
}

void SleepyCatSectionWidget::tick()
{
    const qreal target = targetHeight();
    m_height += (target - m_height) * 0.22;
    if (std::abs(target - m_height) < 1.0) {
        m_timer->stop();
        snap();
        return;
    }
    m_body->setVisible(true);
    m_body->setMaximumHeight(int(m_height));
}

// ---------- Public API ----------
void SleepyCatSectionWidget::toggle()
{
    setExpanded(!m_expanded);
}

bool SleepyCatSectionWidget::isExpanded() const
{
    return m_expanded;
}

void SleepyCatSectionWidget::setExpanded(bool expanded)
{
    if (expanded == m_expanded)
        return;
    m_height = m_body->isVisible() ? qreal(m_body->height()) : 0.0;
    m_expanded = expanded;
    m_header->setExpanded(expanded);
    m_body->setMaximumHeight(int(m_height));
    m_timer->start(16);
    emit toggled(expanded);
}

QWidget *SleepyCatSectionWidget::getContentWidget() const
{
    return m_content;
}

void SleepyCatSectionWidget::setContentWidget(QWidget *content)
{
    if (m_content != nullptr) {
        m_bodyLayout->removeWidget(m_content);
        m_content->setParent(nullptr);
    }
    m_content = content;
    if (content != nullptr)
        m_bodyLayout->addWidget(content);
    snap();
}

CatHeaderWidget *SleepyCatSectionWidget::getHeader() const
{
    return m_header;
}

QString SleepyCatSectionWidget::getTitle() const
{
    return m_header->getTitle();
}

void SleepyCatSectionWidget::setTitle(const QString &title)
{
    m_header->setTitle(title);
}
