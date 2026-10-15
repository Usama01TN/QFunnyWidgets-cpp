/**
 * ChameleonColorPickerWidget: a swatch colour picker with a chameleon on a branch. Its turret eyes track the hovered
 * swatch (the far eye wanders independently); picking a colour makes it shoot its tongue at the swatch, "eat" the
 * colour and change colour from snout to tail.
 */
#include "chameleonpicker.h"

#include <QBrush>
#include <QKeyEvent>
#include <QLinearGradient>
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

inline int clampInt(int x, int a, int b)
{
    return x < a ? a : (x > b ? b : x);
}

inline qreal uniform(qreal a, qreal b)
{
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}

inline int pyModInt(int a, int b)
{
    int r = a % b;
    if (r != 0 && ((r < 0) != (b < 0)))
        r += b;
    return r;
}

} // namespace

QList<QColor> ChameleonColorPickerWidget::defaultColors()
{
    QList<QColor> list;
    for (const char *c : {"#E63946", "#FF9F1C", "#FFD23F", "#9BDB4D", "#3BCEAC", "#4CC9F0",
                          "#4361EE", "#7B5CFF", "#C77DFF", "#FF70A6", "#8D6E63", "#495057"})
        list.append(QColor(QString::fromLatin1(c)));
    return list;
}

ChameleonColorPickerWidget::ChameleonColorPickerWidget(QWidget *parent)
    : ChameleonColorPickerWidget(Options(), parent)
{
}

