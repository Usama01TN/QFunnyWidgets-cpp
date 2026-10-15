/**
 * StarRatingWidget: a star rating where every star has a face. Stars bounce when hovered, do a wave when the
 * rating changes, cry for a 1-star rating, dance and sparkle for a perfect score, and empty stars look sad.
 */
#include "ratingstars.h"

#include <QKeyEvent>
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

} // namespace

QStringList StarRatingWidget::defaultLabels()
{
    return {QStringLiteral("Awful"), QStringLiteral("Not great"), QStringLiteral("Okay"), QStringLiteral("Great"),
            QStringLiteral("Life-changing!")};
}

QPainterPath StarRatingWidget::starPath(const QPointF &c, qreal r, qreal inner, int points)
{
    QPainterPath path;
    for (int j = 0; j < points * 2; ++j) {
        const qreal rr = j % 2 == 0 ? r : r * inner;
        const qreal a = -PI / 2.0 + j * PI / points;
        const QPointF pt(c.x() + std::cos(a) * rr, c.y() + std::sin(a) * rr);
        if (j == 0)
            path.moveTo(pt);
        else
            path.lineTo(pt);
    }
    path.closeSubpath();
    return path;
}

StarRatingWidget::StarRatingWidget(QWidget *parent)
    : StarRatingWidget(Options(), parent)
{
}

