/**
 * DropZoneMonsterWidget: a file drop zone guarded by a hungry blob monster. It watches the cursor, opens its mouth
 * and drools when acceptable files are dragged over it, makes a disgusted face at unsupported files, chomps
 * dropped files (spitting paper bits) and burps. Click to browse.
 */
#include "dropzone.h"

#include <QCursor>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrl>
#include <algorithm>
#include <cmath>
#include <utility>

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

/** str.lstrip('.') */
QString lstripDots(const QString &s)
{
    int i = 0;
    while (i < s.size() && s.at(i) == QLatin1Char('.'))
        ++i;
    return s.mid(i);
}

/** os.path.splitext(path)[1]: the extension including the dot; leading dots of the file name are ignored. */
QString pyExtension(const QString &p)
{
#ifdef Q_OS_WIN
    const int sep = std::max(p.lastIndexOf(QLatin1Char('/')), p.lastIndexOf(QLatin1Char('\\')));
#else
    const int sep = p.lastIndexOf(QLatin1Char('/'));
#endif
    const int dot = p.lastIndexOf(QLatin1Char('.'));
    if (dot > sep) {
        for (int i = sep + 1; i < dot; ++i) {
            if (p.at(i) != QLatin1Char('.'))
                return p.mid(dot);
        }
    }
    return QString();
}

QStringList normaliseExtensions(const QStringList &exts)
{
    QStringList out;
    for (const QString &e : exts)
        out.append(lstripDots(e.toLower()));
    return out;
}

} // namespace

DropZoneMonsterWidget::DropZoneMonsterWidget(QWidget *parent)
    : DropZoneMonsterWidget(Options(), parent)
{
}

DropZoneMonsterWidget::DropZoneMonsterWidget(const Options &options, QWidget *parent)
    : QWidget(parent),
      m_exts(normaliseExtensions(options.acceptedExtensions)),
      m_hint(options.hint),
      m_subtitle(options.subtitle),
      m_monsterColor(options.monsterColor),
      m_accentColor(options.accentColor),
      m_clickToBrowse(options.clickToBrowse)
{
    if (options.filesDropped)
        connect(this, &DropZoneMonsterWidget::filesDropped, this, options.filesDropped);
    if (options.filesRejected)
        connect(this, &DropZoneMonsterWidget::filesRejected, this, options.filesRejected);
    // Widget flags.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAcceptDrops(true);
    setCursor(options.clickToBrowse ? Qt::PointingHandCursor : Qt::ArrowCursor);
    // Timing.
    m_clock.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DropZoneMonsterWidget::tick);
    m_timer->start(16);
}

// ---------- Helpers ----------
qreal DropZoneMonsterWidget::now() const
{
    return m_clock.elapsed() / 1000.0;
}

void DropZoneMonsterWidget::setMode(Mode mode)
{
    m_mode = mode;
    m_modeAt = now();
    update();
}

bool DropZoneMonsterWidget::accepts(const QString &path) const
{
    if (m_exts.isEmpty())
        return true;
    return m_exts.contains(lstripDots(pyExtension(path).toLower()));
}

void DropZoneMonsterWidget::split(const QMimeData *mime, QStringList &good, QStringList &bad) const
{
    QStringList paths;
    if (mime->hasUrls()) {
        for (const QUrl &u : mime->urls()) {
            const QString local = u.toLocalFile();
            paths.append(!local.isEmpty() ? local : u.toString());
        }
    }
    for (const QString &p : std::as_const(paths)) {
        if (accepts(p))
            good.append(p);
    }
    for (const QString &p : std::as_const(paths)) {
        if (!accepts(p))
            bad.append(p);
    }
}

void DropZoneMonsterWidget::geometry(QRectF &zone, QPointF &c, qreal &s) const
{
    zone = QRectF(rect()).adjusted(4, 4, -4, -4);
    s = std::max(30.0, std::min(zone.width() * 0.42, (zone.height() - 60.0) * 0.8));
    c = QPointF(zone.center().x(), zone.top() + 14.0 + s * 0.55);
}

