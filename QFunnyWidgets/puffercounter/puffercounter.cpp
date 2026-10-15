/**
 * PufferCounterWidget: a plain-text area with a character limit, watched by a pufferfish in its footer
 * (PufferFooterWidget). The fish inflates as you type, grows spikes near the limit, turns red and furious when
 * you exceed it, and sighs with relief ("pfff") when you get back under it.
 */
#include "puffercounter.h"

#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QTextCursor>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

const qreal PI = 3.14159265358979323846;

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

QColor lerpColor(const QColor &a, const QColor &b, qreal t)
{
    t = clamp(t, 0.0, 1.0);
    return QColor(int(a.red() + (b.red() - a.red()) * t), int(a.green() + (b.green() - a.green()) * t),
                  int(a.blue() + (b.blue() - a.blue()) * t));
}

/** len(text) in Python: number of code points. */
int pyLen(const QString &s)
{
    int n = 0;
    for (int i = 0; i < s.size(); ++i) {
        if (s.at(i).isHighSurrogate() && i + 1 < s.size() && s.at(i + 1).isLowSurrogate())
            ++i;
        ++n;
    }
    return n;
}

/** text[:n] in Python: the first n code points. */
QString pyLeft(const QString &s, int n)
{
    int i = 0;
    for (int k = 0; k < n && i < s.size(); ++k) {
        if (s.at(i).isHighSurrogate() && i + 1 < s.size() && s.at(i + 1).isLowSurrogate())
            ++i;
        ++i;
    }
    return s.left(i);
}

} // namespace

// ======================================================================================================
// PufferFooterWidget
// ======================================================================================================
PufferFooterWidget::PufferFooterWidget(QWidget *parent)
    : PufferFooterWidget(Options(), parent)
{
}

