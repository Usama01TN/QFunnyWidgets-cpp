/**
 * Gallery of all QFunnyWidgets, split over seven pages.
 */
#include "mainwindow.h"
#include "QFunnyWidgets.h"

#include <QButtonGroup>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHash>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>

namespace {

const QStringList &pigeonMessages()
{
    static const QStringList PIGEON_MESSAGES = {
        QStringLiteral("File saved successfully."), QStringLiteral("Your build passed! (Somehow.)"),
        QStringLiteral("3 new comments on your pull request."), QStringLiteral("Meeting moved to 4 pm. Again."),
        QStringLiteral("Backup complete. Your data is safe-ish."),
        QStringLiteral("Reminder: drink water. Signed, the pigeon.")};
    return PIGEON_MESSAGES;
}

QString randomPigeonMessage()
{
    const QStringList &list = pigeonMessages();
    return list.at(int(QRandomGenerator::global()->bounded(int(list.size()))));
}

const QColor BACKGROUND(0x12, 0x13, 0x18);
const QColor TEXT(0xEC, 0xED, 0xF2);

const char *const STYLE = R"CSS(
QMainWindow, #root { background: #121318; }
QFrame#card { background: #1C1E26; border: 1px solid #2A2D38; border-radius: 18px; }
QLabel { color: #ECEDF2; background: transparent; }
QLabel#cardTitle { font-size: 14pt; font-weight: 700; }
QLabel#cardSub { color: #8B8FA3; font-size: 9pt; }
QLabel#appTitle { font-size: 24pt; font-weight: 800; }
QLabel#appSub { color: #8B8FA3; font-size: 11pt; }
QLabel#status { color: #A5A8FF; font-size: 10pt; }
QLabel#pill { background: #2A2D38; color: #C9CBD6; border-radius: 10px; padding: 3px 10px; font-size: 8.5pt; }
QPushButton#nav { background: transparent; color: #8B8FA3; border: 1px solid #2A2D38; border-radius: 16px;
                  padding: 6px 16px; font-weight: 600; font-size: 11pt; }
QPushButton#nav:hover { color: #ECEDF2; border-color: #4A4E5E; }
QPushButton#nav:checked { background: #7B7FFF; color: #FFFFFF; border-color: #7B7FFF; }
QLineEdit { background: #14151B; color: #ECEDF2; border: 1px solid #343846; border-radius: 10px;
            padding: 6px 12px; selection-background-color: #7B7FFF; font-size: 11pt; }
QLineEdit:focus { border-color: #7B7FFF; }
QPushButton#passwordJudgeToggle { background: #2A2D38; color: #ECEDF2; border: none; border-radius: 10px;
                                  padding: 6px 16px; font-weight: 600; font-size: 10.5pt; }
QPushButton#passwordJudgeToggle:checked { background: #7B7FFF; }
QPushButton#action { background: #7B7FFF; color: #FFFFFF; border: none; border-radius: 12px;
                     padding: 8px 18px; font-weight: 700; font-size: 10.5pt; }
QPushButton#action:hover { background: #9295FF; }
QPushButton#ghost { background: transparent; color: #C9CBD6; border: 1px solid #343846; border-radius: 12px;
                    padding: 8px 18px; font-weight: 600; font-size: 10.5pt; }
QPushButton#ghost:hover { border-color: #7B7FFF; color: #FFFFFF; }
QLabel#chatThem { background: #2A2D38; color: #ECEDF2; border-radius: 12px; padding: 8px 12px; }
QLabel#chatMe { background: #7B7FFF; color: #FFFFFF; border-radius: 12px; padding: 8px 12px; }
QLabel#slide { background: #262935; color: #ECEDF2; border-radius: 14px; font-size: 15pt; font-weight: 700; }
QLabel#avatarName { color: #8B8FA3; font-size: 8.5pt; }
QLabel#post { color: #ECEDF2; font-size: 11pt; }
QLabel#changelog { color: #C9CBD6; font-size: 9.5pt; }
QLabel#sectionText { color: #C9CBD6; font-size: 9.5pt; }
QScrollArea#feed { background: transparent; border: none; }
QScrollArea#feed > QWidget > QWidget { background: transparent; }
QScrollBar:vertical { background: #1C1E26; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #3A3E4C; border-radius: 5px; min-height: 30px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
QPlainTextEdit { background: #14151B; color: #ECEDF2; border: 1px solid #343846; border-radius: 10px;
                 padding: 6px; font-size: 10.5pt; selection-background-color: #7B7FFF; }
QPlainTextEdit:focus { border-color: #7B7FFF; }
QLabel#formLabel { color: #C9CBD6; font-size: 9.5pt; }
QLabel#tabPage { background: #262935; color: #ECEDF2; border-radius: 14px; font-size: 13pt; font-weight: 600; }
)CSS";

} // namespace

// ======================================================================================================
// Card
// ======================================================================================================
Card::Card(const QString &title, const QString &subtitle, const QString &tag, const QString &logo, QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("card"));
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 120));
    setGraphicsEffect(shadow);
    QHBoxLayout *head = new QHBoxLayout();
    const QString logoPath = !logo.isEmpty() ? QStringLiteral(":/QFunnyWidgets/%1/logo.png").arg(logo) : QString();
    if (!logoPath.isEmpty() && QFile::exists(logoPath)) {
        QLabel *icon = new QLabel();
        const QPixmap pix(logoPath);
        icon->setPixmap(pix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon->setFixedSize(42, 42);
        head->addWidget(icon, 0, Qt::AlignTop);
    }
    QVBoxLayout *texts = new QVBoxLayout();
    texts->setSpacing(2);
    QLabel *t = new QLabel(title);
    t->setObjectName(QStringLiteral("cardTitle"));
    QLabel *s = new QLabel(subtitle);
    s->setObjectName(QStringLiteral("cardSub"));
    s->setWordWrap(true);
    texts->addWidget(t);
    texts->addWidget(s);
    head->addLayout(texts, 1);
    if (!tag.isEmpty()) {
        QLabel *pill = new QLabel(tag);
        pill->setObjectName(QStringLiteral("pill"));
        head->addWidget(pill, 0, Qt::AlignTop);
    }
    m_body = new QVBoxLayout();
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->addLayout(head);
    layout->addLayout(m_body, 1);
}

QVBoxLayout *Card::body() const
{
    return m_body;
}