// ---------- Public API ----------
void DropZoneMonsterWidget::feed(const QStringList &paths)
{
    QStringList good;
    QStringList bad;
    for (const QString &p : paths) {
        if (accepts(p))
            good.append(p);
        else
            bad.append(p);
    }
    if (!bad.isEmpty())
        emit filesRejected(bad);
    if (!good.isEmpty()) {
        m_eaten += int(good.size());
        m_lastBatch = int(good.size());
        setMode(EATING);
        emit filesDropped(good);
    } else if (!bad.isEmpty()) {
        setMode(REFUSED);
    }
}

void DropZoneMonsterWidget::browse()
{
    QString filt;
    if (!m_exts.isEmpty()) {
        QStringList globs;
        for (const QString &e : std::as_const(m_exts))
            globs.append(QStringLiteral("*.") + e);
        filt = QStringLiteral("Tasty files (%1)").arg(globs.join(QLatin1Char(' ')));
    } else {
        filt = QStringLiteral("All files (*)");
    }
    const QStringList paths = QFileDialog::getOpenFileNames(this, tr("Feed the monster"), QString(), filt);
    if (!paths.isEmpty())
        feed(paths);
}

int DropZoneMonsterWidget::getEaten() const { return m_eaten; }
QStringList DropZoneMonsterWidget::getAcceptedExtensions() const { return m_exts; }
QString DropZoneMonsterWidget::getHint() const { return m_hint; }
QString DropZoneMonsterWidget::getSubtitle() const { return m_subtitle; }
QColor DropZoneMonsterWidget::getMonsterColor() const { return m_monsterColor; }
QColor DropZoneMonsterWidget::getAccentColor() const { return m_accentColor; }
bool DropZoneMonsterWidget::isClickToBrowse() const { return m_clickToBrowse; }

void DropZoneMonsterWidget::setAcceptedExtensions(const QStringList &exts)
{
    m_exts = normaliseExtensions(exts);
}

void DropZoneMonsterWidget::setHint(const QString &hint)
{
    m_hint = hint;
    update();
}

void DropZoneMonsterWidget::setSubtitle(const QString &subtitle)
{
    m_subtitle = subtitle;
    update();
}

void DropZoneMonsterWidget::setMonsterColor(const QColor &color)
{
    if (m_monsterColor != color) {
        m_monsterColor = color;
        update();
    }
}

void DropZoneMonsterWidget::setAccentColor(const QColor &color)
{
    if (m_accentColor != color) {
        m_accentColor = color;
        update();
    }
}

void DropZoneMonsterWidget::setClickToBrowse(bool clickToBrowse)
{
    m_clickToBrowse = clickToBrowse;
    setCursor(clickToBrowse ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

// ---------- Qt overrides ----------
QSize DropZoneMonsterWidget::sizeHint() const
{
    return QSize(360, 300);
}

QSize DropZoneMonsterWidget::minimumSizeHint() const
{
    return QSize(200, 180);
}

void DropZoneMonsterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_timer->isActive())
        m_timer->start(16);
}

void DropZoneMonsterWidget::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void DropZoneMonsterWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        QStringList good;
        QStringList bad;
        split(e->mimeData(), good, bad);
        setMode(!good.isEmpty() ? HUNGRY : YUCK);
        e->acceptProposedAction();
    } else {
        e->ignore();
    }
}

void DropZoneMonsterWidget::dragLeaveEvent(QDragLeaveEvent *e)
{
    if (m_mode == HUNGRY || m_mode == YUCK)
        setMode(IDLE);
    QWidget::dragLeaveEvent(e);
}

void DropZoneMonsterWidget::dropEvent(QDropEvent *e)
{
    QStringList good;
    QStringList bad;
    split(e->mimeData(), good, bad);
    e->acceptProposedAction();
    feed(good + bad);
}

void DropZoneMonsterWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_clickToBrowse && rect().contains(eventPos(e))
        && (m_mode == IDLE || m_mode == REFUSED))
        browse();
    QWidget::mouseReleaseEvent(e);
}

// ---------- Animation tick ----------
void DropZoneMonsterWidget::tick()
{
    const qreal t = now();
    const qreal dt = std::min(0.05, std::max(0.0, t - m_last));
    m_last = t;
    const qreal since = t - m_modeAt;
    QRectF zone;
    QPointF c;
    qreal s;
    geometry(zone, c, s);
    if (m_mode == EATING) {
        if (since < EAT_TIME) {
            m_bitAcc += dt * 30.0;
            while (m_bitAcc > 1.0) {
                m_bitAcc -= 1.0;
                const qreal a = uniform(-PI * 0.9, -PI * 0.1);
                const qreal sp = uniform(80, 220);
                Bit b;
                b.x = c.x() + uniform(-s * 0.15, s * 0.15);
                b.y = c.y() + s * 0.12;
                b.vx = std::cos(a) * sp;
                b.vy = std::sin(a) * sp;
                b.rot = uniform(0, 360);
                b.vrot = uniform(-600, 600);
                b.life = uniform(0.6, 1.0);
                m_bits.append(b);
            }
        } else if (since > EAT_TIME + BURP_TIME + 0.4) {
            setMode(IDLE);
        }
    } else if (m_mode == REFUSED && since > 1.6) {
        setMode(IDLE);
    }
    if (t >= m_blinkAt + 0.14)
        m_blinkAt = t + uniform(2.0, 4.5);
    QList<Bit> alive;
    for (Bit b : std::as_const(m_bits)) {
        b.vy += 600.0 * dt;
        b.x += b.vx * dt;
        b.y += b.vy * dt;
        b.rot += b.vrot * dt;
        b.life -= dt;
        if (b.life > 0)
            alive.append(b);
    }
    m_bits = alive;
    update();
}