StarRatingWidget::StarRatingWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_maxRating(clampInt(options.maxRating, 3, 10)),
      m_rating(clampInt(options.rating, 0, m_maxRating)),
      m_starColor(options.starColor),
      m_emptyColor(options.emptyColor),
      m_labels(options.labels.isEmpty() ? defaultLabels() : options.labels),
      m_placeholder(options.placeholder),
      m_showLabel(options.showLabel),
      m_allowClear(options.allowClear)
{
    if (options.ratingChanged)
        connect(this, &StarRatingWidget::ratingChanged, this, options.ratingChanged);
    if (options.hovered)
        connect(this, &StarRatingWidget::hovered, this, options.hovered);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    // State.
    for (int i = 0; i < m_maxRating; ++i)
        m_stars.append(StarState());
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &StarRatingWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal StarRatingWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

int StarRatingWidget::shown() const
{
    return m_hover > 0 ? m_hover : m_rating;
}

QList<QPointF> StarRatingWidget::layoutStars(qreal &r) const
{
    const int n = m_maxRating;
    const qreal w = width();
    const qreal h = height();
    const qreal labelH = m_showLabel ? 30.0 : 0.0;
    r = std::max(6.0, std::min(w / (n * 2.35), (h - labelH - 16.0) / 2.3));
    const qreal gap = r * 2.35;
    const qreal x0 = (w - gap * (n - 1)) / 2.0;
    const qreal cy = (h - labelH) / 2.0 + r * 0.08;
    QList<QPointF> centres;
    for (int i = 0; i < n; ++i)
        centres.append(QPointF(x0 + i * gap, cy));
    return centres;
}

int StarRatingWidget::indexAt(const QPointF &pos) const
{
    qreal r;
    const QList<QPointF> centres = layoutStars(r);
    for (int i = 0; i < centres.size(); ++i) {
        const QPointF &c = centres.at(i);
        if (std::abs(pos.x() - c.x()) <= r * 1.17 && std::abs(pos.y() - c.y()) <= r * 1.3)
            return i + 1;
    }
    return 0;
}

QString StarRatingWidget::labelFor(int value) const
{
    if (value <= 0)
        return m_placeholder;
    const int n = m_labels.size();
    const int idx = int(std::nearbyint((value - 1) * (n - 1) / qreal(std::max(1, m_maxRating - 1))));
    return m_labels.at(clampInt(idx, 0, n - 1));
}

void StarRatingWidget::wave()
{
    const qreal t = now();
    for (int i = 0; i < m_stars.size(); ++i)
        m_stars[i].kickAt = t + i * 0.06;
}

// ---------- Public API ----------
int StarRatingWidget::getRating() const
{
    return m_rating;
}

void StarRatingWidget::setRating(int rating)
{
    rating = clampInt(rating, 0, m_maxRating);
    if (rating != m_rating) {
        m_rating = rating;
        wave();
        emit ratingChanged(rating);
        update();
    }
}

int StarRatingWidget::getMaxRating() const
{
    return m_maxRating;
}

void StarRatingWidget::setMaxRating(int maxRating)
{
    maxRating = clampInt(maxRating, 3, 10);
    if (maxRating != m_maxRating) {
        m_maxRating = maxRating;
        m_stars.clear();
        for (int i = 0; i < maxRating; ++i)
            m_stars.append(StarState());
        setRating(std::min(m_rating, maxRating));
        update();
    }
}

QColor StarRatingWidget::getStarColor() const
{
    return m_starColor;
}

void StarRatingWidget::setStarColor(const QColor &color)
{
    if (m_starColor != color) {
        m_starColor = color;
        update();
    }
}

QColor StarRatingWidget::getEmptyColor() const
{
    return m_emptyColor;
}

void StarRatingWidget::setEmptyColor(const QColor &color)
{
    if (m_emptyColor != color) {
        m_emptyColor = color;
        update();
    }
}

QStringList StarRatingWidget::getLabels() const
{
    return m_labels;
}

void StarRatingWidget::setLabels(const QStringList &labels)
{
    m_labels = labels.isEmpty() ? defaultLabels() : labels;
    update();
}

QString StarRatingWidget::getPlaceholder() const
{
    return m_placeholder;
}

void StarRatingWidget::setPlaceholder(const QString &placeholder)
{
    m_placeholder = placeholder;
    update();
}

bool StarRatingWidget::isShowLabel() const
{
    return m_showLabel;
}

void StarRatingWidget::setShowLabel(bool showLabel)
{
    m_showLabel = showLabel;
    update();
}

bool StarRatingWidget::isAllowClear() const
{
    return m_allowClear;
}

void StarRatingWidget::setAllowClear(bool allowClear)
{
    m_allowClear = allowClear;
}

// ---------- Qt overrides ----------
QSize StarRatingWidget::sizeHint() const
{
    return QSize(320, 110);
}

QSize StarRatingWidget::minimumSizeHint() const
{
    return QSize(150, 50);
}

void StarRatingWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void StarRatingWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void StarRatingWidget::mouseMoveEvent(QMouseEvent *e)
{
    const int idx = indexAt(eventPos(e));
    if (idx != m_hover) {
        m_hover = idx;
        if (idx > 0)
            m_stars[idx - 1].vel += 4.0;
        emit hovered(idx);
    }
    QWidget::mouseMoveEvent(e);
}

void StarRatingWidget::leaveEvent(QEvent *e)
{
    m_hover = 0;
    emit hovered(0);
    QWidget::leaveEvent(e);
}

void StarRatingWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        const int idx = indexAt(eventPos(e));
        if (idx > 0) {
            if (idx == m_rating && m_allowClear)
                setRating(0);
            else
                setRating(idx);
        }
    }
    QWidget::mouseReleaseEvent(e);
}

void StarRatingWidget::keyPressEvent(QKeyEvent *e)
{
    const int k = e->key();
    if (k == Qt::Key_Right || k == Qt::Key_Up)
        setRating(m_rating + 1);
    else if (k == Qt::Key_Left || k == Qt::Key_Down)
        setRating(m_rating - 1);
    else if (Qt::Key_0 <= k && k <= Qt::Key_9)
        setRating(k - Qt::Key_0);
    else
        QWidget::keyPressEvent(e);
}