// ======================================================================================================
// MainWindow
// ======================================================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("QFunnyWidgets - Gallery"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/googlyeyes/logo.png")));
    resize(1180, 760);
    setStyleSheet(QString::fromLatin1(STYLE));
    QPalette pal = palette();
    pal.setColor(QPalette::Window, BACKGROUND);
    pal.setColor(QPalette::WindowText, TEXT);
    pal.setColor(QPalette::Text, TEXT);
    setPalette(pal);
    // Header.
    QLabel *title = new QLabel(tr("QFunnyWidgets"));
    title->setObjectName(QStringLiteral("appTitle"));
    QLabel *sub = new QLabel(tr("Seriously professional. Professionally unserious."));
    sub->setObjectName(QStringLiteral("appSub"));
    m_status = new QLabel(tr("Go ahead, poke things."));
    m_status->setObjectName(QStringLiteral("status"));
    QHBoxLayout *header = new QHBoxLayout();
    QVBoxLayout *htxt = new QVBoxLayout();
    htxt->addWidget(title);
    htxt->addWidget(sub);
    header->addLayout(htxt, 1);
    header->addWidget(m_status, 0, Qt::AlignBottom);
    // Googly eyes.
    Card *eyesCard = new Card(tr("Googly Eyes"), tr("Wobbly pupils follow your cursor. Click to poke; poke fast to annoy."),
                              QStringLiteral("physics"), QStringLiteral("googlyeyes"));
    GooglyEyesWidget::Options eo;
    eo.eyeCount = 2;
    eo.lidColor = QColor(QStringLiteral("#E9A98F"));
    eo.poked = [this](int count) { onPoked(count); };
    eo.gotAngry = [this]() { onAngry(); };
    m_eyes = new GooglyEyesWidget(eo);
    eyesCard->body()->addWidget(m_eyes);
    // Jelly switches.
    Card *jellyCard = new Card(tr("Jelly Switch"), tr("Sleeps when off, wakes up delighted when on."),
                               QStringLiteral("toggle"), QStringLiteral("jellyswitch"));
    QHBoxLayout *row = new QHBoxLayout();
    const struct { QString label; const char *color; bool checked; } switches[] = {
        {tr("Wi-Fi"), "#34C759", true}, {tr("Dark mode"), "#5E60CE", false}, {tr("Party"), "#FF4D6D", false}};
    for (const auto &spec : switches) {
        QVBoxLayout *col = new QVBoxLayout();
        JellySwitchWidget::Options so;
        so.checked = spec.checked;
        so.onColor = QColor(QString::fromLatin1(spec.color));
        so.offColor = QColor(QStringLiteral("#4A4E5E"));
        so.knobColor = QColor(QStringLiteral("#F7F7FA"));
        const QString name = spec.label;
        so.toggled = [this, name](bool on) { onToggled(name, on); };
        JellySwitchWidget *sw = new JellySwitchWidget(so);
        sw->setMinimumSize(96, 64);
        QLabel *lbl = new QLabel(spec.label);
        lbl->setObjectName(QStringLiteral("cardSub"));
        lbl->setAlignment(Qt::AlignCenter);
        col->addWidget(sw, 1);
        col->addWidget(lbl);
        row->addLayout(col);
    }
    jellyCard->body()->addLayout(row);
    // Mood slider.
    Card *moodCard = new Card(tr("Mood Slider"), tr("How do you feel about Mondays? Be honest."), QStringLiteral("input"),
                              QStringLiteral("moodslider"));
    MoodSliderWidget::Options mo;
    mo.value = 35;
    mo.trackColor = QColor(255, 255, 255, 30);
    mo.moodChanged = [this](const QString &mood) { onMood(mood); };
    m_mood = new MoodSliderWidget(mo);
    m_mood->setMinimumHeight(110);
    moodCard->body()->addStretch(1);
    moodCard->body()->addWidget(m_mood);
    moodCard->body()->addStretch(1);
    // Runaway button.
    Card *runCard = new Card(tr("Runaway Button"), tr("The submit button your users deserve. Gives up after 7 escapes."),
                             QStringLiteral("button"), QStringLiteral("runawaybutton"));
    RunawayButtonWidget::Options ro;
    ro.text = tr("Submit");
    ro.color = QColor(QStringLiteral("#FF4D6D"));
    ro.maxEscapes = 7;
    ro.caught = [this](int escapes) { onCaught(escapes); };
    m_runner = new RunawayButtonWidget(ro);
    runCard->body()->addWidget(m_runner);
    // Hamster loader.
    Card *hamCard = new Card(tr("Hamster Loader"), tr("Renewable-energy progress indicator."), QStringLiteral("progress"),
                             QStringLiteral("hamsterloader"));
    HamsterLoaderWidget::Options ho;
    ho.progress = 0.0;
    ho.wheelColor = QColor(QStringLiteral("#6C7186"));
    ho.accentColor = QColor(QStringLiteral("#7B7FFF"));
    ho.finished = [this]() { onLoaded(); };
    m_hamster = new HamsterLoaderWidget(ho);
    hamCard->body()->addWidget(m_hamster);
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &MainWindow::advance);
    m_progressTimer->start(120);
    // Grid.
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(20);
    grid->addWidget(eyesCard, 0, 0);
    grid->addWidget(jellyCard, 0, 1);
    grid->addWidget(moodCard, 0, 2);
    grid->addWidget(runCard, 1, 0, 1, 2);
    grid->addWidget(hamCard, 1, 2);
    grid->setRowStretch(0, 4);
    grid->setRowStretch(1, 5);
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    QWidget *page1 = new QWidget();
    page1->setLayout(grid);
    grid->setContentsMargins(0, 0, 0, 0);
    // Page 2: the second batch.
    QWidget *page2 = buildSecondPage();
    QWidget *page3 = buildThirdPage();
    QWidget *page4 = buildFourthPage();
    QWidget *page5 = buildFifthPage();
    QWidget *page6 = buildSixthPage();
    QWidget *page7 = buildSeventhPage();
    m_pages = new QStackedWidget();
    for (QWidget *page : {page1, page2, page3, page4, page5, page6, page7})
        m_pages->addWidget(page);
    // Navigation pills.
    QHBoxLayout *nav = new QHBoxLayout();
    nav->setSpacing(8);
    m_navGroup = new QButtonGroup(this);
    const QString navNames[] = {tr("Classics"), tr("New arrivals"), tr("Fresh batch"), tr("Daily drivers"),
                                tr("Power tools"), tr("Finishing touches"), tr("Little details")};
    int i = 0;
    for (const QString &name : navNames) {
        QPushButton *b = new QPushButton(name);
        b->setObjectName(QStringLiteral("nav"));
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setChecked(i == 0);
        m_navGroup->addButton(b, i);
        nav->addWidget(b);
        ++i;
    }
    nav->addStretch(1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_navGroup, &QButtonGroup::idClicked, m_pages, &QStackedWidget::setCurrentIndex);
#else
    connect(m_navGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), m_pages, &QStackedWidget::setCurrentIndex);