// ---------- Painting ----------
void DropZoneMonsterWidget::paintEvent(QPaintEvent *)
{
    const qreal tNow = now();
    QRectF zone;
    QPointF c;
    qreal s;
    geometry(zone, c, s);
    const Mode mode = m_mode;
    const qreal since = tNow - m_modeAt;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    // Zone with marching-ants border.
    const QColor txt(palette().text().color());
    QColor bc;
    if (mode == HUNGRY) {
        bc = QColor(m_accentColor);
    } else if (mode == YUCK || mode == REFUSED) {
        bc = QColor(QStringLiteral("#E63946"));
    } else {
        bc = QColor(txt);
        bc.setAlpha(80);
    }
    QColor fill(bc);
    fill.setAlpha((mode == HUNGRY || mode == YUCK) ? 28 : 0);
    QPen pen(bc, 2.0);
    pen.setDashPattern({6.0, 5.0});
    pen.setDashOffset(mode == HUNGRY ? -tNow * 20.0 : 0.0);
    p.setPen(pen);
    p.setBrush(fill);
    p.drawRoundedRect(zone, 18, 18);
    // Monster.
    p.save();
    const qreal shake = (mode == YUCK || mode == REFUSED)
                            ? std::sin(since * 30.0) * s * 0.05 * std::max(0.0, 1.0 - since / 0.8) : 0.0;
    p.translate(c.x() + shake, c.y());
    drawMonster(p, s, mode, since, tNow);
    p.restore();
    // Paper bits.
    p.setPen(Qt::NoPen);
    for (const Bit &b : std::as_const(m_bits)) {
        QColor pc(QStringLiteral("#FFFFFF"));
        pc.setAlpha(int(255 * clamp(b.life, 0.0, 1.0)));
        p.save();
        p.translate(b.x, b.y);
        p.rotate(b.rot);
        p.setBrush(pc);
        p.drawRect(QRectF(-4, -2.5, 8, 5));
        p.setPen(QPen(QColor(120, 140, 200, pc.alpha()), 0.8));
        p.drawLine(QPointF(-3, 0), QPointF(3, 0));
        p.setPen(Qt::NoPen);
        p.restore();
    }
    // Labels.
    QString mainText;
    QString sub;
    if (mode == HUNGRY) {
        mainText = tr("FEED ME!");
        sub = tr("Let go. He has been waiting.");
    } else if (mode == YUCK || mode == REFUSED) {
        mainText = tr("Yuck!");
        if (!m_exts.isEmpty()) {
            QStringList dotted;
            for (const QString &e : std::as_const(m_exts))
                dotted.append(QStringLiteral(".") + e);
            sub = tr("I only eat: %1").arg(dotted.join(QStringLiteral(", ")));
        }
    } else if (mode == EATING) {
        if (since < EAT_TIME) {
            mainText = tr("Nom nom nom...");
            sub = tr("%1 file(s) incoming").arg(m_lastBatch);
        } else {
            mainText = tr("*burp*");
            sub = tr("Thanks! That hit the spot.");
        }
    } else {
        mainText = m_hint;
        sub = m_subtitle;
    }
    const qreal ty = c.y() + s * 0.62 + 6.0;
    QFont f(font());
    f.setBold(true);
    f.setPointSizeF(clamp(s * 0.1, 10.0, 15.0));
    p.setFont(f);
    p.setPen((mode == HUNGRY || mode == YUCK || mode == REFUSED) ? bc : txt);
    p.drawText(QRectF(zone.left(), ty, zone.width(), 24.0), Qt::AlignCenter, mainText);
    if (!sub.isEmpty()) {
        QFont f2(font());
        f2.setPointSizeF(8.5);
        p.setFont(f2);
        QColor sc(txt);
        sc.setAlpha(160);
        p.setPen(sc);
        p.drawText(QRectF(zone.left() + 8, ty + 24.0, zone.width() - 16, 18.0), Qt::AlignCenter, sub);
    }
    if (m_eaten) {
        QFont f3(font());
        f3.setPointSizeF(8.0);
        p.setFont(f3);
        QColor cc(txt);
        cc.setAlpha(140);
        p.setPen(cc);
        p.drawText(QRectF(zone.left() + 12, zone.top() + 8, zone.width() - 24, 16), Qt::AlignRight | Qt::AlignVCenter,
                   tr("Eaten: %1").arg(m_eaten));
    }
    p.end();
}