// ---------- Animation tick ----------
void StarRatingWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    for (StarState &st : m_stars) {
        if (0 <= st.kickAt && st.kickAt <= t) {
            st.vel += 9.0;
            st.kickAt = -1.0;
        }
        st.vel += ((1.0 - st.scale) * 320.0 - st.vel * 13.0) * dt;
        st.scale += st.vel * dt;
    }
    // Tears for a 1-star rating, sparkles for a perfect score.
    const int s = shown();
    qreal r;
    const QList<QPointF> centres = layoutStars(r);
    if (s == 1 || s == m_maxRating) {
        m_spawn += dt * (s == 1 ? 6.0 : 9.0);
        while (m_spawn > 1.0) {
            m_spawn -= 1.0;
            if (s == 1) {
                const int i = int(uniform(1, m_maxRating));
                const QPointF c = centres.at(std::min(i, int(centres.size()) - 1));
                const qreal side = uniform(0, 1) < 0.5 ? -1.0 : 1.0;
                const qreal vx = side * uniform(5, 20);
                const qreal vy = uniform(5, 25);
                m_particles.append(Particle(c.x() + side * r * 0.24, c.y() + r * 0.02, vx, vy, 0.9, 0));
            } else {
                const QPointF c = centres.at(int(uniform(0, centres.size() - 0.01)));
                const qreal a = uniform(0, 2.0 * PI);
                m_particles.append(Particle(c.x() + std::cos(a) * r * 1.2, c.y() + std::sin(a) * r * 1.2,
                                            0, -uniform(10, 30), 0.7, 1));
            }
        }
    }
    QList<Particle> alive;
    for (Particle pt : std::as_const(m_particles)) {
        if (pt.kind == 0)
            pt.vy += 220.0 * dt;
        pt.x += pt.vx * dt;
        pt.y += pt.vy * dt;
        pt.life -= dt;
        if (pt.life > 0)
            alive.append(pt);
    }
    m_particles = alive;
    update();
}

// ---------- Painting ----------
void StarRatingWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    qreal r;
    const QList<QPointF> centres = layoutStars(r);
    const int s = shown();
    const int n = m_maxRating;
    const bool perfect = s == n;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    for (int i = 0; i < centres.size(); ++i) {
        const StarState &st = m_stars.at(i);
        const bool filled = i < s;
        p.save();
        p.translate(centres.at(i));
        // Dance when perfect, otherwise just the bounce.
        qreal hop = 0.0;
        if (perfect) {
            const qreal ph = tNow * 9.0 + i * 0.9;
            p.rotate(12.0 * std::sin(ph));
            hop = -std::abs(std::sin(ph)) * r * 0.22;
        }
        p.translate(0, hop);
        const qreal sc = std::max(0.3, st.scale);
        p.scale(sc, sc);
        drawStar(p, r, i, filled, s, tNow);
        p.restore();
    }
    // Particles.
    p.setPen(Qt::NoPen);
    for (const Particle &pt : std::as_const(m_particles)) {
        const qreal a = clamp(pt.life / pt.maxLife, 0.0, 1.0);
        if (pt.kind == 0) {
            QColor col(QStringLiteral("#5AB9FF"));
            col.setAlpha(int(230 * a));
            QPainterPath drop;
            const qreal ds = std::max(2.0, r * 0.09);
            drop.moveTo(pt.x, pt.y - ds * 1.6);
            drop.quadTo(pt.x + ds, pt.y, pt.x, pt.y + ds);
            drop.quadTo(pt.x - ds, pt.y, pt.x, pt.y - ds * 1.6);
            p.fillPath(drop, col);
        } else {
            QColor col(QStringLiteral("#FFF3B0"));
            col.setAlpha(int(255 * a));
            p.fillPath(starPath(QPointF(pt.x, pt.y), r * 0.16 * (0.5 + a), 0.3, 4), col);
        }
    }
    // Label.
    if (m_showLabel) {
        QFont f(font());
        f.setPointSizeF(10.5);
        f.setBold(s > 0);
        p.setFont(f);
        QColor tc(palette().text().color());
        if (s == 0)
            tc.setAlpha(150);
        p.setPen(tc);
        const qreal top = !centres.isEmpty() ? centres.first().y() + r * 1.25 : 0.0;
        p.drawText(QRectF(0, top, width(), 26), Qt::AlignHCenter | Qt::AlignTop, labelFor(s));
    }
    p.end();
}