#endif
    QWidget *root = new QWidget();
    root->setObjectName(QStringLiteral("root"));
    QVBoxLayout *layout = new QVBoxLayout(root);
    layout->setContentsMargins(28, 22, 28, 28);
    layout->setSpacing(16);
    layout->addLayout(header);
    layout->addLayout(nav);
    layout->addWidget(m_pages, 1);
    setCentralWidget(root);
    // The pigeon flies over the whole window.
    PigeonToast::Options to;
    to.bannerColor = QColor(QStringLiteral("#F7F7FA"));
    to.pigeonColor = QColor(QStringLiteral("#A9B4C2"));
    m_toast = new PigeonToast(root, to);
    // Stylesheets stop palette propagation, so give painted widgets the theme text color directly.
    QList<QWidget *> painted = {m_eyes, m_mood, m_runner, m_hamster, m_stars, m_holdBtn, m_bell, m_duck,
                                m_judge->getFace(), m_typing, m_empty, m_knob, m_deadline, m_battery, m_dropzone,
                                m_pager, m_detective->getFace(), m_robot, m_chameleon, m_gremlin, m_snail,
                                m_pancakes, m_heart, m_dogTabs, m_puffer->getFooter(), m_crumbs, m_squirrel};
    for (PresenceAvatarWidget *a : std::as_const(m_avatars))
        painted.append(a);
    for (HelpOwlWidget *o : std::as_const(m_owls))
        painted.append(o);
    for (JellySwitchWidget *sw : findChildren<JellySwitchWidget *>())
        painted.append(sw);
    for (QWidget *wdg : std::as_const(painted)) {
        QPalette wp = wdg->palette();
        wp.setColor(QPalette::Text, TEXT);
        wp.setColor(QPalette::WindowText, TEXT);
        wp.setColor(QPalette::Window, QColor(QStringLiteral("#1C1E26")));
        wdg->setPalette(wp);
    }
}

void MainWindow::showPage(int index)
{
    if (QAbstractButton *btn = m_navGroup->button(index))
        btn->setChecked(true);
    m_pages->setCurrentIndex(index);
}

QWidget *MainWindow::buildSecondPage()
{
    // Password judge.
    Card *judgeCard = new Card(tr("Password Judge"), tr("A live password review by a very qualified critic."),
                               QStringLiteral("form"), QStringLiteral("passwordjudge"));
    PasswordJudgeWidget::Options jo;
    jo.placeholder = tr("Choose a password (he is watching)");
    jo.strengthChanged = [this](int strength) { onStrength(strength); };
    m_judge = new PasswordJudgeWidget(jo);
    judgeCard->body()->addWidget(m_judge);
    // Panic bell.
    Card *bellCard = new Card(tr("Panic Bell"), tr("Notifications arrive on their own. Click to clear before it melts down."),
                              QStringLiteral("badge"), QStringLiteral("panicbell"));
    PanicBellWidget::Options bo;
    bo.count = 2;
    bo.color = QColor(QStringLiteral("#FFB703"));
    bo.countChanged = [this](int count) { onBell(count); };
    m_bell = new PanicBellWidget(bo);
    bellCard->body()->addWidget(m_bell);
    m_bellTimer = new QTimer(this);
    connect(m_bellTimer, &QTimer::timeout, this, &MainWindow::notify);
    m_bellTimer->start(1400);
    // Star rating.
    Card *starCard = new Card(tr("Star Rating"), tr("Every star has feelings. Choose wisely."), QStringLiteral("input"),
                              QStringLiteral("ratingstars"));
    StarRatingWidget::Options so;
    so.emptyColor = QColor(QStringLiteral("#4A4E5E"));
    so.ratingChanged = [this](int rating) { onRated(rating); };
    m_stars = new StarRatingWidget(so);
    starCard->body()->addWidget(m_stars);
    // Nervous hold button.
    Card *holdCard = new Card(tr("Nervous Hold Button"), tr("Hold-to-confirm for destructive actions. It is not ok."),
                              QStringLiteral("button"), QStringLiteral("holdbutton"));
    NervousHoldButtonWidget::Options ho;
    ho.confirmed = [this]() { onDeleted(); };
    ho.cancelled = [this](qreal progress) { onChickened(progress); };
    m_holdBtn = new NervousHoldButtonWidget(ho);
    holdCard->body()->addWidget(m_holdBtn);
    // Rubber duck.
    Card *duckCard = new Card(tr("Rubber Duck"), tr("Senior debugging consultant. Click for wisdom."),
                              QStringLiteral("mascot"), QStringLiteral("rubberduck"));
    RubberDuckWidget::Options dko;
    dko.waterColor = QColor(QStringLiteral("#3A86FF"));
    m_duck = new RubberDuckWidget(dko);
    duckCard->body()->addWidget(m_duck);
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(20);
    grid->addWidget(judgeCard, 0, 0, 1, 2);
    grid->addWidget(bellCard, 0, 2);
    grid->addWidget(starCard, 1, 0);
    grid->addWidget(holdCard, 1, 1);
    grid->addWidget(duckCard, 1, 2);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    QWidget *page = new QWidget();
    page->setLayout(grid);
    return page;
}

