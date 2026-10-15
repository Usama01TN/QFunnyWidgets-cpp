/**
 * PasswordJudgeWidget: a password field reviewed live by a powdered-wig judge (JudgeFaceWidget) with a monocle.
 * The judge's expression, comment and strength meter follow the password strength; he drops his monocle at
 * terrible passwords, follows the text cursor with his eyes and covers them when the password is revealed.
 */
#include "passwordjudge.h"

#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace {

inline qreal clamp(qreal x, qreal a, qreal b)
{
    return x < a ? a : (x > b ? b : x);
}

inline int clampInt(int x, int a, int b)
{
    return x < a ? a : (x > b ? b : x);
}

inline qreal lerp(qreal a, qreal b, qreal t)
{
    return a + (b - a) * t;
}

const char *const LEVEL_NAMES[] = {"Terrible", "Weak", "Fair", "Good", "Strong"};
const char *const LEVEL_COLORS[] = {"#E63946", "#F4845F", "#FFD23F", "#9BDB4D", "#3BCEAC"};

const QSet<QString> &commonPasswords()
{
    static const QSet<QString> set = {
        QStringLiteral("password"), QStringLiteral("password1"), QStringLiteral("123456"),
        QStringLiteral("12345678"), QStringLiteral("123456789"), QStringLiteral("1234"), QStringLiteral("12345"),
        QStringLiteral("qwerty"), QStringLiteral("qwerty123"), QStringLiteral("abc123"), QStringLiteral("111111"),
        QStringLiteral("letmein"), QStringLiteral("iloveyou"), QStringLiteral("admin"), QStringLiteral("welcome"),
        QStringLiteral("monkey"), QStringLiteral("dragon"), QStringLiteral("football"), QStringLiteral("sunshine"),
        QStringLiteral("princess"), QStringLiteral("passw0rd"), QStringLiteral("master")};
    return set;
}

/** The judge's comments per strength (-1..4). */
QStringList comments(int strength)
{
    switch (strength) {
    case 0:
        return {QStringLiteral("That's on every hacker's list. Page one."),
                QStringLiteral("My cat walked on the keyboard and did better."),
                QStringLiteral("I've seen stronger passwords on sticky notes.")};
    case 1:
        return {QStringLiteral("Weak. Like office coffee."), QStringLiteral("Add some numbers. Live a little."),
                QStringLiteral("Is that a password or a nap?")};
    case 2:
        return {QStringLiteral("Acceptable. Barely."), QStringLiteral("Getting there. Perhaps a symbol?"),
                QStringLiteral("Hmm. Mildly respectable.")};
    case 3:
        return {QStringLiteral("Ooh. Now we are talking."), QStringLiteral("Respectable. My monocle is intrigued.")};
    case 4:
        return {QStringLiteral("Magnificent. I'm genuinely impressed."), QStringLiteral("Bravo! Even I can't guess that."),
                QStringLiteral("Fort Knox called. It's jealous.")};
    default:
        return {QStringLiteral("I'm waiting. Impress me.")};
    }
}

QString choiceComment(int strength)
{
    const QStringList list = comments(strength);
    return list.at(int(QRandomGenerator::global()->bounded(int(list.size()))));
}

const QString REVEAL_COMMENT = QStringLiteral("I'm not looking! ...okay, I peeked.");

} // namespace

// ---------- passwordStrength ----------
int passwordStrength(const QString &text, int minLength)
{
    if (text.isEmpty())
        return -1;
    // Work on code points, like Python's str.
    const auto cps = text.toUcs4();
    const int len = int(cps.size());
    QSet<uint> distinct;
    for (auto ch : cps)
        distinct.insert(uint(ch));
    if (commonPasswords().contains(text.toLower()) || distinct.size() <= 2 || len < 4)
        return 0;
    bool lower = false, upper = false, digit = false, symbol = false;
    for (auto ch : cps) {
        const char32_t c = char32_t(ch);
        lower = lower || QChar::isLower(c);
        upper = upper || QChar::isUpper(c);
        digit = digit || QChar::isDigit(c);
        symbol = symbol || !QChar::isLetterOrNumber(c);
    }
    int pts = 0;
    pts += len >= minLength ? 1 : 0;
    pts += len >= minLength + 4 ? 1 : 0;
    pts += (lower && upper) ? 1 : 0;
    pts += digit ? 1 : 0;
    pts += symbol ? 1 : 0;
    if (len < minLength)
        return 1;
    if (pts <= 1)
        return 1;
    if (pts <= 3)
        return 2;
    return pts == 4 ? 3 : 4;
}