ChameleonColorPickerWidget::ChameleonColorPickerWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_colors(options.colors.isEmpty() ? defaultColors() : options.colors),
      m_color(options.color),
      m_columns(clampInt(options.columns, 1, 8)),
      m_showHex(options.showHex),
      m_branchColor(options.branchColor)
{
    if (options.colorChanged)
        connect(this, &ChameleonColorPickerWidget::colorChanged, this, options.colorChanged);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    // State.
    m_prevColor = m_color;
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ChameleonColorPickerWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal ChameleonColorPickerWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

QList<QRectF> ChameleonColorPickerWidget::layoutSwatches(QPointF &c, qreal &s) const
{
    const qreal w = width();
    const qreal h = height();
    const qreal leftW = w * 0.48;
    const qreal hexH = m_showHex ? 26.0 : 0.0;
    // The chameleon spans about 1.7 s from tail curl to snout.
    s = std::max(30.0, std::min((leftW - 8.0) / 1.72, (h - hexH) * 0.95));
    c = QPointF(s * 0.92 + 6.0, (h - hexH) / 2.0 + s * 0.02);
    const int n = int(m_colors.size());
    const int cols = m_columns;
    const int rows = int(std::ceil(n / qreal(cols)));
    const qreal gap = 8.0;
    const qreal areaW = w - leftW - 12.0;
    const qreal cell = std::max(12.0, std::min({(areaW - gap * (cols - 1)) / cols, (h - 16.0 - gap * (rows - 1)) / rows, 52.0}));
    const qreal gridW = cell * cols + gap * (cols - 1);
    const qreal gridH = cell * rows + gap * (rows - 1);
    const qreal x0 = leftW + (areaW - gridW) / 2.0 + 6.0;
    const qreal y0 = (h - gridH) / 2.0;
    QList<QRectF> rects;
    for (int i = 0; i < n; ++i)
        rects.append(QRectF(x0 + (i % cols) * (cell + gap), y0 + (i / cols) * (cell + gap), cell, cell));
    return rects;
}

QPointF ChameleonColorPickerWidget::mouth(const QPointF &c, qreal s) const
{
    return QPointF(c.x() + s * 0.66, c.y() - s * 0.02);
}

QPointF ChameleonColorPickerWidget::eye(const QPointF &c, qreal s) const
{
    return QPointF(c.x() + s * 0.44, c.y() - s * 0.14);
}

void ChameleonColorPickerWidget::pick(int index)
{
    if (!(0 <= index && index < m_colors.size()))
        return;
    m_prevColor = bodyColorAt(1.0); // whatever it currently looks like
    m_target = index;
    m_pickAt = now();
    m_color = QColor(m_colors.at(index));
    emit colorChanged(QColor(m_color));
}

qreal ChameleonColorPickerWidget::wipe() const
{
    const qreal since = now() - m_pickAt;
    return clamp((since - WIPE_START) / (WIPE_END - WIPE_START), 0.0, 1.0);
}

QColor ChameleonColorPickerWidget::bodyColorAt(qreal pos) const
{
    return pos <= wipe() ? QColor(m_color) : QColor(m_prevColor);
}

// ---------- Public API ----------
QColor ChameleonColorPickerWidget::getColor() const
{
    return QColor(m_color);
}

void ChameleonColorPickerWidget::setColor(const QColor &color)
{
    if (color != m_color) {
        m_prevColor = QColor(m_color);
        m_color = QColor(color);
        m_target = -1;
        m_pickAt = now() - WIPE_START;
        emit colorChanged(QColor(color));
    }
}

QStringList ChameleonColorPickerWidget::getColors() const
{
    QStringList names;
    for (const QColor &c : m_colors)
        names.append(c.name());
    return names;
}

void ChameleonColorPickerWidget::setColors(const QStringList &colors)
{
    QList<QColor> list;
    for (const QString &c : colors)
        list.append(QColor(c));
    setColorList(list);
}

void ChameleonColorPickerWidget::setColorList(const QList<QColor> &colors)
{
    m_colors = colors.isEmpty() ? defaultColors() : colors;
    update();
}

int ChameleonColorPickerWidget::getColumns() const { return m_columns; }
bool ChameleonColorPickerWidget::isShowHex() const { return m_showHex; }
QColor ChameleonColorPickerWidget::getBranchColor() const { return m_branchColor; }

void ChameleonColorPickerWidget::setColumns(int columns)
{
    m_columns = clampInt(columns, 1, 8);
    update();
}

void ChameleonColorPickerWidget::setShowHex(bool showHex)
{
    m_showHex = showHex;
    update();
}

void ChameleonColorPickerWidget::setBranchColor(const QColor &color)
{
    m_branchColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize ChameleonColorPickerWidget::sizeHint() const
{
    return QSize(460, 220);
}

QSize ChameleonColorPickerWidget::minimumSizeHint() const
{
    return QSize(260, 130);
}

void ChameleonColorPickerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void ChameleonColorPickerWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void ChameleonColorPickerWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPointF c;
    qreal s;
    const QList<QRectF> rects = layoutSwatches(c, s);
    int hover = -1;
    for (int i = 0; i < rects.size(); ++i) {
        if (rects.at(i).contains(eventPos(e))) {
            hover = i;
            break;
        }
    }
    if (hover != m_hover) {
        m_hover = hover;
        setCursor(hover >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
    QWidget::mouseMoveEvent(e);
}

void ChameleonColorPickerWidget::leaveEvent(QEvent *e)
{
    m_hover = -1;
    QWidget::leaveEvent(e);
}

void ChameleonColorPickerWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QPointF c;
        qreal s;
        const QList<QRectF> rects = layoutSwatches(c, s);
        for (int i = 0; i < rects.size(); ++i) {
            if (rects.at(i).contains(eventPos(e))) {
                pick(i);
                break;
            }
        }
    }
    QWidget::mouseReleaseEvent(e);
}

void ChameleonColorPickerWidget::keyPressEvent(QKeyEvent *e)
{
    const int n = int(m_colors.size());
    int cur = -1;
    for (int i = 0; i < n; ++i) {
        if (m_colors.at(i) == m_color) {
            cur = i;
            break;
        }
    }
    int step = 0;
    bool known = true;
    switch (e->key()) {
    case Qt::Key_Right:
        step = 1;
        break;
    case Qt::Key_Left:
        step = -1;
        break;
    case Qt::Key_Down:
        step = m_columns;
        break;
    case Qt::Key_Up:
        step = -m_columns;
        break;
    default:
        known = false;
    }
    if (known)
        pick(cur >= 0 ? pyModInt(cur + step, n) : 0);
    else
        QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void ChameleonColorPickerWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    QPointF c;
    qreal s;
    const QList<QRectF> rects = layoutSwatches(c, s);
    const QPointF e = eye(c, s);
    const qreal since = t - m_pickAt;
    const int idx = since < TONGUE_IN ? m_target : m_hover;
    qreal ang = 0.0;
    if (0 <= idx && idx < rects.size()) {
        const QPointF tgt = rects.at(idx).center();
        ang = std::atan2(tgt.y() - e.y(), tgt.x() - e.x());
    }
    m_lookAng += (ang - m_lookAng) * std::min(1.0, dt * 10.0);
    if (t >= m_nextWander) {
        m_wanderTarget = uniform(-PI, PI);
        m_nextWander = t + uniform(0.8, 2.0);
    }
    m_wanderAng += (m_wanderTarget - m_wanderAng) * std::min(1.0, dt * 6.0);
    update();
}

// ---------- Painting ----------
void ChameleonColorPickerWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF c;
    qreal s;
    const QList<QRectF> rects = layoutSwatches(c, s);
    const qreal since = tNow - m_pickAt;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Swatches.
    for (int i = 0; i < rects.size(); ++i) {
        const QRectF &r = rects.at(i);
        const QColor col = m_colors.at(i);
        QRectF rr(r);
        if (i == m_target && TONGUE_OUT * 0.8 <= since && since < TONGUE_IN + 0.25) {
            // Bitten: shrinks, then pops back.
            const qreal k = clamp((since - TONGUE_IN) / 0.25, 0.0, 1.0);
            const qreal sc = 0.55 + 0.45 * k;
            rr = QRectF(r.center().x() - r.width() * sc / 2.0, r.center().y() - r.height() * sc / 2.0,
                        r.width() * sc, r.height() * sc);
        } else if (i == m_hover) {
            rr = r.adjusted(-2, -2, 2, 2);
        }
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 40));
        p.drawRoundedRect(rr.translated(0, 2), rr.width() * 0.22, rr.width() * 0.22);
        p.setBrush(col);
        p.drawRoundedRect(rr, rr.width() * 0.22, rr.width() * 0.22);
        if (col == m_color) {
            p.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), 2.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(rr.adjusted(3, 3, -3, -3), rr.width() * 0.16, rr.width() * 0.16);
        }
    }
    // Chameleon.
    drawChameleon(p, c, s, tNow);
    // Tongue.
    if (0 <= m_target && m_target < rects.size() && since < TONGUE_IN) {
        const QPointF m = mouth(c, s);
        const QPointF tgt = rects.at(m_target).center();
        qreal ext;
        if (since < TONGUE_OUT)
            ext = since / TONGUE_OUT;
        else if (since < TONGUE_HOLD)
            ext = 1.0;
        else
            ext = 1.0 - (since - TONGUE_HOLD) / (TONGUE_IN - TONGUE_HOLD);
        const QPointF tip(m.x() + (tgt.x() - m.x()) * ext, m.y() + (tgt.y() - m.y()) * ext);
        p.setPen(QPen(QColor(QStringLiteral("#FF7A90")), std::max(2.5, s * 0.035), Qt::SolidLine, Qt::RoundCap));
        p.drawLine(m, tip);
        p.setPen(QPen(QColor(QStringLiteral("#C2405A")), 1.2));
        p.setBrush(since >= TONGUE_OUT ? m_colors.at(m_target) : QColor(QStringLiteral("#FF7A90")));
        p.drawEllipse(tip, s * 0.05, s * 0.05);
    }
    // Hex label.
    if (m_showHex) {
        QFont f(QStringLiteral("Monospace"));
        f.setStyleHint(QFont::TypeWriter);
        f.setBold(true);
        f.setPointSizeF(10.0);
        p.setFont(f);
        const QColor tc(palette().text().color());
        const QString hx = m_color.name().toUpper();
        const QRectF box(c.x() - s * 0.55, height() - 24.0, s * 1.1, 20.0);
        p.setPen(Qt::NoPen);
        p.setBrush(m_color);
        p.drawEllipse(QPointF(box.left() + 10.0, box.center().y()), 6.0, 6.0);
        p.setPen(tc);
        p.drawText(box.adjusted(22, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, hx);
    }
    p.end();
}