QWidget *MainWindow::buildThirdPage()
{
    // Cookie consent.
    Card *cookieCard = new Card(tr("Cookie Consent"), tr("GDPR compliant. Emotionally, less so."), QStringLiteral("banner"),
                                QStringLiteral("cookieconsent"));
    CookieConsentWidget::Options co;
    co.panelColor = QColor(QStringLiteral("#262935"));
    co.textColor = QColor(QStringLiteral("#ECEDF2"));
    co.accentColor = QColor(QStringLiteral("#7B7FFF"));
    co.autoReset = 4500;
    co.accepted = [this]() { onCookieAccepted(); };
    co.declined = [this]() { onCookieDeclined(); };
    m_cookie = new CookieConsentWidget(co);
    cookieCard->body()->addWidget(m_cookie);
    // Volume knob.
    Card *knobCard = new Card(tr("Volume Knob"), tr("Drag, scroll or double-click to mute."), QStringLiteral("dial"),
                              QStringLiteral("volumeknob"));
    VolumeKnobWidget::Options ko;
    ko.value = 55;
    ko.knobColor = QColor(QStringLiteral("#DADCE5"));
    ko.trackColor = QColor(255, 255, 255, 25);
    ko.valueChanged = [this](int value) { onVolume(value); };
    m_knob = new VolumeKnobWidget(ko);
    knobCard->body()->addWidget(m_knob);
    // Typing indicator in a tiny chat.
    Card *typingCard = new Card(tr("Typing Indicator"), tr("Shows what they are really doing."), QStringLiteral("chat"),
                                QStringLiteral("typingindicator"));
    QLabel *them = new QLabel(tr("Did you finish the report?"));
    them->setObjectName(QStringLiteral("chatThem"));
    QLabel *me = new QLabel(tr("Almost! One more tweak."));
    me->setObjectName(QStringLiteral("chatMe"));
    TypingIndicatorWidget::Options to;
    to.name = tr("Your manager");
    to.autoCycle = true;
    to.bubbleColor = QColor(QStringLiteral("#2A2D38"));
    to.dotColor = QColor(QStringLiteral("#9DA0FF"));
    m_typing = new TypingIndicatorWidget(to);
    m_typing->setMinimumHeight(80);
    typingCard->body()->addWidget(them, 0, Qt::AlignLeft);
    typingCard->body()->addWidget(me, 0, Qt::AlignRight);
    typingCard->body()->addWidget(m_typing);
    typingCard->body()->addStretch(1);
    // Empty state.
    Card *emptyCard = new Card(tr("Empty State"), tr("For lists that have nothing to say."), QStringLiteral("placeholder"),
                               QStringLiteral("emptystate"));
    EmptyStateWidget::Options eo;
    eo.title = tr("No projects yet");
    eo.actionText = tr("New project");
    eo.sandColor = QColor(QStringLiteral("#C9A55A"));
    eo.accentColor = QColor(QStringLiteral("#7B7FFF"));
    eo.actionClicked = [this]() { onEmptyAction(); };
    m_empty = new EmptyStateWidget(eo);
    emptyCard->body()->addWidget(m_empty);
    // Pigeon toast.
    Card *pigeonCard = new Card(tr("Carrier Pigeon Toast"),
                                tr("Notifications, delivered the traditional way. Watch the top-right corner."),
                                QStringLiteral("toast"), QStringLiteral("pigeontoast"));
    QPushButton *send = new QPushButton(tr("Send a pigeon"));
    send->setObjectName(QStringLiteral("action"));
    send->setCursor(Qt::PointingHandCursor);
    connect(send, &QPushButton::clicked, this, [this]() { m_toast->showMessage(randomPigeonMessage()); });
    QPushButton *flock = new QPushButton(tr("Send a flock (3)"));
    flock->setObjectName(QStringLiteral("ghost"));
    flock->setCursor(Qt::PointingHandCursor);
    connect(flock, &QPushButton::clicked, this, &MainWindow::sendFlock);
    QLabel *hint = new QLabel(tr("Messages queue up politely. The pigeon does not."));
    hint->setObjectName(QStringLiteral("cardSub"));
    hint->setWordWrap(true);
    pigeonCard->body()->addStretch(1);
    pigeonCard->body()->addWidget(send);
    pigeonCard->body()->addWidget(flock);
    pigeonCard->body()->addWidget(hint);
    pigeonCard->body()->addStretch(1);
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(20);
    grid->addWidget(cookieCard, 0, 0, 1, 2);
    grid->addWidget(knobCard, 0, 2, 2, 1);
    grid->addWidget(typingCard, 1, 0, 2, 1);
    grid->addWidget(emptyCard, 1, 1, 2, 1);
    grid->addWidget(pigeonCard, 2, 2);
    grid->setRowStretch(0, 4);
    grid->setRowStretch(1, 3);
    grid->setRowStretch(2, 3);
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    QWidget *page = new QWidget();
    page->setLayout(grid);
    return page;
}

QWidget *MainWindow::buildFourthPage()
{
    // Deadline.
    Card *deadlineCard = new Card(tr("Deadline Countdown"), tr("A 45 second deadline, on loop. Relatable."),
                                  QStringLiteral("timer"), QStringLiteral("deadline"));
    DeadlineCountdownWidget::Options dlo;
    dlo.duration = 45;
    dlo.title = tr("Quarterly report due");
    dlo.expired = [this]() { onDeadline(); };
    m_deadline = new DeadlineCountdownWidget(dlo);
    QPushButton *extend = new QPushButton(tr("Ask for an extension (+20 s)"));
    extend->setObjectName(QStringLiteral("ghost"));
    extend->setCursor(Qt::PointingHandCursor);
    connect(extend, &QPushButton::clicked, this, [this]() { m_deadline->extend(20); });
    deadlineCard->body()->addWidget(m_deadline, 1);
    deadlineCard->body()->addWidget(extend, 0, Qt::AlignRight);
    // Battery.
    Card *batteryCard = new Card(tr("Battery Buddy"), tr("Drains, complains, recharges. Like us."), QStringLiteral("status"),
                                 QStringLiteral("batterybuddy"));
    BatteryBuddyWidget::Options bo;
    bo.level = 100;
    bo.caseColor = QColor(QStringLiteral("#8B8FA3"));
    bo.lowBattery = [this]() { onLowBattery(); };
    m_battery = new BatteryBuddyWidget(bo);
    batteryCard->body()->addWidget(m_battery);
    m_batteryTimer = new QTimer(this);
    connect(m_batteryTimer, &QTimer::timeout, this, &MainWindow::batteryStep);
    m_batteryTimer->start(220);
    // Drop zone.
    Card *dropCard = new Card(tr("Drop Zone Monster"), tr("Drag files from your desktop. He eats images, PDFs and text."),
                              QStringLiteral("upload"), QStringLiteral("dropzone"));
    DropZoneMonsterWidget::Options dzo;
    dzo.acceptedExtensions = QStringList{QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"),
                                         QStringLiteral("gif"), QStringLiteral("pdf"), QStringLiteral("txt"),
                                         QStringLiteral("md")};
    dzo.accentColor = QColor(QStringLiteral("#7B7FFF"));
    dzo.filesDropped = [this](const QStringList &paths) { onFilesEaten(paths); };
    m_dropzone = new DropZoneMonsterWidget(dzo);
    dropCard->body()->addWidget(m_dropzone);
    // Presence avatars.
    Card *teamCard = new Card(tr("Presence Avatars"), tr("Hover to say hi. Click to change their status."),
                              QStringLiteral("identity"), QStringLiteral("presenceavatar"));
    QGridLayout *team = new QGridLayout();
    const struct { const char *name; int status; } members[] = {
        {"Ada Lovelace", PresenceAvatarWidget::Online}, {"Alan Turing", PresenceAvatarWidget::Away},
        {"Grace Hopper", PresenceAvatarWidget::Busy}, {"Charles Babbage", PresenceAvatarWidget::Offline}};
    int i = 0;
    for (const auto &m : members) {
        PresenceAvatarWidget::Options ao;
        ao.name = QString::fromLatin1(m.name);
        ao.status = m.status;
        ao.badgeBorderColor = QColor(QStringLiteral("#1C1E26"));
        PresenceAvatarWidget *av = new PresenceAvatarWidget(ao);
        connect(av, &PresenceAvatarWidget::clicked, av, [av]() { av->setStatus((int(av->getStatus()) + 1) % 4); });
        m_avatars.append(av);
        QLabel *lbl = new QLabel(ao.name);
        lbl->setObjectName(QStringLiteral("avatarName"));
        lbl->setAlignment(Qt::AlignCenter);
        team->addWidget(av, (i / 2) * 2, i % 2);
        team->addWidget(lbl, (i / 2) * 2 + 1, i % 2);
        ++i;
    }
    teamCard->body()->addLayout(team);
    // Duckling pager with an onboarding carousel.
    Card *pagerCard = new Card(tr("Duckling Pager"), tr("Carousel dots, but make it ducks. Click a lily pad."),
                               QStringLiteral("navigation"), QStringLiteral("ducklingpager"));
    QStackedWidget *slides = new QStackedWidget();
    for (const QString &text : {tr("Welcome aboard"), tr("Meet your dashboard"), tr("Invite your team"),
                                tr("Set your goals"), tr("You are ready!")}) {
        QLabel *sl = new QLabel(text);
        sl->setObjectName(QStringLiteral("slide"));
        sl->setAlignment(Qt::AlignCenter);
        slides->addWidget(sl);
    }
    DucklingPagerWidget::Options po;
    po.count = 5;
    po.ducklings = 3;
    po.padColor = QColor(QStringLiteral("#4FA36A"));
    po.currentChanged = [slides](int index) { slides->setCurrentIndex(index); };
    m_pager = new DucklingPagerWidget(po);
    m_pager->setMinimumHeight(80);
    pagerCard->body()->addWidget(slides, 1);
    pagerCard->body()->addWidget(m_pager);
    m_pagerTimer = new QTimer(this);
    connect(m_pagerTimer, &QTimer::timeout, m_pager, &DucklingPagerWidget::next);
    m_pagerTimer->start(3200);
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(20);
    grid->addWidget(deadlineCard, 0, 0, 1, 2);
    grid->addWidget(batteryCard, 0, 2);
    grid->addWidget(dropCard, 1, 0);
    grid->addWidget(teamCard, 1, 1);
    grid->addWidget(pagerCard, 1, 2);
    grid->setRowStretch(0, 2);
    grid->setRowStretch(1, 3);
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    QWidget *page = new QWidget();
    page->setLayout(grid);
    return page;
}

