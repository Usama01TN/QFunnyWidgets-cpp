# qmake include file for the QFunnyWidgets library sources (Qt 6 or Qt 5.15).
#   include(path/to/QFunnyWidgets.pri)
QT += widgets
CONFIG += c++17
QFW_DIR = $$PWD/QFunnyWidgets
INCLUDEPATH += $$QFW_DIR
QFW_PACKAGES = googlyeyes runawaybutton jellyswitch hamsterloader moodslider ratingstars holdbutton panicbell \
    rubberduck passwordjudge cookieconsent typingindicator emptystate volumeknob pigeontoast dropzone batterybuddy \
    presenceavatar ducklingpager deadline searchdetective robotcheck chameleonpicker errorgremlin snailstepper \
    pancakestepper heartlike copyblob rockettop catsection dachshundtabs puffercounter helpowl antbreadcrumb autosave
HEADERS += $$QFW_DIR/QFunnyWidgets.h
for(pkg, QFW_PACKAGES) {
    HEADERS += $$QFW_DIR/$${pkg}/$${pkg}.h
    SOURCES += $$QFW_DIR/$${pkg}/$${pkg}.cpp
    RESOURCES += $$QFW_DIR/$${pkg}/$${pkg}.qrc
}