// ======================================================================================================
// JudgeFaceWidget
// ======================================================================================================
QString JudgeFaceWidget::defaultComment()
{
    return comments(-1).first();
}

JudgeFaceWidget::JudgeFaceWidget(QWidget *parent)
    : JudgeFaceWidget(Options(), parent)
{
}

JudgeFaceWidget::JudgeFaceWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_strength(clampInt(options.strength, -1, 4)),
      m_comment(options.comment),
      m_skinColor(options.skinColor),
      m_wigColor(options.wigColor)
{
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &JudgeFaceWidget::tick);
    m_timer->start(16);
}

// ---------- Public API ----------
int JudgeFaceWidget::getStrength() const
{
    return m_strength;
}

void JudgeFaceWidget::setStrength(int strength)
{
    strength = clampInt(strength, -1, 4);
    if (strength != m_strength) {
        m_strength = strength;
        m_changedAt = m_clock.elapsed() / 1000.0;
        update();
    }
}

QString JudgeFaceWidget::getComment() const
{
    return m_comment;
}

void JudgeFaceWidget::setComment(const QString &comment)
{
    m_comment = comment;
    update();
}

bool JudgeFaceWidget::isCovering() const
{
    return m_coverTarget > 0.5;
}

void JudgeFaceWidget::setCovering(bool covering)
{
    m_coverTarget = covering ? 1.0 : 0.0;
}

qreal JudgeFaceWidget::getLook() const
{
    return m_lookTarget;
}

void JudgeFaceWidget::setLook(qreal look)
{
    m_lookTarget = clamp(look, -1.0, 1.0);
}

QColor JudgeFaceWidget::getSkinColor() const
{
    return m_skinColor;
}

void JudgeFaceWidget::setSkinColor(const QColor &color)
{
    if (m_skinColor != color) {
        m_skinColor = color;
        update();
    }
}

QColor JudgeFaceWidget::getWigColor() const
{
    return m_wigColor;
}

void JudgeFaceWidget::setWigColor(const QColor &color)
{
    if (m_wigColor != color) {
        m_wigColor = color;
        update();
    }
}

// ---------- Qt overrides ----------
QSize JudgeFaceWidget::sizeHint() const
{
    return QSize(400, 150);
}

QSize JudgeFaceWidget::minimumSizeHint() const
{
    return QSize(240, 100);
}

void JudgeFaceWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void JudgeFaceWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

// ---------- Animation tick ----------
void JudgeFaceWidget::tick()
{
    const qreal now = m_clock.elapsed() / 1000.0;
    const qreal dt = std::min(0.05, std::max(0.0, now - m_last));
    m_last = now;
    m_cover += (m_coverTarget - m_cover) * std::min(1.0, dt * 9.0);
    m_lookX += (m_lookTarget - m_lookX) * std::min(1.0, dt * 12.0);
    const qreal monoTarget = m_strength == 0 ? 1.0 : 0.0;
    // Bouncy monocle on its chain.
    m_monoVel += ((monoTarget - m_mono) * 140.0 - m_monoVel * 6.0) * dt;
    m_mono += m_monoVel * dt;
    const qreal meterTarget = m_strength >= 0 ? (m_strength + 1) / 5.0 : 0.0;
    m_meter += (meterTarget - m_meter) * std::min(1.0, dt * 8.0);
    update();
}

// ---------- Painting ----------
void JudgeFaceWidget::paintEvent(QPaintEvent *)
{
    const qreal now = m_clock.elapsed() / 1000.0;
    const qreal w = width();
    const qreal h = height();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    const qreal R = std::min(h * 0.4, w * 0.18);
    const QPointF fc(R * 1.25, h / 2.0 + R * 0.05);
    // Disgust = head shake, delight = little hop.
    const qreal since = now - m_changedAt;
    p.save();
    if (m_strength == 0 && since < 0.8)
        p.translate(std::sin(since * 30.0) * R * 0.06 * (1.0 - since / 0.8), 0);
    else if (m_strength == 4 && since < 0.6)
        p.translate(0, -std::abs(std::sin(since * 12.0)) * R * 0.1);
    drawJudge(p, fc, R, now);
    p.restore();
    drawVerdict(p, QRectF(fc.x() + R * 1.45, 4.0, w - fc.x() - R * 1.45 - 4.0, h - 8.0), now);
    p.end();
}