QWidget *MainWindow::buildFifthPage()
{
    // Search detective.
    Card *searchCard = new Card(tr("Search Detective"), tr("Try \"report\", \"cat\" or \"xyz\"."), QStringLiteral("search"),
                                QStringLiteral("searchdetective"));
    SearchDetectiveWidget::Options so;
    so.placeholder = tr("Search files...");
    so.searchFunction = [this](const QString &text) { return findFiles(text); };
    m_detective = new SearchDetectiveWidget(so);
    searchCard->body()->addWidget(m_detective);
    // Robot check.
    Card *robotCard = new Card(tr("Robot Check"), tr("Human verification, administered by a robot."),
                               QStringLiteral("verify"), QStringLiteral("robotcheck"));
    RobotCheckWidget::Options ro;
    ro.panelColor = QColor(QStringLiteral("#262935"));
    ro.textColor = QColor(QStringLiteral("#ECEDF2"));
    ro.failChance = 0.25;
    ro.verified = [this]() { m_status->setText(tr("Verified human. Allegedly.")); };
    ro.failed = [this]() { m_status->setText(tr("Flagged as a robot. Beep boop.")); };
    m_robot = new RobotCheckWidget(ro);
    QPushButton *again = new QPushButton(tr("Reset"));
    again->setObjectName(QStringLiteral("ghost"));
    again->setCursor(Qt::PointingHandCursor);
    connect(again, &QPushButton::clicked, m_robot, &RobotCheckWidget::reset);
    robotCard->body()->addWidget(m_robot, 1);
    robotCard->body()->addWidget(again, 0, Qt::AlignRight);
    // Chameleon color picker: it also recolors the navigation pills.
    Card *chamCard = new Card(tr("Chameleon Picker"), tr("Pick a color. It also themes the tabs up there."),
                              QStringLiteral("color"), QStringLiteral("chameleonpicker"));
    ChameleonColorPickerWidget::Options co;
    co.color = QColor(QStringLiteral("#7B7FFF"));
    co.branchColor = QColor(QStringLiteral("#7A5232"));
    co.colors.clear();
    for (const char *c : {"#E63946", "#FF9F1C", "#FFD23F", "#9BDB4D", "#3BCEAC", "#4CC9F0", "#4361EE", "#7B7FFF",
                          "#C77DFF", "#FF70A6", "#8D6E63", "#6C757D"})
        co.colors.append(QColor(QString::fromLatin1(c)));
    co.colorChanged = [this](const QColor &color) { themeTabs(color); };
    m_chameleon = new ChameleonColorPickerWidget(co);
    chamCard->body()->addWidget(m_chameleon);
    // Error gremlin.
    Card *gremCard = new Card(tr("Error Gremlin"),
                              tr("Inline alerts with a culprit. Retry scares it off; pick a severity to bring it back."),
                              QStringLiteral("alert"), QStringLiteral("errorgremlin"));
    ErrorGremlinWidget::Options go;
    go.severity = ErrorGremlinWidget::Error;
    go.panelColor = QColor(QStringLiteral("#262935"));
    go.textColor = QColor(QStringLiteral("#ECEDF2"));
    go.title = tr("Connection lost");
    go.message = tr("We could not reach the server. Someone appears to be chewing the cable.");
    go.retryClicked = [this]() { onRetry(); };
    go.dismissed = [this]() { m_status->setText(tr("Alert dismissed. The gremlin is still out there.")); };
    m_gremlin = new ErrorGremlinWidget(go);
    m_gremlin->setMinimumHeight(110);
    QHBoxLayout *sevRow = new QHBoxLayout();
    const struct { int sev; QString label; QString title; QString msg; } alerts[] = {
        {ErrorGremlinWidget::Info, tr("Info"), tr("Heads up"),
         tr("Scheduled maintenance tonight at 2 am. The gremlin is resting up for it.")},
        {ErrorGremlinWidget::Warning, tr("Warning"), tr("Disk almost full"),
         tr("Only 3% space left. Something is hoarding cat pictures.")},
        {ErrorGremlinWidget::Error, tr("Error"), tr("Connection lost"),
         tr("We could not reach the server. Someone appears to be chewing the cable.")},
        {ErrorGremlinWidget::Critical, tr("Critical"), tr("Production is on fire"),
         tr("Literally. Please do not panic. Panic quietly.")}};
    for (const auto &a : alerts) {
        QPushButton *b = new QPushButton(a.label);
        b->setObjectName(QStringLiteral("ghost"));
        b->setCursor(Qt::PointingHandCursor);
        const int sev = a.sev;
        const QString t = a.title;
        const QString m = a.msg;
        connect(b, &QPushButton::clicked, this, [this, sev, t, m]() { m_gremlin->showAlert(sev, t, m); });
        sevRow->addWidget(b);
    }
    gremCard->body()->addWidget(m_gremlin);
    gremCard->body()->addStretch(1);
    gremCard->body()->addLayout(sevRow);
    // Snail stepper.
    Card *snailCard = new Card(tr("Snail Stepper"), tr("Checkout progress at a sustainable pace."),
                               QStringLiteral("wizard"), QStringLiteral("snailstepper"));
    SnailStepperWidget::Options sno;
    sno.steps = QStringList{tr("Cart"), tr("Shipping"), tr("Payment"), tr("Done")};
    sno.arrived = [this](int i) { m_status->setText(tr("The snail reached step %1.").arg(i + 1)); };
    m_snail = new SnailStepperWidget(sno);
    QHBoxLayout *nav = new QHBoxLayout();
    QPushButton *back = new QPushButton(tr("Back"));
    back->setObjectName(QStringLiteral("ghost"));
    connect(back, &QPushButton::clicked, m_snail, &SnailStepperWidget::previous);
    QPushButton *nxt = new QPushButton(tr("Next"));
    nxt->setObjectName(QStringLiteral("action"));
    connect(nxt, &QPushButton::clicked, m_snail, &SnailStepperWidget::next);
    for (QPushButton *b : {back, nxt})
        b->setCursor(Qt::PointingHandCursor);
    nav->addWidget(back);
    nav->addStretch(1);
    nav->addWidget(nxt);
    snailCard->body()->addWidget(m_snail, 1);
    snailCard->body()->addLayout(nav);
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(20);
    grid->addWidget(searchCard, 0, 0);
    grid->addWidget(robotCard, 0, 1);
    grid->addWidget(chamCard, 0, 2);
    grid->addWidget(gremCard, 1, 0, 1, 2);
    grid->addWidget(snailCard, 1, 2);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    QWidget *page = new QWidget();
    page->setLayout(grid);
    return page;
}

