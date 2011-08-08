#include <QtGui/QApplication>
#include <QtDeclarative>

#include "model/ircmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    IrcModel model(&app);
    model.fillWithDummyData();

    qmlRegisterUncreatableType<ChannelModel>("net.venemo.ircchatter", 1, 0, "ChannelModel", "This object is created in the model.");
    qmlRegisterUncreatableType<MessageModel>("net.venemo.ircchatter", 1, 0, "MessageModel", "This object is created in the model.");
    qmlRegisterUncreatableType<IrcModel>("net.venemo.ircchatter", 1, 0, "IrcModel", "This object is created in the model.");

    QDeclarativeView view;
    view.rootContext()->setContextProperty("ircModel", &model);
    view.setSource(QUrl("qrc:/qml/harmattan/main.qml"));
    view.showFullScreen();

    return app.exec();
}