void ChameleonColorPickerWidget::drawChameleon(QPainter &p, const QPointF &c, qreal s, qreal now)
{
    const qreal wp = wipe();
    const QColor nw(m_color);
    const QColor old(m_prevColor);
    // Gradient along x: snout (right) is position 0, tail (left) is position 1.
    QLinearGradient grad(QPointF(c.x() + s * 0.7, 0), QPointF(c.x() - s * 0.95, 0));
    grad.setColorAt(0.0, nw);
    if (0.0 < wp && wp < 1.0) {
        grad.setColorAt(std::max(0.0, wp - 0.02), nw);
        grad.setColorAt(std::min(1.0, wp + 0.02), old);
        grad.setColorAt(1.0, old);
    } else {
        grad.setColorAt(1.0, wp >= 1.0 ? nw : old);
    }
    const QBrush brush(grad);
    const QColor outlineC = (wp >= 0.5 ? nw : old).darker(150);
    const QPen outline(outlineC, std::max(1.2, s * 0.018));
    const qreal bob = std::sin(now * 1.6) * s * 0.01;
    // Branch.
    const QColor br = m_branchColor;
    const qreal by = c.y() + s * 0.3;
    p.setPen(QPen(br, std::max(4.0, s * 0.06), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(c.x() - s * 0.95, by + s * 0.03), QPointF(c.x() + s * 0.8, by - s * 0.02));
    p.setPen(QPen(br, std::max(2.0, s * 0.03), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(c.x() + s * 0.45, by - s * 0.01), QPointF(c.x() + s * 0.62, by + s * 0.14));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#5FB37A")));
    QPainterPath leaf;
    const QPointF lp(c.x() + s * 0.62, by + s * 0.14);
    leaf.moveTo(lp);
    leaf.quadTo(lp.x() + s * 0.12, lp.y() + s * 0.02, lp.x() + s * 0.16, lp.y() + s * 0.12);
    leaf.quadTo(lp.x() + s * 0.02, lp.y() + s * 0.12, lp.x(), lp.y());
    p.drawPath(leaf);
    p.save();
    p.translate(0, bob);
    // Tail spiral (stroked with the gradient, getting thinner).
    const QPointF tc(c.x() - s * 0.66, c.y() + s * 0.12);
    QList<QPointF> pts;
    const int steps = 40;
    for (int k = 0; k <= steps; ++k) {
        const qreal t = k / qreal(steps);
        const qreal ang = -PI * 0.15 - t * PI * 2.6;
        const qreal rad = s * 0.2 * (1.0 - t * 0.78);
        pts.append(QPointF(tc.x() + std::cos(ang) * rad, tc.y() + std::sin(ang) * rad));
    }
    pts.prepend(QPointF(c.x() - s * 0.45, c.y() + s * 0.02));
    for (int k = 0; k < pts.size() - 1; ++k) {
        const qreal wdt = s * 0.11 * (1.0 - k / qreal(pts.size())) + s * 0.015;
        p.setPen(QPen(brush, wdt, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(pts.at(k), pts.at(k + 1));
    }
    // Legs.
    p.setPen(QPen(brush, std::max(3.0, s * 0.05), Qt::SolidLine, Qt::RoundCap));
    static const qreal legs[2][2] = {{-0.25, -0.32}, {0.22, 0.3}};
    for (const auto &leg : legs)
        p.drawLine(QPointF(c.x() + leg[0] * s, c.y() + s * 0.12), QPointF(c.x() + leg[1] * s, by - s * 0.01));
    // Body.
    QPainterPath body;
    body.moveTo(c.x() - s * 0.5, c.y() + s * 0.02);
    body.cubicTo(c.x() - s * 0.42, c.y() - s * 0.3, c.x() + s * 0.12, c.y() - s * 0.36, c.x() + s * 0.36, c.y() - s * 0.2);
    body.lineTo(c.x() + s * 0.4, c.y() + s * 0.1);
    body.cubicTo(c.x() + s * 0.15, c.y() + s * 0.24, c.x() - s * 0.3, c.y() + s * 0.24, c.x() - s * 0.5, c.y() + s * 0.02);
    p.setPen(outline);
    p.setBrush(brush);
    p.drawPath(body);
    // Back crest + stripes.
    p.setPen(QPen(outlineC, std::max(1.0, s * 0.012)));
    for (int k = 0; k < 5; ++k) {
        const qreal x = c.x() - s * 0.32 + k * s * 0.14;
        p.drawLine(QPointF(x, c.y() - s * 0.16), QPointF(x + s * 0.04, c.y() + s * 0.12));
    }
    // Head with casque.
    QPainterPath head;
    head.moveTo(c.x() + s * 0.3, c.y() - s * 0.22);
    head.lineTo(c.x() + s * 0.4, c.y() - s * 0.38);
    head.quadTo(c.x() + s * 0.62, c.y() - s * 0.18, c.x() + s * 0.7, c.y() - s * 0.02);
    head.quadTo(c.x() + s * 0.6, c.y() + s * 0.12, c.x() + s * 0.38, c.y() + s * 0.13);
    head.closeSubpath();
    p.setPen(outline);
    p.setBrush(brush);
    p.drawPath(head);
    // Mouth line.
    p.setPen(QPen(outlineC, std::max(1.0, s * 0.015), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(c.x() + s * 0.66, c.y() - s * 0.02), QPointF(c.x() + s * 0.48, c.y() + s * 0.04));
    // Turret eyes: near eye tracks, far eye (peeking over the head) wanders.
    const QPointF ne = eye(c, s);
    const QPointF farEye(ne.x() + s * 0.06, ne.y() - s * 0.13);
    const struct { QPointF pos; qreal ang; qreal r; } eyes[2] = {{farEye, m_wanderAng, s * 0.06}, {ne, m_lookAng, s * 0.1}};
    for (const auto &ey : eyes) {
        const QPointF &pos = ey.pos;
        const qreal ang = ey.ang;
        const qreal r = ey.r;
        p.setPen(outline);
        p.setBrush(brush);
        p.drawEllipse(pos, r, r);
        p.setPen(QPen(outlineC, std::max(0.8, s * 0.008)));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(pos, r * 0.7, r * 0.7);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(QStringLiteral("#1E1E24")));
        p.drawEllipse(QPointF(pos.x() + std::cos(ang) * r * 0.45, pos.y() + std::sin(ang) * r * 0.45), r * 0.3, r * 0.3);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(pos.x() + std::cos(ang) * r * 0.45 - r * 0.1, pos.y() + std::sin(ang) * r * 0.45 - r * 0.1),
                      r * 0.09, r * 0.09);
    }
    p.restore();
}