QWidget *MainWindow::buildSixthPage()
{
    // Pancakes.
    Card *panCard = new Card(tr("Pancake Stepper"), tr("A spin box, served warm. Wheel, arrows or +/-."),
                             QStringLiteral("quantity"), QStringLiteral("pancakestepper"));
    PancakeStepperWidget::Options po;
    po.value = 3;
    po.maximum = 8;
    po.accentColor = QColor(QStringLiteral("#C47F3A"));
    po.valueChanged = [this](int v) {
        m_status->setText(tr("Order: %1 pancake(s). $%2").arg(v).arg(v * 2.5, 0, 'f', 2));
    };
    m_pancakes = new PancakeStepperWidget(po);
    panCard->body()->addWidget(m_pancakes);
    // Heart like.
    Card *heartCard = new Card(tr("Heart Like Button"), tr("Liking is caring. Un-liking leaves a mark."),
                               QStringLiteral("social"), QStringLiteral("heartlike"));
    QLabel *post = new QLabel(tr("Just shipped a new feature on a Friday afternoon. What could possibly go wrong?"));
    post->setObjectName(QStringLiteral("post"));
    post->setWordWrap(true);
    HeartLikeButtonWidget::Options hlo;
    hlo.count = 41;
    hlo.color = QColor(QStringLiteral("#FF4D6D"));
    m_heart = new HeartLikeButtonWidget(hlo);
    heartCard->body()->addWidget(post);
    heartCard->body()->addWidget(m_heart, 1);
    // Copy blobs.
    Card *copyCard = new Card(tr("Copy Blob"), tr("Click a chip. The blob copies itself (and your text)."),
                              QStringLiteral("clipboard"), QStringLiteral("copyblob"));
    copyCard->body()->addStretch(1);
    const struct { const char *text; const char *label; const char *prompt; const char *color; } chips[] = {
        {"pip install funnywidgets", "", "$", "#9BDB4D"},
        {"sk-live-8f3a9c2e7b1d4f60", "sk-live-****************", "", "#FF9F1C"}};
    for (const auto &c : chips) {
        CopyBlobButtonWidget::Options cbo;
        cbo.text = QString::fromLatin1(c.text);
        cbo.label = QString::fromLatin1(c.label);
        cbo.prompt = QString::fromLatin1(c.prompt);
        cbo.blobColor = QColor(QString::fromLatin1(c.color));
        cbo.chipColor = QColor(QStringLiteral("#14151B"));
        cbo.copied = [this](const QString &t) {
            m_status->setText(tr("Copied to clipboard (%1 chars).").arg(t.toUcs4().size()));
        };
        CopyBlobButtonWidget *chip = new CopyBlobButtonWidget(cbo);
        chip->setMinimumHeight(84);
        copyCard->body()->addWidget(chip);
    }
    copyCard->body()->addStretch(1);
    // Cat sections.
    Card *catCard = new Card(tr("Sleepy Cat Sections"), tr("Collapsible panels. Please do not wake the cat. (Wake the cat.)"),
                             QStringLiteral("container"), QStringLiteral("catsection"));
    QLabel *adv = new QLabel(tr("Turbo mode: ON    Cache size: 1 tuna can\nTelemetry: off (the cat insisted)"));
    adv->setObjectName(QStringLiteral("sectionText"));
    QLabel *notes = new QLabel(tr("v6.0: Added pancakes, hearts, blobs, rockets and cats.\n"
                                  "Removed: one bug (it was a feature)."));
    notes->setObjectName(QStringLiteral("sectionText"));
    notes->setWordWrap(true);
    SleepyCatSectionWidget::Options ca;
    ca.title = tr("Advanced settings");
    ca.content = adv;
    ca.barColor = QColor(QStringLiteral("#262935"));
    ca.textColor = QColor(QStringLiteral("#ECEDF2"));
    ca.catColor = QColor(QStringLiteral("#F4A261"));
    SleepyCatSectionWidget::Options cb;
    cb.title = tr("Release notes");
    cb.content = notes;
    cb.barColor = QColor(QStringLiteral("#262935"));
    cb.textColor = QColor(QStringLiteral("#ECEDF2"));
    cb.catColor = QColor(QStringLiteral("#8D99AE"));
    cb.expanded = true;
    m_catSections = {new SleepyCatSectionWidget(ca), new SleepyCatSectionWidget(cb)};
    for (SleepyCatSectionWidget *sec : std::as_const(m_catSections)) {
        catCard->body()->addWidget(sec);
        // Accordion behavior: opening one section puts the other cat to sleep.
        connect(sec, &SleepyCatSectionWidget::toggled, this, [this, sec](bool on) {
            for (SleepyCatSectionWidget *o : std::as_const(m_catSections)) {
                if (on && o != sec)
                    o->setExpanded(false);
            }
        });
    }
    catCard->body()->addStretch(1);
    // Rocket back-to-top over a long changelog.
    Card *rocketCard = new Card(tr("Rocket Back-to-Top"), tr("Scroll down. Then press the rocket."),
                                QStringLiteral("navigation"), QStringLiteral("rockettop"));
    QStringList releases;
    for (int i = 0; i < 40; ++i)
        releases.append(tr("Release %1.0: Fixed a bug where the previous fix introduced a bug. Also made the rocket faster.")
                            .arg(40 - i));
    QLabel *log = new QLabel(releases.join(QStringLiteral("\n\n")));
    log->setObjectName(QStringLiteral("changelog"));
    log->setWordWrap(true);
    log->setAlignment(Qt::AlignTop);
    log->setContentsMargins(4, 4, 90, 4);
    QScrollArea *feed = new QScrollArea();
    feed->setObjectName(QStringLiteral("feed"));
    feed->setWidgetResizable(true);
    feed->setWidget(log);
    feed->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    RocketBackToTop::Options rko;
    rko.threshold = 200;
    rko.color = QColor(QStringLiteral("#7B7FFF"));
    rko.launched = [this]() { m_status->setText(tr("Houston, we have scrolled.")); };
    m_rocket = new RocketBackToTop(feed, rko);
    rocketCard->body()->addWidget(feed, 1);
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(20);
    grid->addWidget(panCard, 0, 0);
    grid->addWidget(heartCard, 0, 1);
    grid->addWidget(copyCard, 0, 2);
    grid->addWidget(catCard, 1, 0, 1, 2);
    grid->addWidget(rocketCard, 1, 2);
    grid->setRowStretch(0, 2);
    grid->setRowStretch(1, 3); // room for the sections to expand
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    QWidget *page = new QWidget();
    page->setLayout(grid);
    return page;
}

