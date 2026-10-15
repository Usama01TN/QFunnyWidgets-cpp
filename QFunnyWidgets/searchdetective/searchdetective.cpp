/**
 * SearchDetectiveWidget: a debounced search field investigated by a pipe-smoking detective (DetectiveFaceWidget).
 * He waits while idle, studies your typing through his magnifying glass, investigates during the search, beams
 * when something is found and gets a personal rain cloud when nothing is.
 */
#include "searchdetective.h"

#include <QFontMetricsF>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QStringList>
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

inline int clampInt(int x, int a, int b)
{
    return x < a ? a : (x > b ? b : x);
}

inline qreal uniform(qreal a, qreal b)
{
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}

QString choice(const QStringList &list)
{
    return list.at(int(QRandomGenerator::global()->bounded(int(list.size()))));
}

} // namespace

// ======================================================================================================
// DetectiveFaceWidget
// ======================================================================================================
QStringList DetectiveFaceWidget::lines(int state)
{
    switch (state) {
    case IDLE:
        return {QStringLiteral("Who are we looking for?"), QStringLiteral("The game is afoot. Type something.")};
    case TYPING:
        return {QStringLiteral("Hmm... a clue."), QStringLiteral("Go on, I am listening."),
                QStringLiteral("Fascinating. Continue.")};
    case SEARCHING:
        return {QStringLiteral("Investigating..."), QStringLiteral("Examining the evidence..."),
                QStringLiteral("Interrogating the database...")};
    case NONE:
        return {QStringLiteral("The trail has gone cold."), QStringLiteral("No suspects. Curious."),
                QStringLiteral("Nothing. Not even fingerprints.")};
    default:
        return {};
    }
}

DetectiveFaceWidget::DetectiveFaceWidget(QWidget *parent)
    : DetectiveFaceWidget(Options(), parent)
{
}

DetectiveFaceWidget::DetectiveFaceWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_state(clampInt(options.state, 0, 4)),
      m_comment(options.comment),
      m_hatColor(options.hatColor),
      m_skinColor(options.skinColor)
{
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DetectiveFaceWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal DetectiveFaceWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void DetectiveFaceWidget::face(QPointF &c, qreal &R) const
{
    const qreal h = height();
    R = std::max(14.0, std::min(h * 0.36, width() * 0.16));
    c = QPointF(R * 1.6, h / 2.0 + R * 0.25);
}

// ---------- Public API ----------
int DetectiveFaceWidget::getState() const { return m_state; }
QString DetectiveFaceWidget::getComment() const { return m_comment; }
QColor DetectiveFaceWidget::getHatColor() const { return m_hatColor; }
QColor DetectiveFaceWidget::getSkinColor() const { return m_skinColor; }

void DetectiveFaceWidget::setState(int state)
{
    state = clampInt(state, 0, 4);
    if (state != m_state) {
        m_state = state;
        m_changedAt = now();
        update();
    }
}

void DetectiveFaceWidget::setComment(const QString &comment)
{
    m_comment = comment;
    update();
}

void DetectiveFaceWidget::setHatColor(const QColor &color)
{
    m_hatColor = color;
    update();
}

void DetectiveFaceWidget::setSkinColor(const QColor &color)
{
    m_skinColor = color;
    update();
}

// ---------- Qt overrides ----------
QSize DetectiveFaceWidget::sizeHint() const
{
    return QSize(400, 140);
}

QSize DetectiveFaceWidget::minimumSizeHint() const
{
    return QSize(220, 90);
}

void DetectiveFaceWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void DetectiveFaceWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void DetectiveFaceWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    QPointF c;
    qreal R;
    face(c, R);
    const int st = m_state;
    if (st == IDLE || st == TYPING || st == SEARCHING) {
        m_puffAcc += dt * (st == IDLE ? 1.2 : 2.5);
        while (m_puffAcc > 1.0) {
            m_puffAcc -= 1.0;
            const qreal vx = uniform(-3, 6);
            const qreal vy = -uniform(14, 24);
            const qreal life = uniform(1.4, 2.0);
            m_puffs.append(Puff(c.x() - R * 1.0, c.y() + R * 0.28, vx, vy, life, 0));
        }
    } else if (st == NONE) {
        m_puffAcc += dt * 18.0;
        while (m_puffAcc > 1.0) {
            m_puffAcc -= 1.0;
            const qreal x = c.x() + uniform(-R * 0.6, R * 0.6);
            const qreal vy = uniform(90, 130);
            const qreal life = uniform(0.25, 0.4);
            m_puffs.append(Puff(x, c.y() - R * 1.35, 0, vy, life, 1));
        }
    }
    QList<Puff> alive;
    for (Puff pf : std::as_const(m_puffs)) {
        pf.x += pf.vx * dt;
        pf.y += pf.vy * dt;
        pf.life -= dt;
        if (pf.life > 0)
            alive.append(pf);
    }
    m_puffs = alive;
    update();
}