void JudgeFaceWidget::drawJudge(QPainter &p, const QPointF &c, qreal R, qreal now)
{
    const int s = m_strength;
    const QColor wig = m_wigColor;
    const QColor skin = m_skinColor;
    const QPen wigPen(wig.darker(135), std::max(1.0, R * 0.03));
    // Wig rolls behind the head.
    p.setPen(wigPen);
    p.setBrush(wig);
    for (qreal d : {-1.0, 1.0}) {
        for (int k = 0; k < 3; ++k)
            p.drawEllipse(QPointF(c.x() + d * R * 0.92, c.y() - R * 0.2 + k * R * 0.42), R * 0.26, R * 0.22);
    }
    // Head.
    p.setPen(QPen(skin.darker(130), std::max(1.0, R * 0.03)));
    p.setBrush(skin);
    p.drawEllipse(c, R * 0.78, R * 0.9);
    // Wig cap.
    QPainterPath cap;
    cap.moveTo(c.x() - R * 0.95, c.y() - R * 0.15);
    cap.cubicTo(c.x() - R * 1.0, c.y() - R * 1.25, c.x() + R * 1.0, c.y() - R * 1.25, c.x() + R * 0.95, c.y() - R * 0.15);
    cap.cubicTo(c.x() + R * 0.6, c.y() - R * 0.55, c.x() - R * 0.6, c.y() - R * 0.55, c.x() - R * 0.95, c.y() - R * 0.15);
    p.setPen(wigPen);
    p.setBrush(wig);
    p.drawPath(cap);
    p.setBrush(Qt::NoBrush);
    for (int k = 0; k < 4; ++k) {
        const qreal x = c.x() - R * 0.55 + k * R * 0.37;
        p.drawArc(QRectF(x - R * 0.16, c.y() - R * 0.92, R * 0.32, R * 0.26), 20 * 16, 140 * 16);
    }
    const QColor ink(QStringLiteral("#2B2B33"));
    const qreal ex = R * 0.3;
    const qreal ey = c.y() - R * 0.08;
    const qreal er = R * (s == 4 ? 0.17 : 0.14);
    // Eyes.
    for (qreal d : {-1.0, 1.0}) {
        const qreal x = c.x() + d * ex;
        p.setPen(QPen(skin.darker(140), std::max(1.0, R * 0.02)));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(x, ey), er, er * 1.05);
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        p.drawEllipse(QPointF(x + m_lookX * er * 0.45, ey + er * 0.15), er * 0.5, er * 0.55);
        // Unimpressed half-lids.
        if (s == 0 || s == 1) {
            QPainterPath lid;
            lid.addRect(QRectF(x - er * 1.1, ey - er * 1.2, er * 2.2, er * (s == 1 ? 1.15 : 0.9)));
            QPainterPath eye;
            eye.addEllipse(QPointF(x, ey), er * 1.02, er * 1.07);
            p.fillPath(lid.intersected(eye), skin.darker(104));
        }
    }
    // Eyebrows.
    const QPen bp(wig.darker(160), std::max(1.5, R * 0.07), Qt::SolidLine, Qt::RoundCap);
    p.setPen(bp);
    for (qreal d : {-1.0, 1.0}) {
        const qreal x = c.x() + d * ex;
        const qreal by = ey - er * 1.5;
        if (s == 0) { // angry
            p.drawLine(QPointF(x + d * er, by - er * 0.4), QPointF(x - d * er * 0.9, by + er * 0.35));
        } else if (s == 1) { // flat, unimpressed
            p.drawLine(QPointF(x - er, by + er * 0.2), QPointF(x + er, by + er * 0.2));
        } else if (s == 2) { // one eyebrow raised (monocle side)
            const qreal lift = d > 0 ? er * 0.7 : 0.0;
            p.drawLine(QPointF(x - er, by - lift), QPointF(x + er, by - lift - (d > 0 ? er * 0.2 : 0)));
        } else if (s >= 3) { // both raised
            const qreal lift = er * (s == 3 ? 0.5 : 0.9);
            p.drawArc(QRectF(x - er, by - lift - er * 0.2, er * 2.0, er * 0.8), 20 * 16, 140 * 16);
        } else { // neutral
            p.drawLine(QPointF(x - er, by), QPointF(x + er, by - er * 0.1));
        }
    }
    // Nose.
    p.setPen(QPen(skin.darker(150), std::max(1.0, R * 0.04), Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(c.x() - R * 0.08, c.y() + R * 0.02, R * 0.16, R * 0.18), 200 * 16, 200 * 16);
    // Mouth (under the mustache).
    const qreal my = c.y() + R * 0.5;
    const QPen mp(QColor(QStringLiteral("#7A3A2E")), std::max(1.2, R * 0.05), Qt::SolidLine, Qt::RoundCap);
    p.setPen(mp);
    if (s == 4) {
        p.setBrush(QColor(QStringLiteral("#7A3A2E")));
        const qreal o = 1.0 + 0.1 * std::sin(now * 6.0);
        p.drawEllipse(QPointF(c.x(), my), R * 0.1 * o, R * 0.12 * o);
    } else if (s == 3) {
        p.drawArc(QRectF(c.x() - R * 0.18, my - R * 0.12, R * 0.36, R * 0.18), 200 * 16, 140 * 16);
    } else if (s == 0 || s == 1) {
        QPainterPath path;
        path.moveTo(c.x() - R * 0.18, my + R * 0.06);
        path.cubicTo(c.x() - R * 0.06, my - R * 0.08, c.x() + R * 0.06, my + R * 0.06, c.x() + R * 0.18, my - R * 0.02);
        p.drawPath(path);
    } else {
        p.drawLine(QPointF(c.x() - R * 0.14, my), QPointF(c.x() + R * 0.14, my + (s == 2 ? R * 0.03 : 0)));
    }
    // Handlebar mustache, twitching when annoyed.
    const qreal twitch = s == 0 ? std::sin(now * 25.0) * R * 0.02 : 0.0;
    const QColor mc = wig.darker(150);
    p.setPen(Qt::NoPen);
    p.setBrush(mc);
    for (qreal d : {-1.0, 1.0}) {
        QPainterPath m;
        m.moveTo(c.x(), c.y() + R * 0.3);
        m.cubicTo(c.x() + d * R * 0.2, c.y() + R * 0.22, c.x() + d * R * 0.42, c.y() + R * 0.42,
                  c.x() + d * R * 0.5, c.y() + R * 0.22 + twitch);
        m.cubicTo(c.x() + d * R * 0.52, c.y() + R * 0.3, c.x() + d * R * 0.38, c.y() + R * 0.44, c.x(), c.y() + R * 0.38);
        m.closeSubpath();
        p.drawPath(m);
    }
    // Blush when impressed.
    if (s >= 3) {
        QColor bc(QStringLiteral("#FF8FA3"));
        bc.setAlpha(110);
        p.setBrush(bc);
        for (qreal d : {-1.0, 1.0})
            p.drawEllipse(QPointF(c.x() + d * R * 0.52, c.y() + R * 0.18), R * 0.12, R * 0.07);
    }
    // Monocle on the right eye, hanging from a chain when dropped.
    const QColor gold(QStringLiteral("#D4A72C"));
    const QPointF anchor(c.x() + R * 0.65, c.y() + R * 0.95);
    const QPointF mono(c.x() + ex + m_mono * R * 0.25, ey + m_mono * R * 0.95);
    QPainterPath chain;
    chain.moveTo(mono.x() + R * 0.12, mono.y() + R * 0.14);
    chain.quadTo(QPointF(anchor.x() + R * 0.15, (mono.y() + anchor.y()) / 2.0 + R * 0.3), anchor);
    p.setPen(QPen(gold, std::max(1.0, R * 0.025)));
    p.setBrush(Qt::NoBrush);
    p.drawPath(chain);
    const QColor glass(200, 230, 255, 70);
    p.setPen(QPen(gold, std::max(1.5, R * 0.05)));
    p.setBrush(glass);
    p.drawEllipse(mono, R * 0.22, R * 0.22);
    p.setPen(QPen(QColor(255, 255, 255, 180), std::max(1.0, R * 0.025)));
    p.drawArc(QRectF(mono.x() - R * 0.14, mono.y() - R * 0.14, R * 0.28, R * 0.28), 100 * 16, 60 * 16);
    // Hands covering the eyes.
    const qreal cv = m_cover;
    if (cv > 0.01) {
        p.setPen(QPen(skin.darker(135), std::max(1.0, R * 0.03)));
        p.setBrush(skin);
        for (qreal d : {-1.0, 1.0}) {
            const qreal peek = d > 0 ? R * 0.18 * std::pow(std::max(0.0, std::sin(now * 1.3)), 8) : 0.0;
            const qreal hy = lerp(c.y() + R * 1.8, ey + peek, cv);
            const qreal hx = c.x() + d * ex;
            const QRectF hand(hx - R * 0.3, hy - R * 0.22, R * 0.6, R * 0.6);
            p.drawRoundedRect(hand, R * 0.18, R * 0.18);
            for (int k = 1; k < 4; ++k) {
                const qreal fx = hand.left() + hand.width() * k / 4.0;
                p.drawLine(QPointF(fx, hand.top() + R * 0.04), QPointF(fx, hand.top() + R * 0.22));
            }
        }
    }
}