QWidget *MainWindow::buildSeventhPage()
{
    // Dachshund tabs over a little stack of pages.
    Card *tabsCard = new Card(tr("Dachshund Tabs"), tr("Segmented control. Jump from the first tab to the last."),
                              QStringLiteral("tabs"), QStringLiteral("dachshundtabs"));
    const QStringList names = {tr("Overview"), tr("Activity"), tr("Settings"), tr("Billing"), tr("Help")};
    QStackedWidget *pages = new QStackedWidget();
    for (const QString &text : {tr("All systems nominal. The dog is a good boy."), tr("3 new events. All of them were the dog."),
                                tr("Settings: everything set to \"very good boy\"."), tr("Invoice: 1 treat. Paid in full."),
                                tr("Help: have you tried belly rubs?")}) {
        QLabel *lbl = new QLabel(text);
        lbl->setObjectName(QStringLiteral("tabPage"));
        lbl->setAlignment(Qt::AlignCenter);
        pages->addWidget(lbl);
    }
    DachshundTabsWidget::Options dto;
    dto.tabs = names;
    dto.dogColor = QColor(QStringLiteral("#B5651D"));
    dto.trackColor = QColor(255, 255, 255, 18);
    dto.currentChanged = [pages](int index) { pages->setCurrentIndex(index); };
    m_dogTabs = new DachshundTabsWidget(dto);
    tabsCard->body()->addWidget(m_dogTabs);
    tabsCard->body()->addWidget(pages, 1);
    // Help owls on a small form.
    Card *owlCard = new Card(tr("Help Owl"), tr("Hover the owls for help. Click to pin."), QStringLiteral("tooltip"),
                             QStringLiteral("helpowl"));
    QGridLayout *form = new QGridLayout();
    const struct { QString label; const char *ph; QString help; } fields[] = {
        {tr("Workspace"), "acme-corp", tr("The name your whole team sees. Choose wisely; the owl is judging.")},
        {tr("Retention"), "30 days", tr("How long deleted items are kept. After that, the owl eats them.")},
        {tr("Webhook"), "https://...", tr("We POST JSON here whenever something happens. Hoo!")}};
    int r = 0;
    for (const auto &fd : fields) {
        QLabel *lbl = new QLabel(fd.label);
        lbl->setObjectName(QStringLiteral("formLabel"));
        QLineEdit *edit = new QLineEdit();
        edit->setPlaceholderText(QString::fromLatin1(fd.ph));
        HelpOwlWidget::Options oo;
        oo.helpText = fd.help;
        oo.title = fd.label;
        oo.owlColor = QColor(QStringLiteral("#A1887F"));
        HelpOwlWidget *owl = new HelpOwlWidget(oo);
        owl->setFixedSize(34, 40);
        m_owls.append(owl);
        form->addWidget(lbl, r, 0);
        form->addWidget(edit, r, 1);
        form->addWidget(owl, r, 2);
        ++r;
    }
    owlCard->body()->addLayout(form);
    owlCard->body()->addStretch(1);
    // Breadcrumb with "folders" to dive into.
    Card *crumbCard = new Card(tr("Ant Breadcrumb"), tr("Open folders below; click a crumb to go back up."),
                               QStringLiteral("navigation"), QStringLiteral("antbreadcrumb"));
    const QHash<QString, QStringList> tree = {
        {QStringLiteral("Home"), {QStringLiteral("Projects"), QStringLiteral("Photos")}},
        {QStringLiteral("Projects"), {QStringLiteral("QFunnyWidgets"), QStringLiteral("Secret plan")}},
        {QStringLiteral("QFunnyWidgets"), {QStringLiteral("src"), QStringLiteral("docs")}},
        {QStringLiteral("Photos"), {QStringLiteral("Cats"), QStringLiteral("More cats")}},
        {QStringLiteral("src"), {QStringLiteral("ants.py")}}};
    AntBreadcrumbWidget::Options abo;
    abo.path = QStringList{QStringLiteral("Home"), QStringLiteral("Projects")};
    abo.antColor = QColor(QStringLiteral("#D8C3A5"));
    m_crumbs = new AntBreadcrumbWidget(abo);
    QHBoxLayout *folders = new QHBoxLayout();
    crumbCard->body()->addWidget(m_crumbs);
    crumbCard->body()->addStretch(1);
    crumbCard->body()->addLayout(folders);
    auto refresh = [this, folders, tree](const QStringList &path) {
        while (folders->count()) {
            QLayoutItem *item = folders->takeAt(0);
            if (item->widget() != nullptr)
                item->widget()->deleteLater();
            delete item;
        }
        const QStringList children = tree.value(path.last());
        for (const QString &name : children) {
            QPushButton *b = new QPushButton(tr("Open %1").arg(name));
            b->setObjectName(QStringLiteral("ghost"));
            b->setCursor(Qt::PointingHandCursor);
            connect(b, &QPushButton::clicked, this, [this, name]() { m_crumbs->push(name); });
            folders->addWidget(b);
        }
        if (children.isEmpty()) {
            QLabel *lbl = new QLabel(tr("Empty folder. Just ants."));
            lbl->setObjectName(QStringLiteral("formLabel"));
            folders->addWidget(lbl);
        }
    };
    connect(m_crumbs, &AntBreadcrumbWidget::pathChanged, this, refresh);
    refresh(m_crumbs->getPath());
    // Puffer counter.
    Card *pufferCard = new Card(tr("Puffer Counter"), tr("A 100-character limit, enforced by a fish."),
                                QStringLiteral("input"), QStringLiteral("puffercounter"));
    PufferCounterWidget::Options pco;
    pco.maxLength = 100;
    pco.placeholder = tr("What's happening? (keep it short)");
    pco.limitExceeded = [this](bool o) {
        m_status->setText(o ? tr("Too long! The fish is furious.") : tr("Back under the limit."));
    };
    m_puffer = new PufferCounterWidget(pco);
    pufferCard->body()->addWidget(m_puffer);
    // Autosave squirrel.
    Card *saveCard = new Card(tr("Autosave Squirrel"), tr("Type, then pause. 20% of saves fail on purpose."),
                              QStringLiteral("status"), QStringLiteral("autosave"));
    QPlainTextEdit *notes = new QPlainTextEdit();
    notes->setPlaceholderText(tr("Your notes..."));
    AutosaveSquirrelWidget::Options aso;
    aso.autosaveDelay = 1200;
    aso.retryDelay = 2500;
    m_squirrel = new AutosaveSquirrelWidget(aso);
    connect(m_squirrel, &AutosaveSquirrelWidget::saveRequested, this, [this]() {
        QPointer<AutosaveSquirrelWidget> sq = m_squirrel;
        QTimer::singleShot(1300, this, [sq]() {
            if (sq)
                sq->saveFinished(QRandomGenerator::global()->generateDouble() > 0.2);
        });
    });
    connect(notes, &QPlainTextEdit::textChanged, m_squirrel, &AutosaveSquirrelWidget::markDirty);
    saveCard->body()->addWidget(notes, 1);
    saveCard->body()->addWidget(m_squirrel);
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(20);
    grid->addWidget(tabsCard, 0, 0, 1, 2);
    grid->addWidget(owlCard, 0, 2);
    grid->addWidget(crumbCard, 1, 0);
    grid->addWidget(pufferCard, 1, 1);
    grid->addWidget(saveCard, 1, 2);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    QWidget *page = new QWidget();
    page->setLayout(grid);
    return page;
}

