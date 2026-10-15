/**
 * CopyBlobButtonWidget: a copy-to-clipboard chip (prompt + text) with a little blob living at its end. Clicking it
 * copies the text and the blob undergoes mitosis: its clone hops up "into the clipboard" with a sparkle while a
 * "Copied!" bubble appears.
 */
#include "copyblob.h"

#include <QFontDatabase>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QFontMetricsF>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
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

inline qreal uniform(qreal a, qreal b)
{
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}

inline qreal easeOut(qreal t)
{
    t = clamp(t, 0.0, 1.0);
    return 1.0 - std::pow(1.0 - t, 3);
}

} // namespace

CopyBlobButtonWidget::CopyBlobButtonWidget(QWidget *parent)
    : CopyBlobButtonWidget(Options(), parent)
{
}

CopyBlobButtonWidget::CopyBlobButtonWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_text(options.text),
      m_label(options.label),
      m_prompt(options.prompt),
      m_blobColor(options.blobColor),
      m_chipColor(options.chipColor),
      m_textColor(options.textColor)
{
    if (options.copied)
        connect(this, &CopyBlobButtonWidget::copied, this, options.copied);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setToolTip(tr("Click to copy"));
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CopyBlobButtonWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal CopyBlobButtonWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QRectF CopyBlobButtonWidget::chip() const
{
    const qreal h = height();
    const qreal ch = clamp(h * 0.42, 34.0, 54.0);
    return QRectF(3.0, h - ch - 6.0, width() - 6.0, ch);
}

// ---------- Public API ----------
void CopyBlobButtonWidget::copy()
{
    QClipboard *cb = QApplication::clipboard();
    if (cb != nullptr)
        cb->setText(m_text);
    m_copiedAt = now();
    emit copied(m_text);
    update();
}

QString CopyBlobButtonWidget::getText() const { return m_text; }
QString CopyBlobButtonWidget::getLabel() const { return m_label; }
QString CopyBlobButtonWidget::getPrompt() const { return m_prompt; }
QColor CopyBlobButtonWidget::getBlobColor() const { return m_blobColor; }
QColor CopyBlobButtonWidget::getChipColor() const { return m_chipColor; }
QColor CopyBlobButtonWidget::getTextColor() const { return m_textColor; }

void CopyBlobButtonWidget::setText(const QString &text)
{
    m_text = text;
    update();
}

void CopyBlobButtonWidget::setLabel(const QString &label)
{
    m_label = label;
    update();
}

void CopyBlobButtonWidget::setPrompt(const QString &prompt)
{
    m_prompt = prompt;
    update();
}

void CopyBlobButtonWidget::setBlobColor(const QColor &color)
{
    m_blobColor = color;
    update();
}

void CopyBlobButtonWidget::setChipColor(const QColor &color)
{
    m_chipColor = color;
    update();
}

void CopyBlobButtonWidget::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize CopyBlobButtonWidget::sizeHint() const
{
    return QSize(340, 110);
}

QSize CopyBlobButtonWidget::minimumSizeHint() const
{
    return QSize(180, 90);
}

void CopyBlobButtonWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void CopyBlobButtonWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void CopyBlobButtonWidget::enterEvent(QEnterEvent *e)
#else
void CopyBlobButtonWidget::enterEvent(QEvent *e)
#endif
{
    m_hovered = true;
    QWidget::enterEvent(e);
}

void CopyBlobButtonWidget::leaveEvent(QEvent *e)
{
    m_hovered = false;
    QWidget::leaveEvent(e);
}

void CopyBlobButtonWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && chip().contains(eventPos(e)))
        m_pressed = true;
    QWidget::mousePressEvent(e);
}

void CopyBlobButtonWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressed && chip().contains(eventPos(e)))
        copy();
    m_pressed = false;
    QWidget::mouseReleaseEvent(e);
}

void CopyBlobButtonWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter
        || e->matches(QKeySequence::Copy)) {
        copy();
        return;
    }
    QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void CopyBlobButtonWidget::tick()
{
    const qreal t = now();
    if (t >= m_blinkAt + 0.14)
        m_blinkAt = t + uniform(2.0, 4.5);
    update();
}

