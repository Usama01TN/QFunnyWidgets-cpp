/**
 * Shared metadata for the Qt Designer plugin (the counterpart of plugins/funnywidgetsinfo.py).
 */
#include "funnywidgetsinfo.h"
#include "QFunnyWidgets.h"

#include <QHash>
#include <functional>

const char *const GROUP_INPUTS = "QFunnyWidgets: Inputs";
const char *const GROUP_BUTTONS = "QFunnyWidgets: Buttons";
const char *const GROUP_NAVIGATION = "QFunnyWidgets: Navigation";
const char *const GROUP_FEEDBACK = "QFunnyWidgets: Feedback";
const char *const GROUP_HELPERS = "QFunnyWidgets: Containers & helpers";
const char *const GROUP_MASCOTS = "QFunnyWidgets: Mascots";

QString WidgetInfo::objectName() const
{
    const QString base = className.endsWith(QLatin1String("Widget")) ? className.left(className.size() - 6) : className;
    return base.left(1).toLower() + base.mid(1);
}

QString WidgetInfo::includeFile() const
{
    return QStringLiteral("%1/%1.h").arg(package);
}

QString WidgetInfo::logoPath() const
{
    return QStringLiteral(":/QFunnyWidgets/%1/logo.png").arg(package);
}

QString WidgetInfo::domXml() const
{
    QString tip = toolTip;
    tip.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    tip.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    tip.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    return QStringLiteral("<ui language=\"c++\" displayname=\"%1\">\n"
                          " <widget class=\"%2\" name=\"%3\">\n"
                          "  <property name=\"geometry\">\n"
                          "   <rect><x>0</x><y>0</y><width>%4</width><height>%5</height></rect>\n"
                          "  </property>\n"
                          "  <property name=\"toolTip\"><string>%6</string></property>\n"
                          " </widget>\n"
                          "</ui>\n")
        .arg(title, className, objectName())
        .arg(width)
        .arg(height)
        .arg(tip);
}

static WidgetInfo info(const char *className, const char *package, const char *title, const char *group,
                       const char *toolTip, int width, int height)
{
    WidgetInfo i;
    i.className = QString::fromLatin1(className);
    i.package = QString::fromLatin1(package);
    i.title = QString::fromLatin1(title);
    i.group = QString::fromLatin1(group);
    i.toolTip = QString::fromLatin1(toolTip);
    i.width = width;
    i.height = height;
    return i;
}

