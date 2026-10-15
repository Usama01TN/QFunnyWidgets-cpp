/**
 * AutosaveSquirrelWidget: an autosave status indicator starring a squirrel. It naps hugging an acorn when
 * everything is saved, gets alert when there are unsaved changes, hops back and forth stashing acorns into its
 * tree while saving, and facepalms after dropping the acorn when saving fails. It debounces autosave requests and
 * retries failed saves.
 */
#include "autosave.h"

#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace {

const qreal PI = 3.14159265358979323846;

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

} // namespace

QColor AutosaveSquirrelWidget::statusColor(int status)
{
    switch (status) {
    case Saved:
        return QColor(QStringLiteral("#3BCEAC"));
    case Unsaved:
        return QColor(QStringLiteral("#FF9F1C"));
    case Saving:
        return QColor(QStringLiteral("#4CC9F0"));
    default:
        return QColor(QStringLiteral("#E63946"));
    }
}

AutosaveSquirrelWidget::AutosaveSquirrelWidget(QWidget *parent)
    : AutosaveSquirrelWidget(Options(), parent)
{
}

AutosaveSquirrelWidget::AutosaveSquirrelWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_status(Status(clampInt(options.status, 0, 3))),
      m_squirrelColor(options.squirrelColor)
{
    if (options.saveRequested)
        connect(this, &AutosaveSquirrelWidget::saveRequested, this, options.saveRequested);
    if (options.statusChanged)
        connect(this, &AutosaveSquirrelWidget::statusChanged, this, options.statusChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // Debounce / retry timers.
    m_autosave = new QTimer(this);
    m_autosave->setSingleShot(true);
    m_autosave->setInterval(std::max(0, options.autosaveDelay));
    connect(m_autosave, &QTimer::timeout, this, &AutosaveSquirrelWidget::requestSave);
    m_retry = new QTimer(this);
    m_retry->setSingleShot(true);
    m_retry->setInterval(std::max(0, options.retryDelay));
    connect(m_retry, &QTimer::timeout, this, &AutosaveSquirrelWidget::requestSave);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal AutosaveSquirrelWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void AutosaveSquirrelWidget::changeStatus(int status)
{
    const Status st = Status(status);
    if (st != m_status) {
        m_status = st;
        m_statusAt = now();
        emit statusChanged(int(st));
        update();
    }
}

QString AutosaveSquirrelWidget::ago() const
{
    const int s = int(now() - m_savedAt);
    if (s < 10)
        return tr("just now");
    if (s < 60)
        return tr("%1 s ago").arg(s);
    return tr("%1 min ago").arg(s / 60);
}

// ---------- Public API ----------
void AutosaveSquirrelWidget::markDirty()
{
    if (m_status == Saving) {
        m_dirtyAgain = true;
        return;
    }
    m_retry->stop();
    changeStatus(Unsaved);
    if (m_autosave->interval() > 0)
        m_autosave->start();
}

void AutosaveSquirrelWidget::requestSave()
{
    m_autosave->stop();
    m_retry->stop();
    m_dirtyAgain = false;
    changeStatus(Saving);
    emit saveRequested();
}

void AutosaveSquirrelWidget::saveFinished(bool ok)
{
    if (ok) {
        m_savedAt = now();
        changeStatus(Saved);
        if (m_dirtyAgain)
            markDirty();
    } else {
        changeStatus(Error);
        if (m_retry->interval() > 0)
            m_retry->start();
    }
}

AutosaveSquirrelWidget::Status AutosaveSquirrelWidget::getStatus() const
{
    return m_status;
}

void AutosaveSquirrelWidget::setStatus(int status)
{
    changeStatus(clampInt(status, 0, 3));
}

int AutosaveSquirrelWidget::getAutosaveDelay() const { return m_autosave->interval(); }
int AutosaveSquirrelWidget::getRetryDelay() const { return m_retry->interval(); }
QColor AutosaveSquirrelWidget::getSquirrelColor() const { return m_squirrelColor; }

void AutosaveSquirrelWidget::setAutosaveDelay(int autosaveDelay)
{
    m_autosave->setInterval(std::max(0, autosaveDelay));
}

void AutosaveSquirrelWidget::setRetryDelay(int retryDelay)
{
    m_retry->setInterval(std::max(0, retryDelay));
}

void AutosaveSquirrelWidget::setSquirrelColor(const QColor &color)
{
    m_squirrelColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize AutosaveSquirrelWidget::sizeHint() const
{
    return QSize(300, 76);
}

QSize AutosaveSquirrelWidget::minimumSizeHint() const
{
    return QSize(180, 56);
}

void AutosaveSquirrelWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void AutosaveSquirrelWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Painting ----------
void AutosaveSquirrelWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    const qreal h = height();
    const qreal s = std::min(h, 84.0);
    const QRectF icon(0, (h - s) / 2.0, s * 1.35, s);
    const int st = int(m_status);
    const qreal since = tNow - m_statusAt;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    drawScene(p, icon, s, st, since, tNow);
    // Status text.
    const QColor tc(palette().text().color());
    const QColor sc = statusColor(st);
    const qreal x0 = icon.right() + 10.0;
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(10.0);
    p.setFont(f);
    p.setPen(st != Saved ? sc : tc);
    QString mainText;
    QString sub;
    if (st == Saved) {
        mainText = tr("All changes saved");
        sub = m_savedAt > 0 ? tr("Saved %1. Squirrel is napping.").arg(ago()) : tr("Nothing to save. Zzz.");
    } else if (st == Unsaved) {
        mainText = tr("Unsaved changes");
        sub = m_autosave->isActive() ? tr("Autosaving shortly...") : tr("Squirrel is on alert.");
    } else if (st == Saving) {
        mainText = tr("Saving") + QString(1 + int(tNow * 3.0) % 3, QLatin1Char('.'));
        sub = tr("Stashing your changes for winter.");
    } else {
        mainText = tr("Couldn't save");
        sub = m_retry->isActive() ? tr("The acorn got away. Retrying...") : tr("The acorn got away.");
    }
    const qreal cy = h / 2.0;
    p.drawText(QRectF(x0, cy - 20.0, width() - x0, 20.0), Qt::AlignLeft | Qt::AlignVCenter, mainText);
    QFont f2(font());
    f2.setPointSizeF(8.5);
    p.setFont(f2);
    QColor sub2(tc);
    sub2.setAlpha(160);
    p.setPen(sub2);
    p.drawText(QRectF(x0, cy, width() - x0, 18.0), Qt::AlignLeft | Qt::AlignVCenter, sub);
    p.end();
}

void AutosaveSquirrelWidget::drawScene(QPainter &p, const QRectF &icon, qreal s, int st, qreal since, qreal)
{
    const qreal ground = icon.bottom() - s * 0.1;
    // Tree trunk with a stash hole.
    const QRectF trunk(icon.right() - s * 0.3, icon.top() + s * 0.05, s * 0.24, ground - icon.top() - s * 0.05);
    p.setPen(QPen(QColor(QStringLiteral("#5A3A22")), 1.0));
    p.setBrush(QColor(QStringLiteral("#7A5232")));
    p.drawRect(trunk);
    const QPointF hole(trunk.center().x(), trunk.top() + trunk.height() * 0.4);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#2E1C10")));
    p.drawEllipse(hole, s * 0.06, s * 0.08);
    p.setPen(QPen(QColor(0, 0, 0, 40), 1.0));
    p.drawLine(QPointF(icon.left(), ground), QPointF(icon.right(), ground));
    // Squirrel position (hops toward the tree while saving).
    qreal sx = icon.left() + s * 0.42;
    qreal hop = 0.0;
    if (st == Saving) {
        const qreal ph = pyMod(since, 0.9) / 0.9;
        sx += std::sin(ph * PI) * s * 0.18;
        hop = std::abs(std::sin(ph * PI * 2.0)) * s * 0.1;
    }
    const QPointF base(sx, ground - hop);
    drawSquirrel(p, base, s, st, since, now());
    // Acorns.
    if (st == Saving) {
        const qreal ph = pyMod(since, 0.9) / 0.9;
        if (ph > 0.35) {
            const qreal t = (ph - 0.35) / 0.65;
            const QPointF a0(base.x() + s * 0.12, base.y() - s * 0.3);
            const qreal ax = a0.x() + (hole.x() - a0.x()) * t;
            const qreal ay = a0.y() + (hole.y() - a0.y()) * t - std::sin(t * PI) * s * 0.25;
            if (t < 0.92)
                drawAcorn(p, QPointF(ax, ay), s * 0.07, t * 540.0);
        }
    } else if (st == Error) {
        const qreal t = clamp(since / 1.2, 0.0, 1.0);
        const qreal ax = base.x() + s * 0.12 + t * s * 0.5;
        const qreal ay = std::min(ground - s * 0.07, base.y() - s * 0.3 + t * t * s * 0.6);
        drawAcorn(p, QPointF(ax, ay), s * 0.07, t * 720.0);
    } else if (st == Saved) {
        // Little check badge.
        const QPointF bc(icon.left() + s * 0.15, icon.top() + s * 0.2);
        p.setPen(Qt::NoPen);
        p.setBrush(statusColor(0));
        p.drawEllipse(bc, s * 0.09, s * 0.09);
        p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), std::max(1.5, s * 0.025), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPainterPath ck;
        ck.moveTo(bc.x() - s * 0.04, bc.y());
        ck.lineTo(bc.x() - s * 0.01, bc.y() + s * 0.03);
        ck.lineTo(bc.x() + s * 0.045, bc.y() - s * 0.035);
        p.setBrush(Qt::NoBrush);
        p.drawPath(ck);
    }
}

void AutosaveSquirrelWidget::drawSquirrel(QPainter &p, const QPointF &b, qreal s, int st, qreal since, qreal now)
{
    const QColor col = m_squirrelColor;
    const QColor ink(QStringLiteral("#2B1A10"));
    const QPen outline(col.darker(150), 1.0);
    const bool sleeping = st == Saved;
    // Bushy tail.
    const qreal twitch = std::sin(now * (st == Unsaved ? 14.0 : 2.0)) * s * (st == Unsaved ? 0.03 : 0.01);
    QPainterPath tail;
    if (sleeping) {
        tail.moveTo(b.x() - s * 0.15, b.y() - s * 0.05);
        tail.cubicTo(b.x() - s * 0.45, b.y() - s * 0.1, b.x() - s * 0.35, b.y() - s * 0.5, b.x() + s * 0.05, b.y() - s * 0.38);
        tail.cubicTo(b.x() - s * 0.18, b.y() - s * 0.32, b.x() - s * 0.25, b.y() - s * 0.15, b.x() - s * 0.1, b.y() - s * 0.08);
    } else {
        tail.moveTo(b.x() - s * 0.12, b.y() - s * 0.08);
        tail.cubicTo(b.x() - s * 0.5, b.y() - s * 0.15, b.x() - s * 0.42, b.y() - s * 0.62 + twitch, b.x() - s * 0.2,
                     b.y() - s * 0.72 + twitch);
        tail.cubicTo(b.x() - s * 0.3, b.y() - s * 0.5, b.x() - s * 0.28, b.y() - s * 0.25, b.x() - s * 0.05, b.y() - s * 0.15);
    }
    tail.closeSubpath();
    p.setPen(outline);
    p.setBrush(col.lighter(118));
    p.drawPath(tail);
    // Body.
    p.setBrush(col);
    QPointF hc;
    if (sleeping) {
        p.drawEllipse(QPointF(b.x(), b.y() - s * 0.12), s * 0.17, s * 0.12);
        hc = QPointF(b.x() + s * 0.14, b.y() - s * 0.13);
    } else {
        p.drawEllipse(QPointF(b.x(), b.y() - s * 0.18), s * 0.12, s * 0.17);
        p.setBrush(QColor(QStringLiteral("#F2D2B0")));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(b.x() + s * 0.03, b.y() - s * 0.15), s * 0.06, s * 0.11);
        p.setPen(outline);
        p.setBrush(col);
        hc = QPointF(b.x() + s * 0.05, b.y() - s * 0.4);
    }
    // Head.
    p.drawEllipse(hc, s * 0.1, s * 0.09);
    QPainterPath ear;
    ear.moveTo(hc.x() - s * 0.03, hc.y() - s * 0.07);
    ear.lineTo(hc.x() - s * 0.01, hc.y() - s * 0.16);
    ear.lineTo(hc.x() + s * 0.04, hc.y() - s * 0.07);
    ear.closeSubpath();
    p.drawPath(ear);
    p.setPen(Qt::NoPen);
    p.setBrush(ink);
    p.drawEllipse(QPointF(hc.x() + s * 0.1, hc.y() + s * 0.01), s * 0.018, s * 0.016); // nose
    // Eye.
    const qreal ex = hc.x() + s * 0.035;
    const qreal ey = hc.y() - s * 0.02;
    if (sleeping) {
        p.setPen(QPen(ink, 1.2, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(ex - s * 0.025, ey - s * 0.01, s * 0.05, s * 0.03), 180 * 16, 180 * 16);
        QFont f(font());
        f.setBold(true);
        const qreal ph = pyMod(now * 0.5, 1.0);
        f.setPixelSize(std::max(6, int(s * (0.1 + 0.06 * ph))));
        p.setFont(f);
        QColor zc(palette().text().color());
        zc.setAlpha(int(200 * std::sin(ph * PI)));
        p.setPen(zc);
        p.drawText(QPointF(hc.x() + s * 0.08 + ph * s * 0.08, hc.y() - s * 0.12 - ph * s * 0.12), QStringLiteral("z"));
    } else if (st == Error && since > 0.6) {
        // Facepalm.
        p.setPen(QPen(col.darker(130), std::max(2.0, s * 0.05), Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(b.x() + s * 0.05, b.y() - s * 0.22), QPointF(ex, ey));
    } else {
        const bool big = st == Unsaved;
        p.setBrush(ink);
        p.drawEllipse(QPointF(ex, ey), s * (big ? 0.026 : 0.02), s * (big ? 0.03 : 0.022));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(ex - s * 0.006, ey - s * 0.01), s * 0.007, s * 0.007);
    }
    // Acorn held in paws (asleep: hugged; alert: held up).
    if (sleeping) {
        drawAcorn(p, QPointF(b.x() + s * 0.05, b.y() - s * 0.05), s * 0.06, 0.0);
    } else if (st == Unsaved) {
        drawAcorn(p, QPointF(b.x() + s * 0.12, b.y() - s * 0.27), s * 0.065, std::sin(now * 3.0) * 10.0);
        QFont f(font());
        f.setBold(true);
        f.setPixelSize(std::max(8, int(s * 0.2)));
        p.setFont(f);
        p.setPen(statusColor(1));
        p.drawText(QPointF(hc.x() + s * 0.12, hc.y() - s * 0.1), QStringLiteral("!"));
    }
}

void AutosaveSquirrelWidget::drawAcorn(QPainter &p, const QPointF &c, qreal r, qreal rot)
{
    p.save();
    p.translate(c);
    p.rotate(rot);
    p.setPen(QPen(QColor(QStringLiteral("#8A5A2B")), 0.8));
    p.setBrush(QColor(QStringLiteral("#D9A066")));
    p.drawEllipse(QPointF(0, r * 0.25), r * 0.8, r * 0.95);
    p.setBrush(QColor(QStringLiteral("#7A5232")));
    p.drawChord(QRectF(-r * 0.95, -r * 0.75, r * 1.9, r * 1.2), 0, 180 * 16);
    p.setPen(QPen(QColor(QStringLiteral("#5A3A22")), std::max(1.0, r * 0.2), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(0, -r * 0.75), QPointF(r * 0.15, -r * 1.05));
    p.restore();
}