int MainWindow::findFiles(const QString &text)
{
    static const QStringList files = {
        QStringLiteral("quarterly_report_final.pdf"), QStringLiteral("quarterly_report_final_v2.pdf"),
        QStringLiteral("report_FINAL_really.docx"), QStringLiteral("cat_photo_001.jpg"), QStringLiteral("cat_photo_002.jpg"),
        QStringLiteral("catalogue.xlsx"), QStringLiteral("budget.xlsx"), QStringLiteral("notes.txt")};
    int n = 0;
    for (const QString &f : files) {
        if (f.toLower().contains(text.toLower()))
            ++n;
    }
    m_status->setText(tr("Search for \"%1\": %2 result(s)").arg(text).arg(n));
    return n;
}

void MainWindow::themeTabs(const QColor &color)
{
    const QString css = QStringLiteral("QPushButton#nav:checked { background: %1; border-color: %1; }").arg(color.name());
    for (QAbstractButton *b : m_navGroup->buttons())
        b->setStyleSheet(css);
}

void MainWindow::onRetry()
{
    m_status->setText(tr("Retrying..."));
    QTimer::singleShot(1700, this, [this]() {
        m_gremlin->showAlert(ErrorGremlinWidget::Info, tr("Fixed! (for now)"),
                             tr("The gremlin has been relocated to a nice farm upstate."));
    });
}

void MainWindow::batteryStep()
{
    const int lv = m_battery->getLevel();
    if (m_battery->isCharging()) {
        if (lv >= 100)
            m_battery->setCharging(false);
        else
            m_battery->setLevel(lv + 2);
    } else if (lv <= 2) {
        m_battery->setCharging(true);
    } else {
        m_battery->setLevel(lv - 1);
    }
}

void MainWindow::onLowBattery()
{
    m_status->setText(tr("Battery low. It is being very dramatic about it."));
}

void MainWindow::onDeadline()
{
    m_status->setText(tr("Deadline missed. This is fine."));
    QTimer::singleShot(7000, m_deadline, [this]() { m_deadline->start(); });
}

void MainWindow::onFilesEaten(const QStringList &paths)
{
    m_toast->showMessage(tr("The monster ate %1 file(s). He says thanks.").arg(paths.size()));
}

void MainWindow::sendFlock()
{
    for (int i = 0; i < 3; ++i)
        m_toast->showMessage(randomPigeonMessage(), 1400);
}

void MainWindow::onCookieAccepted()
{
    m_status->setText(tr("Cookies accepted. The cookie was delicious."));
}

void MainWindow::onCookieDeclined()
{
    m_status->setText(tr("Cookies declined. Somewhere, a cookie weeps."));
}

void MainWindow::onVolume(int value)
{
    m_status->setText(tr("Volume %1%").arg(value));
}

void MainWindow::onEmptyAction()
{
    m_toast->showMessage(tr("Project created! (Not really. This is a demo.)"));
}

void MainWindow::notify()
{
    const int n = m_bell->getCount();
    m_bell->setCount(n + (n < 8 ? 1 : (n < 30 ? 3 : 12)));
}

void MainWindow::onStrength(int strength)
{
    static const char *const names[] = {"", "terrible", "weak", "fair", "good", "strong"};
    if (strength >= 0)
        m_status->setText(tr("Password strength: %1").arg(QString::fromLatin1(names[strength + 1])));
}

void MainWindow::onBell(int count)
{
    if (count == 0)
        m_status->setText(tr("Inbox zero. The bell can finally sleep."));
}

void MainWindow::onRated(int rating)
{
    m_status->setText(tr("Rated %1 star(s). The stars have been informed.").arg(rating));
}

void MainWindow::onDeleted()
{
    m_status->setText(tr("Deleted. The button needs a moment."));
}

void MainWindow::onChickened(qreal progress)
{
    m_status->setText(tr("Backed out at %1%. Wise.").arg(int(progress * 100)));
}

void MainWindow::advance()
{
    const qreal p = m_hamster->getProgress();
    if (p >= 1.0)
        return;
    m_hamster->setProgress(std::min(1.0, p + 0.012));
}

void MainWindow::onLoaded()
{
    m_status->setText(tr("Loading complete. Hamster requests a snack."));
    QTimer::singleShot(3500, m_hamster, &HamsterLoaderWidget::reset);
}

void MainWindow::onPoked(int count)
{
    m_status->setText(tr("Poke #%1. Rude.").arg(count));
}

void MainWindow::onAngry()
{
    m_status->setText(tr("The eyes are now angry. Happy?"));
}

void MainWindow::onToggled(const QString &name, bool on)
{
    m_status->setText(tr("%1 is %2").arg(name, on ? tr("awake") : tr("asleep")));
}

void MainWindow::onMood(const QString &mood)
{
    m_status->setText(tr("Current mood: %1").arg(mood));
}

void MainWindow::onCaught(int escapes)
{
    m_status->setText(tr("Submitted after only %1 escapes. Impressive.").arg(escapes));
}