PufferFooterWidget::PufferFooterWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_count(std::max(0, options.count)),
      m_maxLength(std::max(1, options.maxLength)),
      m_fishColor(options.fishColor)
{
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFixedHeight(64);
    // State.
    m_puff = m_count / qreal(m_maxLength);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PufferFooterWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal PufferFooterWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

qreal PufferFooterWidget::ratio() const
{
    return m_count / qreal(m_maxLength);
}

// ---------- Public API ----------
int PufferFooterWidget::getCount() const
{
    return m_count;
}

void PufferFooterWidget::setCount(int count)
{
    count = std::max(0, count);
    if (count != m_count) {
        if (m_count > m_maxLength && m_maxLength >= count)
            m_deflateAt = now();
        m_count = count;
        update();
    }
}

int PufferFooterWidget::getMaxLength() const
{
    return m_maxLength;
}

void PufferFooterWidget::setMaxLength(int maxLength)
{
    m_maxLength = std::max(1, maxLength);
    update();
}

QColor PufferFooterWidget::getFishColor() const
{
    return m_fishColor;
}

void PufferFooterWidget::setFishColor(const QColor &color)
{
    m_fishColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize PufferFooterWidget::sizeHint() const
{
    return QSize(360, 64);
}

void PufferFooterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void PufferFooterWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void PufferFooterWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal target = clamp(ratio(), 0.0, 1.3);
    m_puffVel += ((target - m_puff) * 140.0 - m_puffVel * 9.0) * dt;
    m_puff += m_puffVel * dt;
    const qreal fx = width() - 46.0;
    const qreal fy = height() / 2.0;
    m_bubbleAcc += dt * (1.2 + (t - m_deflateAt < 0.8 ? 6.0 : 0.0));
    while (m_bubbleAcc > 1.0) {
        m_bubbleAcc -= 1.0;
        const qreal x = fx + 20.0 + uniform(-4, 6);
        const qreal r = uniform(1.5, 3.5);
        const qreal life = uniform(0.8, 1.4);
        m_bubbles.append(Bubble(x, fy - 4.0, r, life));
    }
    QList<Bubble> alive;
    for (Bubble b : std::as_const(m_bubbles)) {
        b.y -= 22.0 * dt;
        b.x += std::sin(t * 4.0 + b.y * 0.2) * 6.0 * dt;
        b.life -= dt;
        if (b.life > 0)
            alive.append(b);
    }
    m_bubbles = alive;
    update();
}

// ---------- Painting ----------
void PufferFooterWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const qreal r = ratio();
    const bool over = m_count > m_maxLength;
    const QColor tc(palette().text().color());
    // Counter text + mini progress line.
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(10.0);
    p.setFont(f);
    const int remaining = m_maxLength - m_count;
    QColor col;
    if (over)
        col = QColor(QStringLiteral("#E63946"));
    else if (r > 0.8)
        col = QColor(QStringLiteral("#FF9F1C"));
    else
        col = QColor(tc);
    p.setPen(col);
    p.drawText(QRectF(4, 6, 220, 20), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("%1 / %2").arg(m_count).arg(m_maxLength));
    QFont f2(font());
    f2.setPointSizeF(8.5);
    f2.setItalic(true);
    p.setFont(f2);
    QColor sub(col);
    if (!over)
        sub.setAlpha(160);
    p.setPen(sub);
    QString msg;
    if (over)
        msg = tr("%1 over. The fish is furious.").arg(-remaining);
    else if (tNow - m_deflateAt < 1.2)
        msg = tr("pfff... thank you.");
    else if (m_count == 0)
        msg = tr("Say something. The fish is bored.");
    else if (r > 0.8)
        msg = tr("%1 left. Getting puffy...").arg(remaining);
    else
        msg = tr("%1 characters left").arg(remaining);
    p.drawText(QRectF(4, 26, width() - 100, 18), Qt::AlignLeft | Qt::AlignVCenter, msg);
    const QRectF bar(4, 50, std::max(10.0, width() - 110.0), 4);
    QColor track(tc);
    track.setAlpha(35);
    p.setPen(Qt::NoPen);
    p.setBrush(track);
    p.drawRoundedRect(bar, 2, 2);
    p.setBrush((over || r > 0.8) ? col : QColor(QStringLiteral("#3BCEAC")));
    p.drawRoundedRect(QRectF(bar.left(), bar.top(), bar.width() * clamp(r, 0.0, 1.0), 4), 2, 2);
    // Bubbles + fish.
    for (const Bubble &b : std::as_const(m_bubbles)) {
        QColor bc(QStringLiteral("#9ED8F5"));
        bc.setAlpha(int(180 * clamp(b.life / b.maxLife, 0.0, 1.0)));
        p.setPen(QPen(bc, 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(b.x, b.y), b.r, b.r);
    }
    drawFish(p, QPointF(width() - 46.0, height() / 2.0), tNow);
    p.end();
}

void PufferFooterWidget::drawFish(QPainter &p, const QPointF &c, qreal now)
{
    const qreal puff = clamp(m_puff, 0.0, 1.3);
    const bool over = m_count > m_maxLength;
    const bool asleep = m_count == 0;
    const qreal r = 9.0 + 15.0 * std::min(puff, 1.15);
    const QColor body = lerpColor(m_fishColor, QColor(QStringLiteral("#E63946")), (puff - 0.9) / 0.25);
    const QColor ink(QStringLiteral("#2B2B33"));
    p.save();
    const qreal jx = over ? uniform(-1.5, 1.5) : 0.0;
    p.translate(c.x() + jx, c.y() + std::sin(now * 2.0) * 2.0);
    // Tail (wiggles).
    const qreal wig = std::sin(now * (!asleep ? 10.0 : 3.0)) * r * 0.15;
    QPainterPath tail;
    tail.moveTo(-r * 0.85, 0);
    tail.lineTo(-r * 1.35, -r * 0.45 + wig);
    tail.lineTo(-r * 1.35, r * 0.45 + wig);
    tail.closeSubpath();
    p.setPen(QPen(body.darker(140), 1.0));
    p.setBrush(body.darker(110));
    p.drawPath(tail);
    // Spikes appear past 80 %.
    const qreal spikes = clamp((puff - 0.75) / 0.3, 0.0, 1.0);
    if (spikes > 0) {
        p.setPen(QPen(body.darker(150), std::max(1.0, r * 0.06), Qt::SolidLine, Qt::RoundCap));
        for (int k = 0; k < 14; ++k) {
            const qreal a = k * 2.0 * PI / 14.0;
            p.drawLine(QPointF(std::cos(a) * r * 0.95, std::sin(a) * r * 0.95),
                       QPointF(std::cos(a) * r * (0.95 + 0.35 * spikes), std::sin(a) * r * (0.95 + 0.35 * spikes)));
        }
    }
    // Body.
    QRadialGradient grad(QPointF(-r * 0.3, -r * 0.35), r * 1.6);
    grad.setColorAt(0.0, body.lighter(120));
    grad.setColorAt(1.0, body.darker(112));
    p.setPen(QPen(body.darker(145), 1.2));
    p.setBrush(grad);
    p.drawEllipse(QPointF(0, 0), r, r * 0.92);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 60));
    p.drawEllipse(QPointF(0, r * 0.35), r * 0.6, r * 0.35);
    // Spots.
    p.setBrush(body.darker(125));
    static const qreal spots[3][2] = {{-0.35, -0.4}, {-0.05, -0.55}, {-0.5, -0.05}};
    for (const auto &sp : spots)
        p.drawEllipse(QPointF(sp[0] * r, sp[1] * r), r * 0.08, r * 0.08);
    // Side fin.
    p.setBrush(body.darker(108));
    p.setPen(QPen(body.darker(140), 1.0));
    p.drawEllipse(QPointF(-r * 0.1, r * 0.2), r * 0.22, r * 0.12);
    // Eye.
    const qreal ex = r * 0.42;
    const qreal ey = -r * 0.22;
    if (asleep) {
        p.setPen(QPen(ink, 1.4, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(QRectF(ex - r * 0.15, ey - r * 0.05, r * 0.3, r * 0.18), 180 * 16, 180 * 16);
    } else {
        const qreal er = r * (!over ? 0.2 : 0.24);
        p.setPen(QPen(ink, 1.0));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(ex, ey), er, er);
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        const qreal pr = er * (0.55 - 0.25 * clamp(puff - 0.8, 0.0, 0.5));
        p.drawEllipse(QPointF(ex + er * 0.2, ey), pr, pr);
        if (over) {
            p.setPen(QPen(ink, std::max(1.5, r * 0.08), Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(ex - er * 1.1, ey - er * 1.3), QPointF(ex + er * 0.9, ey - er * 0.7));
        }
    }
    // Mouth: little "o", pursed when puffed.
    p.setPen(QPen(ink, 1.2));
    if (over)
        p.setBrush(QColor(QStringLiteral("#7A2E2E")));
    else
        p.setBrush(Qt::NoBrush);
    const qreal mo = r * (0.08 + (over ? 0.07 : 0.0));
    p.drawEllipse(QPointF(r * 0.86, r * 0.12), mo, mo * (over ? 1.2 : 1.0));
    p.restore();
    if (asleep) {
        QFont f(font());
        f.setBold(true);
        const qreal ph = pyMod(now * 0.5, 1.0);
        f.setPointSizeF(6.5 + 3.0 * ph);
        p.setFont(f);
        QColor zc(palette().text().color());
        zc.setAlpha(int(200 * std::sin(ph * PI)));
        p.setPen(zc);
        p.drawText(QPointF(c.x() + 10.0 + ph * 8.0, c.y() - 12.0 - ph * 12.0), QStringLiteral("z"));
    }
}

// ======================================================================================================
// PufferCounterWidget
// ======================================================================================================
PufferCounterWidget::PufferCounterWidget(QWidget *parent)
    : PufferCounterWidget(Options(), parent)
{
}

PufferCounterWidget::PufferCounterWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_enforce(options.enforceLimit)
{
    if (options.textChanged)
        connect(this, &PufferCounterWidget::textChanged, this, options.textChanged);
    if (options.limitExceeded)
        connect(this, &PufferCounterWidget::limitExceeded, this, options.limitExceeded);
    // Children.
    m_edit = new QPlainTextEdit();
    m_edit->setObjectName(QStringLiteral("pufferEdit"));
    m_edit->setPlaceholderText(options.placeholder);
    PufferFooterWidget::Options fo;
    fo.maxLength = options.maxLength;
    fo.fishColor = options.fishColor;
    m_footer = new PufferFooterWidget(fo);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(m_edit, 1);
    layout->addWidget(m_footer);
    connect(m_edit, &QPlainTextEdit::textChanged, this, &PufferCounterWidget::onText);
    m_edit->setPlainText(options.text);
}

// ---------- Helpers ----------
void PufferCounterWidget::onText()
{
    QString text = m_edit->toPlainText();
    const int limit = m_footer->getMaxLength();
    if (m_enforce && pyLen(text) > limit) {
        QTextCursor cur = m_edit->textCursor();
        const int pos = std::min(cur.position(), limit);
        m_edit->blockSignals(true);
        m_edit->setPlainText(pyLeft(text, limit));
        m_edit->blockSignals(false);
        cur = m_edit->textCursor();
        cur.setPosition(pos);
        m_edit->setTextCursor(cur);
        text = pyLeft(text, limit);
    }
    const int len = pyLen(text);
    m_footer->setCount(len);
    const bool over = len > limit;
    if (over != m_over) {
        m_over = over;
        emit limitExceeded(over);
    }
    emit textChanged(text);
}

// ---------- Public API ----------
QPlainTextEdit *PufferCounterWidget::getEditor() const { return m_edit; }
PufferFooterWidget *PufferCounterWidget::getFooter() const { return m_footer; }
bool PufferCounterWidget::isOverLimit() const { return m_over; }
QString PufferCounterWidget::getText() const { return m_edit->toPlainText(); }
QString PufferCounterWidget::getPlaceholder() const { return m_edit->placeholderText(); }
int PufferCounterWidget::getMaxLength() const { return m_footer->getMaxLength(); }
bool PufferCounterWidget::isEnforceLimit() const { return m_enforce; }

void PufferCounterWidget::setText(const QString &text)
{
    m_edit->setPlainText(text);
}

void PufferCounterWidget::setPlaceholder(const QString &placeholder)
{
    m_edit->setPlaceholderText(placeholder);
}

void PufferCounterWidget::setMaxLength(int maxLength)
{
    m_footer->setMaxLength(maxLength);
    onText();
}

void PufferCounterWidget::setEnforceLimit(bool enforceLimit)
{
    m_enforce = enforceLimit;
    onText();
}
