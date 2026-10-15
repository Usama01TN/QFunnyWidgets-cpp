/**
 * Gallery of all QFunnyWidgets, split over seven pages.
 */
#ifndef QFUNNYWIDGETS_MAINWINDOW_H
#define QFUNNYWIDGETS_MAINWINDOW_H

#include <QFrame>
#include <QList>
#include <QMainWindow>

class QButtonGroup;
class QLabel;
class QStackedWidget;
class QTimer;
class QVBoxLayout;

class AntBreadcrumbWidget;
class AutosaveSquirrelWidget;
class BatteryBuddyWidget;
class ChameleonColorPickerWidget;
class CookieConsentWidget;
class DachshundTabsWidget;
class DeadlineCountdownWidget;
class DropZoneMonsterWidget;
class DucklingPagerWidget;
class EmptyStateWidget;
class ErrorGremlinWidget;
class GooglyEyesWidget;
class HamsterLoaderWidget;
class HeartLikeButtonWidget;
class HelpOwlWidget;
class MoodSliderWidget;
class NervousHoldButtonWidget;
class PancakeStepperWidget;
class PanicBellWidget;
class PasswordJudgeWidget;
class PigeonToast;
class PresenceAvatarWidget;
class PufferCounterWidget;
class RobotCheckWidget;
class RocketBackToTop;
class RubberDuckWidget;
class RunawayButtonWidget;
class SearchDetectiveWidget;
class SleepyCatSectionWidget;
class SnailStepperWidget;
class StarRatingWidget;
class TypingIndicatorWidget;
class VolumeKnobWidget;

/**
 * A rounded card with a logo, a title, a subtitle, an optional tag pill and a body layout.
 */
class Card : public QFrame
{
    Q_OBJECT

public:
    explicit Card(const QString &title = QString(), const QString &subtitle = QString(), const QString &tag = QString(),
                  const QString &logo = QString(), QWidget *parent = nullptr);

    QVBoxLayout *body() const;

private:
    QVBoxLayout *m_body;
};

/**
 * MainWindow class: the gallery.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void showPage(int index);

public slots:
    void onLowBattery();
    void onDeadline();
    void onFilesEaten(const QStringList &paths);
    void onCookieAccepted();
    void onCookieDeclined();
    void onVolume(int value);
    void onEmptyAction();
    void onStrength(int strength);
    void onBell(int count);
    void onRated(int rating);
    void onDeleted();
    void onChickened(qreal progress);
    void onLoaded();
    void onPoked(int count);
    void onAngry();
    void onToggled(const QString &name, bool on);
    void onMood(const QString &mood);
    void onCaught(int escapes);

private slots:
    void onRetry();
    void batteryStep();
    void sendFlock();
    void notify();
    void advance();

private:
    QWidget *buildSecondPage();
    QWidget *buildThirdPage();
    QWidget *buildFourthPage();
    QWidget *buildFifthPage();
    QWidget *buildSixthPage();
    QWidget *buildSeventhPage();
    int findFiles(const QString &text);
    void themeTabs(const QColor &color);

    QLabel *m_status = nullptr;
    PigeonToast *m_toast = nullptr;
    QStackedWidget *m_pages = nullptr;
    QButtonGroup *m_navGroup = nullptr;
    QTimer *m_progressTimer = nullptr;
    QTimer *m_bellTimer = nullptr;
    QTimer *m_batteryTimer = nullptr;
    QTimer *m_pagerTimer = nullptr;
    // Page 1.
    GooglyEyesWidget *m_eyes = nullptr;
    MoodSliderWidget *m_mood = nullptr;
    RunawayButtonWidget *m_runner = nullptr;
    HamsterLoaderWidget *m_hamster = nullptr;
    // Page 2.
    PasswordJudgeWidget *m_judge = nullptr;
    PanicBellWidget *m_bell = nullptr;
    StarRatingWidget *m_stars = nullptr;
    NervousHoldButtonWidget *m_holdBtn = nullptr;
    RubberDuckWidget *m_duck = nullptr;
    // Page 3.
    CookieConsentWidget *m_cookie = nullptr;
    VolumeKnobWidget *m_knob = nullptr;
    TypingIndicatorWidget *m_typing = nullptr;
    EmptyStateWidget *m_empty = nullptr;
    // Page 4.
    DeadlineCountdownWidget *m_deadline = nullptr;
    BatteryBuddyWidget *m_battery = nullptr;
    DropZoneMonsterWidget *m_dropzone = nullptr;
    QList<PresenceAvatarWidget *> m_avatars;
    DucklingPagerWidget *m_pager = nullptr;
    // Page 5.
    SearchDetectiveWidget *m_detective = nullptr;
    RobotCheckWidget *m_robot = nullptr;
    ChameleonColorPickerWidget *m_chameleon = nullptr;
    ErrorGremlinWidget *m_gremlin = nullptr;
    SnailStepperWidget *m_snail = nullptr;
    // Page 6.
    PancakeStepperWidget *m_pancakes = nullptr;
    HeartLikeButtonWidget *m_heart = nullptr;
    QList<SleepyCatSectionWidget *> m_catSections;
    RocketBackToTop *m_rocket = nullptr;
    // Page 7.
    DachshundTabsWidget *m_dogTabs = nullptr;
    QList<HelpOwlWidget *> m_owls;
    AntBreadcrumbWidget *m_crumbs = nullptr;
    PufferCounterWidget *m_puffer = nullptr;
    AutosaveSquirrelWidget *m_squirrel = nullptr;
};

#endif // QFUNNYWIDGETS_MAINWINDOW_H