// ---------- Painting ----------
void DetectiveFaceWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QPointF c;
    qreal R;
    face(c, R);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const int st = m_state;
    const qreal since = tNow - m_changedAt;
    const qreal hop = st == FOUND ? -std::abs(std::sin(since * 10.0)) * R * 0.12 * std::max(0.0, 1.0 - since / 0.8) : 0.0;
    p.save();
    p.translate(0, hop);
    drawDetective(p, c, R, st, tNow);
    p.restore();
    // Smoke and rain.
    p.setPen(Qt::NoPen);
    for (const Puff &pf : std::as_const(m_puffs)) {
        const qreal a = clamp(pf.life / pf.maxLife, 0.0, 1.0);
        if (pf.kind == 0) {
            QColor sc(200, 205, 215);
            sc.setAlpha(int(150 * a));
            p.setBrush(sc);
            const qreal rr = R * (0.06 + 0.12 * (1.0 - a));
            p.drawEllipse(QPointF(pf.x, pf.y), rr, rr);
        } else {
            QColor rc(QStringLiteral("#7FB8FF"));
            rc.setAlpha(int(220 * a));
            p.setPen(QPen(rc, std::max(1.0, R * 0.03), Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(pf.x, pf.y), QPointF(pf.x, pf.y + R * 0.08));
            p.setPen(Qt::NoPen);
        }
    }
    if (st == NONE) {
        // Personal rain cloud.
        p.setBrush(QColor(QStringLiteral("#9AA3B5")));
        const qreal cy = c.y() - R * 1.45;
        static const qreal blobs[3][2] = {{-0.35, 0.22}, {0.0, 0.3}, {0.35, 0.22}};
        for (const auto &b : blobs)
            p.drawEllipse(QPointF(c.x() + b[0] * R + std::sin(tNow) * R * 0.05, cy), b[1] * R, b[1] * R * 0.8);
    }
    drawBubble(p, QRectF(c.x() + R * 1.25, 4.0, width() - c.x() - R * 1.25 - 4.0, height() - 8.0), tNow);
    p.end();
}