const QList<WidgetInfo> &funnyWidgetInfos()
{
    static const QList<WidgetInfo> WIDGETS = {
        info("JellySwitchWidget", "jellyswitch", "Jelly Switch", GROUP_INPUTS,
             "Toggle switch with a jelly knob that sleeps when off.", 130, 70),
        info("MoodSliderWidget", "moodslider", "Mood Slider", GROUP_INPUTS,
             "Slider whose thumb is a face from furious to ecstatic.", 340, 110),
        info("StarRatingWidget", "ratingstars", "Star Rating", GROUP_INPUTS,
             "Star rating where every star has feelings.", 320, 110),
        info("VolumeKnobWidget", "volumeknob", "Volume Knob", GROUP_INPUTS,
             "Rotary dial that goes from zipped mouth to screaming.", 240, 260),
        info("PasswordJudgeWidget", "passwordjudge", "Password Judge", GROUP_INPUTS,
             "Password field reviewed live by a powdered-wig judge.", 400, 194),
        info("SearchDetectiveWidget", "searchdetective", "Search Detective", GROUP_INPUTS,
             "Debounced search field with an investigating detective.", 400, 184),
        info("ChameleonColorPickerWidget", "chameleonpicker", "Chameleon Color Picker", GROUP_INPUTS,
             "Swatch color picker; the chameleon eats the color you pick.", 460, 220),
        info("PancakeStepperWidget", "pancakestepper", "Pancake Stepper", GROUP_INPUTS,
             "Quantity spin box served as a stack of pancakes.", 260, 280),
        info("PufferCounterWidget", "puffercounter", "Puffer Counter", GROUP_INPUTS,
             "Text area with a character limit and an inflating pufferfish.", 360, 262),
        info("RunawayButtonWidget", "runawaybutton", "Runaway Button", GROUP_BUTTONS,
             "A button that dodges the cursor until it gets tired.", 460, 260),
        info("NervousHoldButtonWidget", "holdbutton", "Nervous Hold Button", GROUP_BUTTONS,
             "Hold-to-confirm button that panics, then faints.", 320, 110),
        info("HeartLikeButtonWidget", "heartlike", "Heart Like Button", GROUP_BUTTONS,
             "Like button that pops when liked and cracks when un-liked.", 200, 120),
        info("CopyBlobButtonWidget", "copyblob", "Copy Blob Button", GROUP_BUTTONS,
             "Copy-to-clipboard chip whose blob clones itself.", 340, 110),
        info("RobotCheckWidget", "robotcheck", "Robot Check", GROUP_BUTTONS,
             "\"I am not a robot\" box with a nervous robot behind it.", 360, 190),
        info("DucklingPagerWidget", "ducklingpager", "Duckling Pager", GROUP_NAVIGATION,
             "Carousel page indicator: a duck family on lily pads.", 360, 100),
        info("SnailStepperWidget", "snailstepper", "Snail Stepper", GROUP_NAVIGATION,
             "Wizard step indicator carried by a slow snail.", 520, 130),
        info("DachshundTabsWidget", "dachshundtabs", "Dachshund Tabs", GROUP_NAVIGATION,
             "Segmented control whose highlight is a stretchy dachshund.", 480, 64),
        info("AntBreadcrumbWidget", "antbreadcrumb", "Ant Breadcrumb", GROUP_NAVIGATION,
             "Breadcrumb trail of bread crumbs carried by ants.", 420, 56),
        info("HamsterLoaderWidget", "hamsterloader", "Hamster Loader", GROUP_FEEDBACK,
             "Progress indicator powered by a hamster wheel.", 300, 260),
        info("PanicBellWidget", "panicbell", "Panic Bell", GROUP_FEEDBACK,
             "Notification bell that panics as the count grows.", 180, 180),
        info("TypingIndicatorWidget", "typingindicator", "Typing Indicator", GROUP_FEEDBACK,
             "Chat typing dots that type, think, delete or sleep.", 260, 90),
        info("BatteryBuddyWidget", "batterybuddy", "Battery Buddy", GROUP_FEEDBACK,
             "Battery status with moods from energized to empty.", 280, 200),
        info("DeadlineCountdownWidget", "deadline", "Deadline Countdown", GROUP_FEEDBACK,
             "Countdown with a calendar that ends up \"fine\" in flames.", 440, 170),
        info("AutosaveSquirrelWidget", "autosave", "Autosave Squirrel", GROUP_FEEDBACK,
             "Autosave status run by an acorn-stashing squirrel.", 300, 76),
        info("ErrorGremlinWidget", "errorgremlin", "Error Gremlin", GROUP_FEEDBACK,
             "Inline alert banner with a severity-matched gremlin.", 520, 110),
        info("CookieConsentWidget", "cookieconsent", "Cookie Consent", GROUP_FEEDBACK,
             "Cookie banner starring an emotionally invested cookie.", 520, 160),
        info("EmptyStateWidget", "emptystate", "Empty State", GROUP_FEEDBACK,
             "\"Nothing here yet\" placeholder with a rolling tumbleweed.", 420, 300),
        info("SleepyCatSectionWidget", "catsection", "Sleepy Cat Section", GROUP_HELPERS,
             "Collapsible section with a cat napping on its header.", 360, 82),
        info("HelpOwlWidget", "helpowl", "Help Owl", GROUP_HELPERS,
             "Inline help icon: an owl that wakes up and explains.", 34, 40),
        info("DropZoneMonsterWidget", "dropzone", "Drop Zone Monster", GROUP_HELPERS,
             "File drop zone guarded by a hungry monster.", 360, 300),
        info("PresenceAvatarWidget", "presenceavatar", "Presence Avatar", GROUP_HELPERS,
             "Initials avatar whose online status is its mood.", 130, 150),
        info("GooglyEyesWidget", "googlyeyes", "Googly Eyes", GROUP_MASCOTS,
             "Googly eyes that follow the cursor with real physics.", 260, 140),
        info("RubberDuckWidget", "rubberduck", "Rubber Duck", GROUP_MASCOTS,
             "Rubber-duck-debugging companion with dubious advice.", 320, 260),
    };
    return WIDGETS;
}

QWidget *createFunnyWidget(const QString &className, QWidget *parent)
{
    using Factory = std::function<QWidget *(QWidget *)>;
#define QFW_FACTORY(Class) {QStringLiteral(#Class), [](QWidget *p) -> QWidget * { return new Class(p); }}
    static const QHash<QString, Factory> FACTORIES = {
        QFW_FACTORY(JellySwitchWidget), QFW_FACTORY(MoodSliderWidget), QFW_FACTORY(StarRatingWidget),
        QFW_FACTORY(VolumeKnobWidget), QFW_FACTORY(PasswordJudgeWidget), QFW_FACTORY(SearchDetectiveWidget),
        QFW_FACTORY(ChameleonColorPickerWidget), QFW_FACTORY(PancakeStepperWidget), QFW_FACTORY(PufferCounterWidget),
        QFW_FACTORY(RunawayButtonWidget), QFW_FACTORY(NervousHoldButtonWidget), QFW_FACTORY(HeartLikeButtonWidget),
        QFW_FACTORY(CopyBlobButtonWidget), QFW_FACTORY(RobotCheckWidget), QFW_FACTORY(DucklingPagerWidget),
        QFW_FACTORY(SnailStepperWidget), QFW_FACTORY(DachshundTabsWidget), QFW_FACTORY(AntBreadcrumbWidget),
        QFW_FACTORY(HamsterLoaderWidget), QFW_FACTORY(PanicBellWidget), QFW_FACTORY(TypingIndicatorWidget),
        QFW_FACTORY(BatteryBuddyWidget), QFW_FACTORY(DeadlineCountdownWidget), QFW_FACTORY(AutosaveSquirrelWidget),
        QFW_FACTORY(ErrorGremlinWidget), QFW_FACTORY(CookieConsentWidget), QFW_FACTORY(EmptyStateWidget),
        QFW_FACTORY(SleepyCatSectionWidget), QFW_FACTORY(HelpOwlWidget), QFW_FACTORY(DropZoneMonsterWidget),
        QFW_FACTORY(PresenceAvatarWidget), QFW_FACTORY(GooglyEyesWidget), QFW_FACTORY(RubberDuckWidget),
    };
#undef QFW_FACTORY
    const auto it = FACTORIES.constFind(className);
    return it != FACTORIES.constEnd() ? (*it)(parent) : nullptr;
}