void JudgeFaceWidget::drawVerdict(QPainter &p, const QRectF &area, qreal now)
{
    if (area.width() < 60)
        return;
    const int s = m_strength;
    QFont f(font());
    f.setPointSizeF(clamp(area.height() * 0.09, 8.5, 11.0));
    f.setItalic(true);
    const QFontMetricsF fm(f);
    const qreal meterH = 34.0;
    const qreal maxW = area.width() - 14.0;
    const QRectF br = fm.boundingRect(QRectF(0, 0, maxW - 24.0, 1000.0), Qt::TextWordWrap, m_comment);
    const qreal bw = std::min(maxW, br.width() + 26.0);
    const qreal bh = br.height() + 18.0;
    const qreal bx = area.left() + 12.0;
    const qreal by = area.top() + std::max(0.0, (area.height() - meterH - bh) / 2.0);
    const qreal since = now - m_changedAt;
    const qreal pop = 1.0 + 0.06 * std::max(0.0, 1.0 - since * 4.0);
    p.save();
    p.translate(bx, by + bh / 2.0);
    p.scale(pop, pop);
    p.translate(-bx, -(by + bh / 2.0));
    QPainterPath bubble;
    bubble.addRoundedRect(QRectF(bx, by, bw, bh), 12, 12);
    QPainterPath tail;
    tail.moveTo(bx + 2.0, by + bh * 0.4);
    tail.lineTo(bx - 10.0, by + bh * 0.55);
    tail.lineTo(bx + 2.0, by + bh * 0.7);
    tail.closeSubpath();
    bubble = bubble.united(tail);
    p.setPen(QPen(QColor(20, 20, 30, 60), 1.2));
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    p.drawPath(bubble);
    p.setFont(f);
    p.setPen(QColor(QStringLiteral("#22222A")));
    p.drawText(QRectF(bx + 13.0, by + 9.0, bw - 26.0, bh - 18.0), Qt::TextWordWrap | Qt::AlignVCenter, m_comment);
    p.restore();
    // Strength meter.
    const qreal my = by + bh + 12.0;
    const qreal segW = (std::min(area.width() - 12.0, 260.0) - 4 * 5.0) / 5.0;
    const qreal lit = m_meter * 5.0;
    const QColor col = s >= 0 ? QColor(QString::fromLatin1(LEVEL_COLORS[s])) : QColor(127, 127, 127);
    p.setPen(Qt::NoPen);
    for (int k = 0; k < 5; ++k) {
        const QRectF r(bx + k * (segW + 5.0), my, segW, 6.0);
        p.setBrush(QColor(127, 127, 127, 70));
        p.drawRoundedRect(r, 3, 3);
        const qreal amt = clamp(lit - k, 0.0, 1.0);
        if (amt > 0) {
            p.setBrush(col);
            p.drawRoundedRect(QRectF(r.left(), r.top(), r.width() * amt, r.height()), 3, 3);
        }
    }
    if (s >= 0) {
        QFont lf(font());
        lf.setBold(true);
        lf.setPointSizeF(8.5);
        p.setFont(lf);
        p.setPen(col);
        p.drawText(QRectF(bx, my + 9.0, 200.0, 16.0), Qt::AlignLeft | Qt::AlignVCenter, QString::fromLatin1(LEVEL_NAMES[s]));
    }
}