void StarRatingWidget::drawStar(QPainter &p, qreal r, int i, bool filled, int shown, qreal now)
{
    const QColor base = filled ? m_starColor : m_emptyColor;
    const QPainterPath path = starPath(QPointF(0, 0), r, 0.52);
    QRadialGradient grad(QPointF(-r * 0.3, -r * 0.35), r * 1.6);
    grad.setColorAt(0.0, base.lighter(118));
    grad.setColorAt(1.0, base.darker(112));
    QPen pen(base.darker(135), std::max(1.5, r * 0.12));
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(grad);
    p.drawPath(path);
    const QColor ink = filled ? QColor(QStringLiteral("#3A2E1F")) : QColor(QStringLiteral("#6B6F7C"));
    const QPen fp(ink, std::max(1.0, r * 0.075), Qt::SolidLine, Qt::RoundCap);
    const qreal ex = r * 0.22;
    const qreal ey = -r * 0.04;
    const int n = m_maxRating;
    if (filled) {
        const bool perfect = shown == n;
        // Happy closed eyes when perfect, nervous dots for a 1-star score.
        if (perfect) {
            p.setPen(fp);
            p.setBrush(Qt::NoBrush);
            for (qreal d : {-1.0, 1.0})
                p.drawArc(QRectF(d * ex - r * 0.09, ey - r * 0.04, r * 0.18, r * 0.16), 0, 180 * 16);
        } else {
            p.setPen(Qt::NoPen);
            p.setBrush(ink);
            for (qreal d : {-1.0, 1.0})
                p.drawEllipse(QPointF(d * ex, ey), r * 0.065, r * 0.085);
        }
        const qreal mood = shown / qreal(n);
        p.setPen(fp);
        QPainterPath mouth;
        const qreal mw = r * (0.16 + 0.12 * mood);
        const qreal my = r * 0.17;
        mouth.moveTo(-mw, my);
        if (shown == 1) {
            // Awkward wavy smile.
            mouth.cubicTo(-mw * 0.3, my - r * 0.06, mw * 0.3, my + r * 0.06, mw, my);
            p.setBrush(Qt::NoBrush);
        } else {
            mouth.quadTo(0, my + r * (0.08 + 0.22 * mood), mw, my);
            if (perfect) {
                mouth.closeSubpath();
                p.setBrush(QColor(QStringLiteral("#C2412D")));
            } else {
                p.setBrush(Qt::NoBrush);
            }
        }
        p.drawPath(mouth);
        if (mood > 0.5) {
            QColor bc(QStringLiteral("#FF8A7A"));
            bc.setAlpha(140);
            p.setPen(Qt::NoPen);
            p.setBrush(bc);
            for (qreal d : {-1.0, 1.0})
                p.drawEllipse(QPointF(d * r * 0.36, r * 0.12), r * 0.08, r * 0.05);
        }
    } else {
        // Empty stars: hopeful when unrated, sad when left out.
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        const bool sad = shown > 0;
        for (qreal d : {-1.0, 1.0}) {
            if (sad) {
                p.setPen(fp);
                // Sad brows: inner ends raised.
                p.drawLine(QPointF(d * (ex + r * 0.09), ey - r * 0.12), QPointF(d * (ex - r * 0.07), ey - r * 0.19));
                p.setPen(Qt::NoPen);
            }
            p.drawEllipse(QPointF(d * ex, ey), r * 0.055, r * 0.07);
        }
        p.setPen(fp);
        p.setBrush(Qt::NoBrush);
        QPainterPath mouth;
        const qreal my = r * 0.2;
        if (sad) {
            const qreal depth = shown == 1 ? 0.12 : 0.07;
            mouth.moveTo(-r * 0.13, my + r * 0.04);
            mouth.quadTo(0, my - r * depth, r * 0.13, my + r * 0.04);
        } else {
            const qreal wob = std::sin(now * 3.0 + i) * r * 0.02;
            mouth.moveTo(-r * 0.1, my);
            mouth.quadTo(0, my + r * 0.06 + wob, r * 0.1, my);
        }
        p.drawPath(mouth);
    }
}