// ---------- Painting ----------
void CopyBlobButtonWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const QRectF chp = chip();
    const qreal since = tNow - m_copiedAt;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Chip.
    const QColor cc = m_hovered ? m_chipColor.lighter(115) : m_chipColor;
    const qreal r = chp.height() * 0.3;
    const QRectF rect = m_pressed ? chp.adjusted(1, 1, -1, -1) : chp;
    p.setPen(QPen(QColor(255, 255, 255, 25), 1.0));
    p.setBrush(cc);
    p.drawRoundedRect(rect, r, r);
    const QRectF blobArea(rect.right() - rect.height(), rect.top(), rect.height(), rect.height());
    p.setPen(QPen(QColor(255, 255, 255, 30), 1.0));
    p.drawLine(QPointF(blobArea.left(), rect.top() + 8), QPointF(blobArea.left(), rect.bottom() - 8));
    // Text.
    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont); // portable "Monospace"
    f.setPointSizeF(clamp(rect.height() * 0.26, 9.0, 12.0));
    p.setFont(f);
    const QFontMetricsF fm(f);
    qreal x = rect.left() + 14.0;
    if (!m_prompt.isEmpty()) {
        p.setPen(QColor(m_blobColor));
        p.drawText(QRectF(x, rect.top(), 30, rect.height()), Qt::AlignLeft | Qt::AlignVCenter, m_prompt);
        x += fm.horizontalAdvance(m_prompt + QStringLiteral(" "));
    }
    const QString label = !m_label.isEmpty() ? m_label : m_text;
    const qreal avail = blobArea.left() - x - 10.0;
    p.setPen(m_textColor);
    p.drawText(QRectF(x, rect.top(), avail, rect.height()), Qt::AlignLeft | Qt::AlignVCenter,
               fm.elidedText(label, Qt::ElideMiddle, avail));
    // Blob(s).
    const QPointF bc = blobArea.center();
    const qreal br = rect.height() * 0.3;
    drawBlobs(p, bc, br, since, tNow);
    // "Copied!" bubble.
    if (0.25 < since && since < BUBBLE) {
        const qreal a = clamp(std::min((since - 0.25) * 6.0, (BUBBLE - since) * 4.0), 0.0, 1.0);
        QFont bf(font());
        bf.setBold(true);
        bf.setPointSizeF(8.5);
        const QFontMetricsF bfm(bf);
        const QString txt = tr("Copied!");
        const qreal tw = bfm.horizontalAdvance(txt);
        const qreal bw = tw + 16.0;
        const qreal bh = bfm.height() + 8.0;
        const qreal bx = clamp(bc.x() - bw - br * 0.6, 2.0, width() - bw - 2.0);
        const qreal by = std::max(2.0, chp.top() - bh - 8.0 - (1.0 - a) * 6.0);
        p.save();
        p.setOpacity(a);
        p.setPen(Qt::NoPen);
        p.setBrush(m_blobColor);
        p.drawRoundedRect(QRectF(bx, by, bw, bh), bh / 2.0, bh / 2.0);
        p.setFont(bf);
        p.setPen(QColor(QStringLiteral("#1E2A12")));
        p.drawText(QRectF(bx, by, bw, bh), Qt::AlignCenter, txt);
        p.restore();
    }
    p.end();
}