// ======================================================================================================
// PasswordJudgeWidget
// ======================================================================================================
PasswordJudgeWidget::PasswordJudgeWidget(QWidget *parent)
    : PasswordJudgeWidget(Options(), parent)
{
}

PasswordJudgeWidget::PasswordJudgeWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_minLength(std::max(1, options.minLength))
{
    if (options.strengthChanged)
        connect(this, &PasswordJudgeWidget::strengthChanged, this, options.strengthChanged);
    if (options.textChanged)
        connect(this, &PasswordJudgeWidget::textChanged, this, options.textChanged);
    if (options.returnPressed)
        connect(this, &PasswordJudgeWidget::returnPressed, this, options.returnPressed);
    // Children.
    m_face = new JudgeFaceWidget();
    m_edit = new QLineEdit();
    m_edit->setObjectName(QStringLiteral("passwordJudgeEdit"));
    m_edit->setEchoMode(QLineEdit::Password);
    m_edit->setPlaceholderText(options.placeholder);
    m_edit->setMinimumHeight(34);
    m_toggle = new QPushButton(tr("Show"));
    m_toggle->setObjectName(QStringLiteral("passwordJudgeToggle"));
    m_toggle->setCheckable(true);
    m_toggle->setCursor(Qt::PointingHandCursor);
    m_toggle->setMinimumHeight(34);
    m_toggle->setVisible(options.showToggle);
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(8);
    row->addWidget(m_edit, 1);
    row->addWidget(m_toggle);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->addWidget(m_face, 1);
    layout->addLayout(row);
    // Connections.
    connect(m_edit, &QLineEdit::textChanged, this, &PasswordJudgeWidget::onText);
    connect(m_edit, &QLineEdit::cursorPositionChanged, this, &PasswordJudgeWidget::onCursor);
    connect(m_edit, &QLineEdit::returnPressed, this, &PasswordJudgeWidget::returnPressed);
    connect(m_toggle, &QPushButton::toggled, this, &PasswordJudgeWidget::setRevealed);
    m_edit->setText(options.text);
    setRevealed(options.revealed);
}

