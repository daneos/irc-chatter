
TARGET = irc-chatter
TEMPLATE = app
VERSION = 0.3.2
QT += core declarative network

DEFINES += \
    COMMUNI_STATIC \
    HAVE_ICU \
    APP_VERSION=\\\"$$VERSION\\\"

include(communi.pri)

HEADERS += \
    util.h \
    appeventlistener.h \
    model/channelmodel.h \
    model/servermodel.h \
    model/ircmodel.h \
    qobjectlistmodel.h \
    model/settings/appsettings.h \
    model/settings/serversettings.h \
    model/clients/abstractircclient.h \
    model/clients/communiircclient.h \
    model/helpers/commandparser.h \
    model/helpers/channelhelper.h \
    model/helpers/notifier.h \
    model/helpers/channelmodelcollection.h

SOURCES += \
    main.cpp \
    appeventlistener.cpp \
    model/channelmodel.cpp \
    model/ircmodel.cpp \
    model/servermodel.cpp \
    model/settings/appsettings.cpp \
    model/settings/serversettings.cpp \
    model/clients/abstractircclient.cpp \
    model/clients/communiircclient.cpp \
    model/helpers/commandparser.cpp \
    model/helpers/channelhelper.cpp \
    model/helpers/notifier.cpp \
    model/helpers/channelmodelcollection.cpp

RESOURCES += \
    ui-meego.qrc

OTHER_FILES += \
    LICENSE \
    LICENSE-DOCS \
    AUTHORS \
    qml/meego/AppWindow.qml \
    qml/meego/components/TitleLabel.qml \
    qml/meego/components/WorkingSelectionDialog.qml \
    qml/meego/components/CommonDialog.qml \
    qml/meego/components/ServerSettingsList.qml \
    qml/meego/pages/StartPage.qml \
    qml/meego/pages/ChatPage.qml \
    qml/meego/pages/SettingsPage.qml \
    qml/meego/pages/ManageServersPage.qml \
    qml/meego/sheets/JoinSheet.qml \
    qml/meego/sheets/ServerSettingsSheet.qml

CONFIG += meegotouch

unix {
    QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
    QMAKE_LFLAGS += -pie -rdynamic

    QT += dbus
    INSTALLS += target icon desktopfile
    target.path=/usr/bin
    icon.files = installables/irc-chatter-harmattan-icon.png
    icon.path = /usr/share/icons/hicolor/80x80/apps
    desktopfile.files = installables/irc-chatter-harmattan.desktop
    desktopfile.path = /usr/share/applications
}

contains(MEEGO_EDITION, harmattan) {
    # for Harmattan
    DEFINES += MEEGO_EDITION_HARMATTAN HAVE_APPLAUNCHERD
    CONFIG += qdeclarative-boostable link_pkgconfig
    PKGCONFIG += qdeclarative-boostable
    INCLUDEPATH += /usr/include/applauncherd

    # Portrait and landscape splash screens
    splashes.files = installables/irc-chatter-splash-harmattan-portrait.jpg installables/irc-chatter-splash-harmattan-landscape.jpg
    splashes.path = /usr/share/irc-chatter
    # Notification icons
    notifyicons.files = installables/irc-chatter-harmattan-icon.png installables/irc-chatter-harmattan-lpm-icon.png installables/irc-chatter-harmattan-statusbar-icon.png
    notifyicons.path = /usr/share/themes/blanco/meegotouch/icons
    # Notification event type config
    notifyconfig.files = installables/irc-chatter.irc.conf
    notifyconfig.path = /usr/share/meegotouch/notifications/eventtypes

    INSTALLS = target splashes notifyicons icon notifyconfig desktopfile
}

QMAKE_CLEAN += Makefile build-stamp configure-stamp irc-chatter