void DropZoneMonsterWidget::drawMonster(QPainter &p, qreal s, Mode mode, qreal since, qreal now)
{
    QColor mc = m_monsterColor;
    if (mode == YUCK || mode == REFUSED)
        mc = QColor(int(mc.red() * 0.7 + 60), int(mc.green() * 0.85 + 30), int(mc.blue() * 0.5)); // queasy
    const QColor ink(QStringLiteral("#1E2A22"));
    const qreal r = s * 0.5;
    // Chomp amount: 0 closed .. 1 wide open.
    qreal mouth;
    if (mode == HUNGRY)
        mouth = 0.85 + 0.15 * std::sin(now * 6.0);
    else if (mode == EATING && since < EAT_TIME)
        mouth = std::abs(std::sin(since / EAT_TIME * PI * 3.0));
    else
        mouth = 0.0;
    qreal squash = (mode == EATING && since < EAT_TIME) ? 1.0 - 0.08 * (1.0 - mouth) : 1.0;
    const qreal bob = std::sin(now * 2.2) * r * 0.04;
    if (mode == EATING && EAT_TIME <= since && since < EAT_TIME + BURP_TIME) {
        const qreal k = (since - EAT_TIME) / BURP_TIME;
        squash = 1.0 + 0.12 * std::sin(k * PI); // burp inflation
    }
    p.translate(0, bob);
    p.scale(1.0 + (1.0 - squash) * 0.6, squash);
    // Horns.
    p.setPen(QPen(mc.darker(150), std::max(1.0, r * 0.03)));
    p.setBrush(QColor(QStringLiteral("#F6E7C1")));
    for (qreal d : {-1.0, 1.0}) {
        QPainterPath horn;
        horn.moveTo(d * r * 0.45, -r * 0.78);
        horn.quadTo(d * r * 0.7, -r * 1.15, d * r * 0.62, -r * 1.25);
        horn.quadTo(d * r * 0.55, -r * 1.0, d * r * 0.25, -r * 0.88);
        horn.closeSubpath();
        p.drawPath(horn);
    }
    // Wobbly blob body.
    QPainterPath body;
    for (int k = 0; k < 49; ++k) {
        const qreal a = k / 48.0 * 2.0 * PI;
        const qreal rr = r * (1.0 + 0.035 * std::sin(a * 5.0 + now * 3.0));
        const QPointF pt(std::cos(a) * rr * 1.08, std::sin(a) * rr * 0.95);
        if (k == 0)
            body.moveTo(pt);
        else
            body.lineTo(pt);
    }
    body.closeSubpath();
    p.setPen(QPen(mc.darker(140), std::max(1.5, r * 0.04)));
    p.setBrush(mc);
    p.drawPath(body);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 45));
    p.drawEllipse(QPointF(-r * 0.4, -r * 0.45), r * 0.25, r * 0.14);
    // Spots.
    p.setBrush(mc.darker(115));
    static const qreal spots[3][3] = {{0.7, -0.3, 0.1}, {0.78, 0.1, 0.07}, {-0.8, 0.2, 0.08}};
    for (const auto &sp : spots)
        p.drawEllipse(QPointF(sp[0] * r, sp[1] * r), sp[2] * r, sp[2] * r);
    // Eyes (one big, one small, like any respectable monster).
    const QPoint cur = mapFromGlobal(QCursor::pos());
    QRectF zone;
    QPointF c;
    qreal s2;
    geometry(zone, c, s2);
    const qreal lx = clamp((cur.x() - c.x()) / 200.0, -1.0, 1.0);
    const qreal ly = clamp((cur.y() - c.y()) / 200.0, -1.0, 1.0);
    const bool blinking = m_blinkAt <= now && now < m_blinkAt + 0.14;
    const bool happy = mode == EATING;
    const QPen pen(ink, std::max(1.5, r * 0.07), Qt::SolidLine, Qt::RoundCap);
    const qreal eyes[2][3] = {{-r * 0.32, -r * 0.35, r * 0.24}, {r * 0.3, -r * 0.4, r * 0.17}};
    for (const auto &eye : eyes) {
        const qreal ex = eye[0];
        const qreal ey = eye[1];
        const qreal er = eye[2];
        if (mode == YUCK || mode == REFUSED) {
            p.setPen(pen);
            const qreal d = ex < 0 ? -1.0 : 1.0;
            p.drawLine(QPointF(ex - d * er * 0.7, ey - er * 0.6), QPointF(ex + d * er * 0.5, ey));
            p.drawLine(QPointF(ex + d * er * 0.5, ey), QPointF(ex - d * er * 0.7, ey + er * 0.6));
            continue;
        }
        if (happy || blinking) {
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            if (happy)
                p.drawArc(QRectF(ex - er * 0.8, ey - er * 0.4, er * 1.6, er * 0.9), 20 * 16, 140 * 16);
            else
                p.drawArc(QRectF(ex - er * 0.8, ey - er * 0.4, er * 1.6, er * 0.9), 180 * 16, 180 * 16);
            continue;
        }
        const qreal big = er * (mode == HUNGRY ? 1.25 : 1.0);
        p.setPen(QPen(mc.darker(150), std::max(1.0, r * 0.03)));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(ex, ey), big, big);
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        const qreal pr = big * (mode == HUNGRY ? 0.6 : 0.48);
        p.drawEllipse(QPointF(ex + lx * (big - pr) * 0.8, ey + ly * (big - pr) * 0.8), pr, pr);
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        p.drawEllipse(QPointF(ex + lx * (big - pr) * 0.8 - pr * 0.35, ey + ly * (big - pr) * 0.8 - pr * 0.35),
                      pr * 0.3, pr * 0.3);
    }
    // Mouth.
    const qreal my = r * 0.25;
    const qreal mw = r * 0.62;
    if (mode == YUCK || mode == REFUSED) {
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        QPainterPath wav;
        wav.moveTo(-mw * 0.6, my);
        for (int k = 1; k < 7; ++k)
            wav.lineTo(-mw * 0.6 + k * mw * 1.2 / 6.0, my + ((k % 2) ? r * 0.05 : -r * 0.05));
        p.drawPath(wav);
        p.setPen(QPen(QColor(QStringLiteral("#C2405A")), std::max(1.0, r * 0.03)));
        p.setBrush(QColor(QStringLiteral("#FF7A90")));
        p.drawRoundedRect(QRectF(r * 0.02, my + r * 0.02, r * 0.2, r * 0.28), r * 0.1, r * 0.1); // bleh
        return;
    }
    const qreal mh = r * 0.55 * mouth;
    if (mh < r * 0.06) {
        // Closed grin with one tooth poking out.
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-mw * 0.6, my - r * 0.2, mw * 1.2, r * 0.35), 200 * 16, 140 * 16);
        p.setPen(QPen(mc.darker(150), std::max(1.0, r * 0.02)));
        p.setBrush(QColor(QStringLiteral("#FFFFFF")));
        QPainterPath tooth;
        tooth.moveTo(r * 0.12, my + r * 0.13);
        tooth.lineTo(r * 0.26, my + r * 0.1);
        tooth.lineTo(r * 0.2, my + r * 0.26);
        tooth.closeSubpath();
        p.drawPath(tooth);
        return;
    }
    const QRectF mrect(-mw, my - mh * 0.4, mw * 2.0, mh);
    QPainterPath mp;
    mp.addEllipse(mrect);
    p.setPen(pen);
    p.setBrush(QColor(QStringLiteral("#3B1020")));
    p.drawPath(mp);
    p.save();
    p.setClipPath(mp);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#FF7A90")));
    p.drawEllipse(QPointF(0, mrect.bottom()), mw * 0.55, mh * 0.45);
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    const int n = 6;
    for (int k = 0; k < n; ++k) {
        const qreal x = -mw + (k + 0.5) * (2.0 * mw / n);
        const qreal tw = mw / n * 0.8;
        QPainterPath top;
        top.moveTo(x - tw, mrect.top());
        top.lineTo(x + tw, mrect.top());
        top.lineTo(x, mrect.top() + mh * 0.28);
        top.closeSubpath();
        p.drawPath(top);
        if (k % 2 == 1) {
            QPainterPath bot;
            bot.moveTo(x - tw, mrect.bottom());
            bot.lineTo(x + tw, mrect.bottom());
            bot.lineTo(x, mrect.bottom() - mh * 0.22);
            bot.closeSubpath();
            p.drawPath(bot);
        }
    }
    p.restore();
    // Drool.
    if (mode == HUNGRY) {
        const qreal dl = r * (0.1 + 0.25 * pyMod(now * 0.7, 1.0));
        QColor dc(QStringLiteral("#BEE9FF"));
        dc.setAlpha(200);
        p.setPen(QPen(dc, std::max(2.0, r * 0.06), Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(-mw * 0.5, mrect.bottom() - mh * 0.15), QPointF(-mw * 0.5, mrect.bottom() + dl));
    }
}