// ---------- Helpers ----------
QString PasswordJudgeWidget::comment() const
{
    if (isRevealed() && !m_edit->text().isEmpty())
        return REVEAL_COMMENT;
    return choiceComment(m_strength);
}

void PasswordJudgeWidget::onText(const QString &text)
{
    const int strength = passwordStrength(text, m_minLength);
    if (strength != m_strength) {
        m_strength = strength;
        m_face->setStrength(strength);
        m_face->setComment(comment());
        emit strengthChanged(strength);
    }
    m_face->setCovering(isRevealed() && !text.isEmpty());
    if (isRevealed() && !text.isEmpty())
        m_face->setComment(REVEAL_COMMENT);
    emit textChanged(text);
}

void PasswordJudgeWidget::onCursor(int, int)
{
    // QLineEdit::cursorRect() is protected in C++; the input-method query returns the same rectangle.
    const int x = m_edit->inputMethodQuery(Qt::ImCursorRectangle).toRect().center().x();
    m_face->setLook((x / qreal(std::max(1, m_edit->width()))) * 2.0 - 1.0);
}

// ---------- Public API ----------
int PasswordJudgeWidget::getStrength() const
{
    return m_strength;
}

JudgeFaceWidget *PasswordJudgeWidget::getFace() const
{
    return m_face;
}

QLineEdit *PasswordJudgeWidget::getLineEdit() const
{
    return m_edit;
}

QString PasswordJudgeWidget::getText() const
{
    return m_edit->text();
}

void PasswordJudgeWidget::setText(const QString &text)
{
    m_edit->setText(text);
}

QString PasswordJudgeWidget::getPlaceholder() const
{
    return m_edit->placeholderText();
}

void PasswordJudgeWidget::setPlaceholder(const QString &placeholder)
{
    m_edit->setPlaceholderText(placeholder);
}

int PasswordJudgeWidget::getMinLength() const
{
    return m_minLength;
}

void PasswordJudgeWidget::setMinLength(int minLength)
{
    m_minLength = std::max(1, minLength);
    m_strength = -2; // force re-evaluation
    onText(m_edit->text());
}

bool PasswordJudgeWidget::isShowToggle() const
{
    return !m_toggle->isHidden();
}

void PasswordJudgeWidget::setShowToggle(bool showToggle)
{
    m_toggle->setVisible(showToggle);
}

bool PasswordJudgeWidget::isRevealed() const
{
    return m_edit->echoMode() == QLineEdit::Normal;
}

void PasswordJudgeWidget::setRevealed(bool revealed)
{
    m_edit->setEchoMode(revealed ? QLineEdit::Normal : QLineEdit::Password);
    if (m_toggle->isChecked() != revealed)
        m_toggle->setChecked(revealed);
    m_toggle->setText(revealed ? tr("Hide") : tr("Show"));
    const bool has = !m_edit->text().isEmpty();
    m_face->setCovering(revealed && has);
    m_face->setComment(revealed && has ? REVEAL_COMMENT : choiceComment(std::max(-1, m_strength)));
}