void CopyBlobButtonWidget::drawBlobs(QPainter &p, const QPointF &c, qreal r, qreal since, qreal now)
{
    if (since < SPLIT) {
        // Mitosis: two halves separate, pinching at the waist.
        const qreal t = easeOut(since / SPLIT);
        const qreal d = r * 0.85 * t;
        const qreal rr = r * (1.0 - 0.22 * std::sin(t * PI));
        QPainterPath body;
        body.addEllipse(QPointF(c.x() - d, c.y()), rr, rr);
        QPainterPath other;
        other.addEllipse(QPointF(c.x() + d, c.y()), rr, rr);
        paintBlobShape(p, body.united(other));
        for (qreal dx : {-d, d})
            drawBlobFace(p, QPointF(c.x() + dx, c.y()), rr * 0.9, now, true);
        return;
    }
    if (since < SPLIT + HOP) {
        const qreal t = (since - SPLIT) / HOP;
        // Original slides home; the clone arcs up into "the clipboard".
        const QPointF home(c.x() - r * 0.85 * (1.0 - easeOut(t * 2.0)), c.y());
        paintBlob(p, home, r, now, true);
        const qreal k = clamp(t * 1.15, 0.0, 1.0);
        const qreal cx = c.x() + r * 0.85 - k * r * 1.2;
        const qreal cy = c.y() - std::sin(k * PI * 0.8) * r * 3.2 - k * r * 0.8;
        const qreal cr = r * std::pow(1.0 - k, 0.7);
        if (cr > 1.0) {
            paintBlob(p, QPointF(cx, cy), cr, now, true);
        } else if (k >= 1.0 && t < 0.95) {
            p.setPen(QPen(QColor(QStringLiteral("#FFF6C8")), 1.6, Qt::SolidLine, Qt::RoundCap));
            for (int a = 0; a < 4; ++a) {
                const qreal ang = a * PI / 2.0 + now * 3.0;
                p.drawLine(QPointF(cx, cy), QPointF(cx + std::cos(ang) * r * 0.5, cy + std::sin(ang) * r * 0.5));
            }
        }
        return;
    }
    const qreal bob = 1.0 + 0.04 * std::sin(now * 3.0);
    paintBlob(p, c, r * bob, now, false);
}

void CopyBlobButtonWidget::paintBlobShape(QPainter &p, const QPainterPath &shape)
{
    const QColor bc = m_blobColor;
    p.setPen(QPen(bc.darker(140), 1.2));
    p.setBrush(bc);
    p.drawPath(shape);
}

void CopyBlobButtonWidget::paintBlob(QPainter &p, const QPointF &c, qreal r, qreal now, bool happy)
{
    QPainterPath shape;
    shape.addEllipse(QPointF(c.x(), c.y() + r * 0.05), r * 1.05, r * 0.95);
    paintBlobShape(p, shape);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 80));
    p.drawEllipse(QPointF(c.x() - r * 0.4, c.y() - r * 0.4), r * 0.22, r * 0.13);
    drawBlobFace(p, c, r, now, happy);
}

void CopyBlobButtonWidget::drawBlobFace(QPainter &p, const QPointF &c, qreal r, qreal now, bool happy)
{
    const QColor ink(QStringLiteral("#1E2A12"));
    const qreal ex = r * 0.34;
    const qreal ey = c.y() - r * 0.1;
    const QPen pen(ink, std::max(1.0, r * 0.12), Qt::SolidLine, Qt::RoundCap);
    const bool blinking = m_blinkAt <= now && now < m_blinkAt + 0.14;
    if (happy || blinking) {
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const int a0 = happy ? 20 * 16 : 180 * 16;
        const int a1 = happy ? 140 * 16 : 180 * 16;
        for (qreal d : {-1.0, 1.0})
            p.drawArc(QRectF(c.x() + d * ex - r * 0.16, ey - r * 0.1, r * 0.32, r * 0.24), a0, a1);
    } else {
        const QPoint cur = mapFromGlobal(QCursor::pos());
        const qreal lx = clamp((cur.x() - c.x()) / 120.0, -1.0, 1.0) * r * 0.08;
        const qreal ly = clamp((cur.y() - c.y()) / 120.0, -1.0, 1.0) * r * 0.08;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(c.x() + d * ex, ey), r * 0.2, r * 0.22);
        p.setBrush(ink);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(c.x() + d * ex + lx, ey + ly), r * 0.1, r * 0.11);
    }
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(c.x() - r * 0.2, c.y() + r * 0.05, r * 0.4, r * 0.3), 200 * 16, 140 * 16);
}