void DetectiveFaceWidget::drawDetective(QPainter &p, const QPointF &c, qreal R, int st, qreal now)
{
    const QColor skin = m_skinColor;
    const QColor hat = m_hatColor;
    const QColor ink(QStringLiteral("#2B2B33"));
    // Head.
    p.setPen(QPen(skin.darker(130), std::max(1.0, R * 0.03)));
    p.setBrush(skin);
    p.drawEllipse(c, R * 0.8, R * 0.88);
    // Deerstalker: dome with check pattern, front and back bills, tied-up ear flaps.
    QPainterPath dome;
    dome.moveTo(c.x() - R * 0.85, c.y() - R * 0.25);
    dome.cubicTo(c.x() - R * 0.9, c.y() - R * 1.25, c.x() + R * 0.9, c.y() - R * 1.25, c.x() + R * 0.85, c.y() - R * 0.25);
    dome.quadTo(c.x(), c.y() - R * 0.45, c.x() - R * 0.85, c.y() - R * 0.25);
    p.setPen(QPen(hat.darker(140), std::max(1.0, R * 0.03)));
    p.setBrush(hat);
    p.drawPath(dome);
    p.save();
    p.setClipPath(dome);
    p.setPen(QPen(hat.darker(125), std::max(1.0, R * 0.025)));
    const qreal step = R * 0.18;
    qreal x = c.x() - R;
    while (x < c.x() + R) {
        p.drawLine(QPointF(x, c.y() - R * 1.3), QPointF(x, c.y()));
        x += step;
    }
    qreal y = c.y() - R * 1.2;
    while (y < c.y()) {
        p.drawLine(QPointF(c.x() - R, y), QPointF(c.x() + R, y));
        y += step;
    }
    p.restore();
    p.setPen(QPen(hat.darker(140), std::max(1.0, R * 0.03)));
    p.setBrush(hat.darker(110));
    p.drawEllipse(QPointF(c.x() + R * 0.92, c.y() - R * 0.3), R * 0.3, R * 0.1);  // front bill
    p.drawEllipse(QPointF(c.x() - R * 0.92, c.y() - R * 0.3), R * 0.25, R * 0.09); // back bill
    p.setBrush(hat.lighter(110));
    p.drawEllipse(QPointF(c.x(), c.y() - R * 1.05), R * 0.1, R * 0.07); // bow on top
    // Pipe (left side of the mouth).
    p.setPen(QPen(QColor(QStringLiteral("#4A2F1E")), std::max(1.5, R * 0.06), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(c.x() - R * 0.2, c.y() + R * 0.42), QPointF(c.x() - R * 0.85, c.y() + R * 0.5));
    p.setPen(QPen(QColor(QStringLiteral("#3A2414")), std::max(1.0, R * 0.03)));
    p.setBrush(QColor(QStringLiteral("#6B4226")));
    p.drawRoundedRect(QRectF(c.x() - R * 1.13, c.y() + R * 0.3, R * 0.26, R * 0.3), R * 0.06, R * 0.06);
    // Left eye + brow.
    const qreal ex = c.x() - R * 0.3;
    const qreal ey = c.y() - R * 0.08;
    p.setPen(Qt::NoPen);
    p.setBrush(ink);
    if (st == FOUND) {
        p.setPen(QPen(ink, std::max(1.2, R * 0.05), Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(ex - R * 0.1, ey - R * 0.05, R * 0.2, R * 0.14), 20 * 16, 140 * 16);
    } else {
        p.drawEllipse(QPointF(ex, ey), R * 0.06, R * (st == SEARCHING ? 0.03 : 0.07));
    }
    const QPen bp(QColor(QStringLiteral("#5A4030")), std::max(1.2, R * 0.05), Qt::SolidLine, Qt::RoundCap);
    p.setPen(bp);
    if (st == SEARCHING) {
        p.drawLine(QPointF(ex - R * 0.14, ey - R * 0.18), QPointF(ex + R * 0.12, ey - R * 0.12));
    } else if (st == NONE) {
        p.drawLine(QPointF(ex - R * 0.14, ey - R * 0.14), QPointF(ex + R * 0.12, ey - R * 0.2));
    } else {
        const qreal lift = st == FOUND ? R * 0.08 : 0.0;
        p.drawLine(QPointF(ex - R * 0.14, ey - R * 0.17 - lift), QPointF(ex + R * 0.12, ey - R * 0.19 - lift));
    }
    // Mouth.
    const QPen mp(QColor(QStringLiteral("#7A3A2E")), std::max(1.2, R * 0.05), Qt::SolidLine, Qt::RoundCap);
    p.setPen(mp);
    p.setBrush(Qt::NoBrush);
    const qreal mx = c.x() + R * 0.02;
    const qreal my = c.y() + R * 0.42;
    if (st == FOUND) {
        p.setBrush(QColor(QStringLiteral("#7A3A2E")));
        QPainterPath m;
        m.moveTo(mx - R * 0.22, my - R * 0.06);
        m.quadTo(mx, my + R * 0.25, mx + R * 0.22, my - R * 0.06);
        m.closeSubpath();
        p.drawPath(m);
    } else if (st == NONE) {
        p.drawArc(QRectF(mx - R * 0.16, my, R * 0.32, R * 0.18), 30 * 16, 120 * 16);
    } else if (st == TYPING) {
        p.drawEllipse(QPointF(mx + R * 0.05, my), R * 0.05, R * 0.06);
    } else {
        p.drawLine(QPointF(mx - R * 0.14, my), QPointF(mx + R * 0.14, my));
    }
    // Magnifying glass over the right eye: sweeps while typing / searching.
    qreal sweep = 0.0;
    if (st == TYPING)
        sweep = std::sin(now * 4.0) * R * 0.12;
    else if (st == SEARCHING)
        sweep = std::sin(now * 7.0) * R * 0.3;
    const QPointF lc(c.x() + R * 0.35 + sweep, c.y() - R * 0.05 + (st == FOUND ? -R * 0.15 : 0.0));
    const qreal lr = R * 0.36;
    const QColor gold(QStringLiteral("#C9A227"));
    p.setPen(QPen(QColor(QStringLiteral("#3A2414")), std::max(2.0, R * 0.1), Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(lc.x() + lr * 0.65, lc.y() + lr * 0.65), QPointF(lc.x() + lr * 1.5, lc.y() + lr * 1.5));
    QPainterPath lens;
    lens.addEllipse(lc, lr, lr);
    p.save();
    p.setClipPath(lens);
    p.fillPath(lens, skin.lighter(104));
    // The magnified eye (it follows the lens, obviously).
    const qreal er = lr * 0.62;
    p.setPen(QPen(skin.darker(140), std::max(1.0, R * 0.02)));
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    p.drawEllipse(QPointF(lc.x(), lc.y()), er, er * (st == SEARCHING ? 0.55 : 0.9));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#4B6B8A")));
    const qreal look = st == SEARCHING ? std::sin(now * 7.0) * er * 0.3 : 0.0;
    p.drawEllipse(QPointF(lc.x() + look, lc.y()), er * 0.5, er * (st == SEARCHING ? 0.4 : 0.5));
    p.setBrush(ink);
    p.drawEllipse(QPointF(lc.x() + look, lc.y()), er * 0.25, er * 0.25);
    p.setBrush(QColor(255, 255, 255, 220));
    p.drawEllipse(QPointF(lc.x() + look - er * 0.18, lc.y() - er * 0.18), er * 0.1, er * 0.1);
    p.fillPath(lens, QColor(190, 225, 255, 50));
    p.restore();
    p.setPen(QPen(gold, std::max(2.0, R * 0.08)));
    p.setBrush(Qt::NoBrush);
    p.drawPath(lens);
    if (st == FOUND) {
        // Triumphant glint.
        const QColor gl(QStringLiteral("#FFF6C8"));
        const qreal k = 0.6 + 0.4 * std::sin(now * 8.0);
        p.setPen(QPen(gl, std::max(1.5, R * 0.04), Qt::SolidLine, Qt::RoundCap));
        const QPointF g0(lc.x() - lr * 0.7, lc.y() - lr * 0.75);
        for (int a = 0; a < 4; ++a) {
            const qreal ang = a * PI / 2.0;
            p.drawLine(g0, QPointF(g0.x() + std::cos(ang) * lr * 0.35 * k, g0.y() + std::sin(ang) * lr * 0.35 * k));
        }
    }
}

void DetectiveFaceWidget::drawBubble(QPainter &p, const QRectF &area, qreal now)
{
    if (area.width() < 60)
        return;
    QFont f(font());
    f.setPointSizeF(clamp(area.height() * 0.09, 9.0, 11.5));
    f.setItalic(true);
    const QFontMetricsF fm(f);
    QString text = m_comment;
    if (m_state == SEARCHING) {
        while (text.endsWith(QLatin1Char('.')))
            text.chop(1);
        text += QString(1 + int(now * 3.0) % 3, QLatin1Char('.'));
    }
    const qreal maxW = area.width() - 14.0;
    const QRectF br = fm.boundingRect(QRectF(0, 0, maxW - 26.0, 1000.0), Qt::TextWordWrap, m_comment + QStringLiteral(".."));
    const qreal bw = std::min(maxW, br.width() + 28.0);
    const qreal bh = br.height() + 18.0;
    const qreal bx = area.left() + 12.0;
    const qreal by = area.top() + (area.height() - bh) / 2.0;
    QPainterPath bubble;
    bubble.addRoundedRect(QRectF(bx, by, bw, bh), 12, 12);
    QPainterPath tail;
    tail.moveTo(bx + 2.0, by + bh * 0.4);
    tail.lineTo(bx - 10.0, by + bh * 0.6);
    tail.lineTo(bx + 2.0, by + bh * 0.72);
    tail.closeSubpath();
    p.setPen(QPen(QColor(20, 20, 30, 60), 1.2));
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    p.drawPath(bubble.united(tail));
    p.setFont(f);
    p.setPen(QColor(QStringLiteral("#22222A")));
    p.drawText(QRectF(bx + 14.0, by + 9.0, bw - 28.0, bh - 18.0), Qt::TextWordWrap | Qt::AlignVCenter, text);
}

// ======================================================================================================
// SearchDetectiveWidget
// ======================================================================================================
SearchDetectiveWidget::SearchDetectiveWidget(QWidget *parent)
    : SearchDetectiveWidget(Options(), parent)
{
}

SearchDetectiveWidget::SearchDetectiveWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_searchFunction(options.searchFunction)
{
    if (options.searchRequested)
        connect(this, &SearchDetectiveWidget::searchRequested, this, options.searchRequested);
    if (options.textChanged)
        connect(this, &SearchDetectiveWidget::textChanged, this, options.textChanged);
    if (options.returnPressed)
        connect(this, &SearchDetectiveWidget::returnPressed, this, options.returnPressed);
    // Children.
    m_face = new DetectiveFaceWidget();
    m_edit = new QLineEdit();
    m_edit->setObjectName(QStringLiteral("searchDetectiveEdit"));
    m_edit->setPlaceholderText(options.placeholder);
    m_edit->setClearButtonEnabled(true);
    m_edit->setMinimumHeight(34);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->addWidget(m_face, 1);
    layout->addWidget(m_edit);
    // Debounce + fake investigation delay.
    m_debounce = new QTimer(this);
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(std::max(0, options.debounce));
    connect(m_debounce, &QTimer::timeout, this, &SearchDetectiveWidget::fire);
    m_investigate = new QTimer(this);
    m_investigate->setSingleShot(true);
    m_investigate->setInterval(900);
    connect(m_investigate, &QTimer::timeout, this, &SearchDetectiveWidget::autoResult);
    // Connections.
    connect(m_edit, &QLineEdit::textChanged, this, &SearchDetectiveWidget::onText);
    connect(m_edit, &QLineEdit::returnPressed, this, &SearchDetectiveWidget::onReturn);
}

// ---------- Helpers ----------
void SearchDetectiveWidget::onText(const QString &text)
{
    emit textChanged(text);
    m_investigate->stop();
    if (text.trimmed().isEmpty()) {
        m_debounce->stop();
        m_face->setState(DetectiveFaceWidget::IDLE);
        m_face->setComment(choice(DetectiveFaceWidget::lines(DetectiveFaceWidget::IDLE)));
        return;
    }
    if (m_face->getState() != DetectiveFaceWidget::TYPING) {
        m_face->setState(DetectiveFaceWidget::TYPING);
        m_face->setComment(choice(DetectiveFaceWidget::lines(DetectiveFaceWidget::TYPING)));
    }
    m_debounce->start();
}

void SearchDetectiveWidget::onReturn()
{
    m_debounce->stop();
    fire();
    emit returnPressed();
}

void SearchDetectiveWidget::fire()
{
    const QString text = m_edit->text().trimmed();
    if (text.isEmpty())
        return;
    m_face->setState(DetectiveFaceWidget::SEARCHING);
    m_face->setComment(choice(DetectiveFaceWidget::lines(DetectiveFaceWidget::SEARCHING)));
    emit searchRequested(text);
    if (m_searchFunction)
        m_investigate->start();
}

void SearchDetectiveWidget::autoResult()
{
    int n = 0;
    try {
        n = m_searchFunction ? m_searchFunction(m_edit->text().trimmed()) : 0;
    } catch (...) {
        n = 0;
    }
    setResultCount(n);
}

// ---------- Public API ----------
void SearchDetectiveWidget::setResultCount(int count)
{
    count = std::max(0, count);
    if (m_edit->text().trimmed().isEmpty())
        return;
    if (count) {
        m_face->setState(DetectiveFaceWidget::FOUND);
        m_face->setComment(tr("Elementary! %1 result%2.").arg(count).arg(count == 1 ? QString() : QStringLiteral("s")));
    } else {
        m_face->setState(DetectiveFaceWidget::NONE);
        m_face->setComment(choice(DetectiveFaceWidget::lines(DetectiveFaceWidget::NONE)));
    }
}

DetectiveFaceWidget *SearchDetectiveWidget::getFace() const { return m_face; }
QLineEdit *SearchDetectiveWidget::getLineEdit() const { return m_edit; }
QString SearchDetectiveWidget::getText() const { return m_edit->text(); }
QString SearchDetectiveWidget::getPlaceholder() const { return m_edit->placeholderText(); }
int SearchDetectiveWidget::getDebounce() const { return m_debounce->interval(); }
SearchDetectiveWidget::SearchFunction SearchDetectiveWidget::getSearchFunction() const { return m_searchFunction; }

void SearchDetectiveWidget::setText(const QString &text)
{
    m_edit->setText(text);
}

void SearchDetectiveWidget::setPlaceholder(const QString &placeholder)
{
    m_edit->setPlaceholderText(placeholder);
}

void SearchDetectiveWidget::setDebounce(int debounce)
{
    m_debounce->setInterval(std::max(0, debounce));
}

void SearchDetectiveWidget::setSearchFunction(const SearchFunction &searchFunction)
{
    m_searchFunction = searchFunction;
}
