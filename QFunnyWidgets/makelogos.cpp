/**
 * Regenerates <package>/logo.png for every widget (the counterpart of makelogos.py): each widget is rendered
 * off-screen in a characteristic state, cropped to its content and composed onto a rounded gradient tile.
 *
 *   funnywidgets-makelogos [size] [source-dir]
 *
 * size defaults to 256; source-dir defaults to the QFunnyWidgets source directory this tool was built from.
 * Remember to rebuild afterwards: the logos are compiled in as Qt resources.
 */
#include "QFunnyWidgets.h"

#include <QApplication>
#include <QDir>
#include <QDragEnterEvent>
#include <QImage>
#include <QLabel>
#include <QLinearGradient>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRegion>
#include <QScrollArea>
#include <QScrollBar>
#include <QSharedPointer>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <algorithm>
#include <functional>

namespace {

QRect contentBox(const QImage &img, const QRect &region, bool &found)
{
    int x0 = region.right(), y0 = region.bottom(), x1 = region.left(), y1 = region.top();
    found = false;
    for (int y = region.top(); y <= region.bottom(); y += 2) {
        for (int x = region.left(); x <= region.right(); x += 2) {
            if (QColor::fromRgba(img.pixel(x, y)).alpha() > 40) {
                found = true;
                x0 = std::min(x0, x);
                y0 = std::min(y0, y);
                x1 = std::max(x1, x);
                y1 = std::max(y1, y);
            }
        }
    }
    return QRect(x0, y0, x1 - x0 + 2, y1 - y0 + 2).intersected(region);
}

QImage clearFaint(const QImage &img, int threshold = 40)
{
    QImage out = img.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y) {
        for (int x = 0; x < out.width(); ++x) {
            if (QColor::fromRgba(out.pixel(x, y)).alpha() < threshold)
                out.setPixel(x, y, 0);
        }
    }
    return out;
}

QImage compose(const QImage &art, const QColor &c0, const QColor &c1, int size)
{
    QImage icon(size, size, QImage::Format_ARGB32_Premultiplied);
    icon.fill(Qt::transparent);
    QPainter p(&icon);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    const qreal s = size;
    const QRectF rect(s * 0.03, s * 0.03, s * 0.94, s * 0.94);
    QPainterPath shape;
    shape.addRoundedRect(rect, s * 0.22, s * 0.22);
    p.fillPath(shape.translated(0, s * 0.012), QColor(0, 0, 0, 60));
    QLinearGradient grad(rect.topLeft(), rect.bottomRight());
    grad.setColorAt(0.0, c0);
    grad.setColorAt(1.0, c1);
    p.fillPath(shape, grad);
    p.save();
    p.setClipPath(shape);
    p.fillRect(QRectF(rect.left(), rect.top(), rect.width(), rect.height() * 0.45), QColor(255, 255, 255, 28));
    p.restore();
    p.setPen(QPen(QColor(255, 255, 255, 70), std::max(1.0, s * 0.008)));
    p.setBrush(Qt::NoBrush);
    p.drawPath(shape);
    // Fit the artwork into ~74 % of the tile, with a soft drop shadow.
    const qreal box = s * 0.74;
    const qreal k = std::min(box / art.width(), box / art.height());
    const qreal w = art.width() * k;
    const qreal h = art.height() * k;
    const QRectF target((s - w) / 2.0, (s - h) / 2.0 + s * 0.01, w, h);
    QImage shadow(art.size(), QImage::Format_ARGB32_Premultiplied);
    shadow.fill(Qt::transparent);
    QPainter sp(&shadow);
    sp.drawImage(0, 0, art);
    sp.setCompositionMode(QPainter::CompositionMode_SourceIn);
    sp.fillRect(shadow.rect(), QColor(0, 0, 0, 70));
    sp.end();
    p.drawImage(target.translated(0, s * 0.018), shadow, QRectF(shadow.rect()));
    p.drawImage(target, art, QRectF(art.rect()));
    p.end();
    return icon;
}

