#ifndef SERVERMODEL_H
#define SERVERMODEL_H

#include <QObject>
#include "util.h"
#include "qobjectlistmodel.h"
#include "channelmodel.h"

namespace Irc { class Session; }
class IrcModel;

class ServerModel : public QObject
{
    Q_OBJECT

    GENPROPERTY_R(QObjectListModel<ChannelModel>*, _channels, channels)
    Q_PROPERTY(QObject* channels READ channels NOTIFY channelsChanged)
    GENPROPERTY(QString, _url, url, setUrl, urlChanged)
    GENPROPERTY(QString, _password, password, setPassword, passwordChanged)

    Irc::Session *_backend;

    friend class IrcModel;

protected:
    explicit ServerModel(IrcModel *parent, const QString &url, Irc::Session *backend);

public:
    ~ServerModel();
    Q_INVOKABLE bool joinChannel(const QString &channelName);
    Q_INVOKABLE bool partChannel(const QString &channelName);
    Q_INVOKABLE bool queryUser(const QString &userName);
    Q_INVOKABLE bool closeUser(const QString &userName);

signals:
    void channelsChanged();
    void urlChanged();
    void passwordChanged();

private slots:
    void backendConnectedToServer();
    void receiveNumericMessageFromBackend(const QString &name, uint x, const QStringList &message);

};

#endif // SERVERMODEL_H