QImage grabWidget(QWidget *wdg)
{
    QImage img(wdg->size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    wdg->render(&p, QPoint(), QRegion(), QWidget::DrawChildren);
    p.end();
    return img;
}

/** Holder for objects that must outlive the factory call (the Python "h" dict). */
struct Holder
{
    QList<QSharedPointer<QObject>> keep;
    QWidget *target = nullptr;
};

struct Config
{
    QString name;
    std::function<QWidget *(Holder &)> factory;
    QSize size;  // invalid = the factory shows its own window
    int wait;
    bool hasCrop;
    QRectF crop; // fractions: left, top, right, bottom stored as x1,y1,x2,y2 via QRectF(QPointF, QPointF)
    QColor c0;
    QColor c1;
};

Config cfg(const char *name, std::function<QWidget *(Holder &)> factory, QSize size, int wait, const QColor &c0,
           const QColor &c1)
{
    return Config{QString::fromLatin1(name), std::move(factory), size, wait, false, QRectF(), c0, c1};
}

Config cfgCrop(const char *name, std::function<QWidget *(Holder &)> factory, QSize size, int wait, qreal l, qreal t,
               qreal r, qreal b, const QColor &c0, const QColor &c1)
{
    return Config{QString::fromLatin1(name), std::move(factory), size, wait, true, QRectF(QPointF(l, t), QPointF(r, b)), c0, c1};
}

QColor c(const char *hex)
{
    return QColor(QString::fromLatin1(hex));
}

void sendMousePress(QWidget *w, const QPointF &pos)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMouseEvent e(QEvent::MouseButtonPress, pos, w->mapToGlobal(pos), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#else
    QMouseEvent e(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#endif
    QApplication::sendEvent(w, &e);
}

QList<Config> configs()
{
    QList<Config> list;
    list << cfg("googlyeyes", [](Holder &) { return new GooglyEyesWidget(); }, QSize(260, 150), 900, c("#FFD23F"), c("#FF9F1C"));
    list << cfgCrop("runawaybutton", [](Holder &) {
        RunawayButtonWidget::Options o;
        o.maxEscapes = 0;
        return new RunawayButtonWidget(o);
    }, QSize(300, 190), 700, 0.18, 0.28, 0.82, 0.72, c("#FFE5EC"), c("#FFB3C6"));
    list << cfg("jellyswitch", [](Holder &) {
        JellySwitchWidget::Options o;
        o.checked = true;
        return new JellySwitchWidget(o);
    }, QSize(150, 90), 900, c("#D8F3DC"), c("#74C69D"));
    list << cfgCrop("hamsterloader", [](Holder &) {
        HamsterLoaderWidget::Options o;
        o.progress = 0.55;
        o.showPercent = false;
        return new HamsterLoaderWidget(o);
    }, QSize(300, 270), 1200, 0.0, 0.0, 1.0, 0.7, c("#FFF1E6"), c("#FFC6A5"));
    list << cfg("moodslider", [](Holder &) {
        MoodSliderWidget::Options o;
        o.value = 100;
        o.showLabel = false;
        return new MoodSliderWidget(o);
    }, QSize(110, 110), 900, c("#EBFBEE"), c("#8CE99A"));
    list << cfg("ratingstars", [](Holder &) {
        StarRatingWidget::Options o;
        o.rating = 3;
        o.maxRating = 3;
        o.showLabel = false;
        return new StarRatingWidget(o);
    }, QSize(260, 110), 1200, c("#5E60CE"), c("#7B5CFF"));
    list << cfg("holdbutton", [](Holder &) {
        NervousHoldButtonWidget::Options o;
        o.stages = QStringList{QString(), QString(), QString(), QString()};
        NervousHoldButtonWidget *w = new NervousHoldButtonWidget(o);
        // The Python version calls the private _press(); a real press on the button does the same.
        QTimer::singleShot(50, w, [w]() { sendMousePress(w, QPointF(w->width() / 2.0, w->height() / 2.0)); });
        return w;
    }, QSize(300, 110), 1000, c("#FFE3E3"), c("#FFA8A8"));
    list << cfg("panicbell", [](Holder &) {
        PanicBellWidget::Options o;
        o.count = 15;
        o.clearOnClick = false;
        return new PanicBellWidget(o);
    }, QSize(200, 200), 900, c("#E7F5FF"), c("#74C0FC"));
    list << cfg("rubberduck", [](Holder &) {
        RubberDuckWidget::Options o;
        o.adviceOnClick = false;
        return new RubberDuckWidget(o);
    }, QSize(240, 220), 900, c("#E3FAFC"), c("#66D9E8"));
    list << cfgCrop("passwordjudge", [](Holder &) {
        JudgeFaceWidget::Options o;
        o.strength = 3;
        o.comment = QString();
        return new JudgeFaceWidget(o);
    }, QSize(300, 150), 900, 0.0, 0.0, 0.47, 1.0, c("#F3F0FF"), c("#B197FC"));
    list << cfgCrop("cookieconsent", [](Holder &) {
        CookieConsentWidget::Options o;
        o.panelColor = QColor(0, 0, 0, 0);
        o.title = QString();
        o.message = QString();
        return new CookieConsentWidget(o);
    }, QSize(520, 160), 600, 0.0, 0.0, 0.27, 1.0, c("#FFF4E6"), c("#FFC078"));
    list << cfg("typingindicator", [](Holder &) {
        TypingIndicatorWidget::Options o;
        o.name = QString();
        return new TypingIndicatorWidget(o);
    }, QSize(200, 80), 700, c("#EDF2FF"), c("#91A7FF"));
    list << cfgCrop("emptystate", [](Holder &) {
        EmptyStateWidget::Options o;
        o.title = QString();
        o.subtitle = QString();
        o.actionText = QString();
        o.speed = 0.35;
        return new EmptyStateWidget(o);
    }, QSize(260, 200), 2600, 0.0, 0.3, 1.0, 1.0, c("#FFF9DB"), c("#FFE066"));
    list << cfg("volumeknob", [](Holder &) {
        VolumeKnobWidget::Options o;
        o.value = 60;
        o.showLabel = false;
        return new VolumeKnobWidget(o);
    }, QSize(200, 200), 900, c("#343A40"), c("#212529"));
    list << cfg("pigeontoast", [](Holder &h) {
        QWidget *holder = new QWidget();
        holder->resize(330, 170);
        PigeonToast::Options o;
        o.caption = QString();
        o.duration = 5000;
        PigeonToast *t = new PigeonToast(holder, o);
        QTimer::singleShot(30, t, [t]() { t->showMessage(QStringLiteral("Hi!")); });
        Q_UNUSED(h)
        return holder;
    }, QSize(330, 170), 1500, c("#E7F5FF"), c("#A5D8FF"));
    list << cfgCrop("dropzone", [](Holder &h) {
        DropZoneMonsterWidget::Options o;
        o.hint = QString();
        o.subtitle = QString();
        o.clickToBrowse = false;
        DropZoneMonsterWidget *w = new DropZoneMonsterWidget(o);
        QSharedPointer<QMimeData> m(new QMimeData());
        m->setUrls({QUrl::fromLocalFile(QStringLiteral("/tmp/snack.png"))});
        h.keep.append(m);
        QMimeData *mime = m.data();
        QTimer::singleShot(50, w, [w, mime]() {
            QDragEnterEvent e(w->rect().center(), Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(w, &e);
        });
        return w;
    }, QSize(300, 260), 700, 0.24, 0.03, 0.76, 0.6, c("#EBFBEE"), c("#69DB7C"));
    list << cfg("batterybuddy", [](Holder &) {
        BatteryBuddyWidget::Options o;
        o.level = 88;
        o.showLabel = false;
        return new BatteryBuddyWidget(o);
    }, QSize(220, 140), 900, c("#E6FCF5"), c("#63E6BE"));
    list << cfg("presenceavatar", [](Holder &) {
        PresenceAvatarWidget::Options o;
        o.name = QStringLiteral("Funny Widgets");
        o.badgeBorderColor = c("#FFFFFF");
        return new PresenceAvatarWidget(o);
    }, QSize(140, 150), 700, c("#F8F0FC"), c("#E599F7"));
    list << cfg("ducklingpager", [](Holder &) {
        DucklingPagerWidget::Options o;
        o.count = 3;
        o.current = 0;
        DucklingPagerWidget *w = new DucklingPagerWidget(o);
        QTimer::singleShot(200, w, [w]() { w->setCurrent(2); });
        return w;
    }, QSize(230, 90), 2200, c("#E3FAFC"), c("#3BC9DB"));
    list << cfgCrop("deadline", [](Holder &) {
        DeadlineCountdownWidget::Options o;
        o.duration = 100;
        DeadlineCountdownWidget *w = new DeadlineCountdownWidget(o);
        QTimer::singleShot(30, w, [w]() { w->extend(0); });
        QTimer::singleShot(40, w, [w]() { w->setTimeScale(400.0); });
        return w;
    }, QSize(420, 170), 1500, 0.0, 0.0, 0.3, 1.0, c("#FFF0F6"), c("#FFA8C5"));
    list << cfgCrop("searchdetective", [](Holder &) {
        DetectiveFaceWidget::Options o;
        o.state = DetectiveFaceWidget::FOUND;
        o.comment = QString();
        return new DetectiveFaceWidget(o);
    }, QSize(330, 150), 900, 0.0, 0.0, 0.42, 1.0, c("#FFF4E6"), c("#FFD8A8"));
    list << cfgCrop("robotcheck", [](Holder &) {
        RobotCheckWidget::Options o;
        o.text = QString();
        o.verifiedText = QString();
        RobotCheckWidget *w = new RobotCheckWidget(o);
        QTimer::singleShot(50, w, &RobotCheckWidget::verify);
        return w;
    }, QSize(360, 190), 2300, 0.57, 0.0, 0.93, 0.6, c("#E7F5FF"), c("#4DABF7"));
    list << cfgCrop("chameleonpicker", [](Holder &) {
        ChameleonColorPickerWidget::Options o;
        o.color = c("#3BCEAC");
        o.showHex = false;
        return new ChameleonColorPickerWidget(o);
    }, QSize(460, 220), 900, 0.0, 0.05, 0.5, 0.95, c("#F4FCE3"), c("#C0EB75"));
    list << cfgCrop("errorgremlin", [](Holder &) {
        ErrorGremlinWidget::Options o;
        o.severity = 2;
        o.title = QString();
        o.message = QString();
        o.retryText = QString();
        o.closable = false;
        return new ErrorGremlinWidget(o);
    }, QSize(520, 110), 900, 0.03, 0.06, 0.22, 0.94, c("#FFE3E3"), c("#FF8787"));
    list << cfgCrop("snailstepper", [](Holder &) {
        SnailStepperWidget::Options o;
        o.steps = QStringList{QString(), QString()};
        return new SnailStepperWidget(o);
    }, QSize(240, 130), 700, 0.0, 0.0, 0.45, 0.72, c("#F4FCE3"), c("#A9E34B"));
    list << cfgCrop("pancakestepper", [](Holder &) {
        PancakeStepperWidget::Options o;
        o.value = 4;
        o.maximum = 8;
        return new PancakeStepperWidget(o);
    }, QSize(260, 280), 1300, 0.12, 0.25, 0.88, 0.8, c("#FFF4E6"), c("#FFA94D"));
    list << cfg("heartlike", [](Holder &) {
        HeartLikeButtonWidget::Options o;
        o.liked = true;
        o.showCount = false;
        return new HeartLikeButtonWidget(o);
    }, QSize(160, 140), 700, c("#3D405B"), c("#2B2D42"));
    list << cfgCrop("copyblob", [](Holder &) {
        CopyBlobButtonWidget::Options o;
        o.text = QString();
        o.label = QStringLiteral(" ");
        o.prompt = QString();
        o.chipColor = QColor(0, 0, 0, 0);
        CopyBlobButtonWidget *w = new CopyBlobButtonWidget(o);
        QTimer::singleShot(820, w, &CopyBlobButtonWidget::copy);
        return w;
    }, QSize(220, 110), 1000, 0.7, 0.48, 1.0, 1.0, c("#F4FCE3"), c("#94D82D"));
    list << cfg("rockettop", [](Holder &h) -> QWidget * {
        QScrollArea *area = new QScrollArea();
        QStringList xs;
        for (int i = 0; i < 120; ++i)
            xs.append(QStringLiteral("x"));
        QLabel *lbl = new QLabel(xs.join(QStringLiteral("\n\n")));
        area->setWidget(lbl);
        area->resize(220, 300);
        area->show();
        RocketBackToTop::Options o;
        o.threshold = 50;
        RocketBackToTop *r = new RocketBackToTop(area, o);
        QTimer::singleShot(60, area, [area]() { area->verticalScrollBar()->setValue(600); });
        h.keep.append(QSharedPointer<QObject>(area));
        h.target = r;
        return nullptr;
    }, QSize(), 1400, c("#3A3F7A"), c("#1B1F3B"));
    list << cfgCrop("catsection", [](Holder &) {
        CatHeaderWidget::Options o;
        o.title = QString();
        o.expanded = false;
        return new CatHeaderWidget(o);
    }, QSize(240, 82), 700, 0.66, 0.0, 1.0, 0.57, c("#FFF4E6"), c("#FFC078"));
    list << cfgCrop("dachshundtabs", [](Holder &) {
        DachshundTabsWidget::Options o;
        o.tabs = QStringList{QString(), QString()};
        o.trackColor = QColor(0, 0, 0, 0);
        return new DachshundTabsWidget(o);
    }, QSize(220, 64), 600, 0.0, 0.0, 0.62, 1.0, c("#FFF4E6"), c("#E8A87C"));
    list << cfgCrop("puffercounter", [](Holder &) {
        PufferFooterWidget::Options o;
        o.count = 262;
        o.maxLength = 280;
        return new PufferFooterWidget(o);
    }, QSize(360, 64), 1200, 0.72, 0.0, 1.0, 1.0, c("#E3FAFC"), c("#3BC9DB"));
    list << cfg("helpowl", [](Holder &) { return new HelpOwlWidget(); }, QSize(120, 140), 700, c("#F3F0FF"), c("#9775FA"));
    list << cfg("antbreadcrumb", [](Holder &) {
        AntBreadcrumbWidget::Options o;
        o.path = QStringList{QStringLiteral("Home")};
        return new AntBreadcrumbWidget(o);
    }, QSize(120, 60), 900, c("#FFF9DB"), c("#FCC419"));
    list << cfgCrop("autosave", [](Holder &) {
        AutosaveSquirrelWidget::Options o;
        o.autosaveDelay = 0;
        AutosaveSquirrelWidget *w = new AutosaveSquirrelWidget(o);
        QTimer::singleShot(30, w, &AutosaveSquirrelWidget::markDirty);
        return w;
    }, QSize(300, 84), 900, 0.0, 0.0, 0.38, 1.0, c("#F4FCE3"), c("#C0EB75"));
    return list;
}

} // namespace

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    const int size = argc > 1 ? QString::fromLocal8Bit(argv[1]).toInt() : 256;
#ifdef QFW_SOURCE_DIR
    const QString root = argc > 2 ? QString::fromLocal8Bit(argv[2]) : QStringLiteral(QFW_SOURCE_DIR);
#else
    const QString root = argc > 2 ? QString::fromLocal8Bit(argv[2]) : QDir::currentPath();
#endif
    QTextStream out(stdout);
    struct Job
    {
        Config config;
        QSharedPointer<Holder> holder;
        QWidget *target;
        QWidget *widget;
    };
    QList<Job> jobs;
    for (const Config &config : configs()) {
        QSharedPointer<Holder> holder(new Holder());
        QWidget *wdg = config.factory(*holder);
        if (wdg != nullptr) {
            wdg->resize(config.size);
            wdg->show();
        }
        QWidget *target = holder->target != nullptr ? holder->target : wdg;
        jobs.append(Job{config, holder, target, wdg});
    }
    int done = 0;
    int maxWait = 0;
    for (const Job &job : std::as_const(jobs)) {
        maxWait = std::max(maxWait, job.config.wait);
        QTimer::singleShot(job.config.wait, &app, [job, size, root, &done, &out]() {
            const QImage img = grabWidget(job.target);
            QRect r(0, 0, img.width(), img.height());
            if (job.config.hasCrop) {
                const QRectF &cr = job.config.crop;
                r = QRect(int(cr.left() * img.width()), int(cr.top() * img.height()),
                          int((cr.right() - cr.left()) * img.width()), int((cr.bottom() - cr.top()) * img.height()));
            }
            bool found = false;
            const QRect box = contentBox(img, r, found);
            if (!found) {
                out << "  empty render: " << job.config.name << Qt::endl;
                return;
            }
            const QImage icon = compose(clearFaint(img.copy(box)), job.config.c0, job.config.c1, size);
            icon.save(QDir(root).filePath(job.config.name + QStringLiteral("/logo.png"))); // beside the widget sources
            ++done;
        });
    }
    QTimer::singleShot(maxWait + 300, &app, &QApplication::quit);
    app.exec();
    out << done << " logos written" << Qt::endl;
    for (const Job &job : std::as_const(jobs))
        delete job.widget;
    return done == jobs.size() ? 0 : 1;
}
