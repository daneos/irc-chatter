// This file is part of IRC Chatter, the first IRC Client for MeeGo.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Copyright (C) 2011, Timur Kristóf <venemo@fedoraproject.org>
// Copyright (C) 2011, Hiemanshu Sharma <mail@theindiangeek.in>

#include <QtCore>
#include <QStringList>

#include "ircmodel.h"
#include "appsettings.h"
#include "clients/communiircclient.h"

static bool channelLessThan(ChannelModel * m1, ChannelModel *m2)
{
    if (m1->channelType() == m2->channelType())
        return m1->name() < m2->name();

    return m1->channelType() < m2->channelType();
}

IrcModel::IrcModel(QObject *parent) :
    QObject(parent),
    _currentChannelIndex(-1),
    _networkConfigurationManager(new QNetworkConfigurationManager(this)),
    _networkSession(0)
{
    connect(_networkConfigurationManager, SIGNAL(onlineStateChanged(bool)), this, SLOT(onlineStateChanged(bool)));
}

void IrcModel::connectToServer(ServerSettings *server, AppSettings *settings)
{
    attemptConnection();

    if (_networkConfigurationManager->isOnline())
    {
        AbstractIrcClient *ircClient = new CommuniIrcClient(server->serverUrl(), this, server, settings);
        ServerModel *serverModel = new ServerModel(this, server->serverUrl(), ircClient);

        serverModel->_autoJoinChannels = server->autoJoinChannels();
        _servers.append(serverModel);

        connect(ircClient, SIGNAL(connectedToServer()), this, SLOT(backendsConnectedToServer()));
        connect(serverModel, SIGNAL(channelsChanged()), this, SLOT(refreshChannelList()));

        ircClient->connectToServer();
    }
    else
    {
        // Putting this IRC connection to the waiting queue
        _queue.append(IrcSettingPair(server, settings));
        setIsWaitingForConnection(true);
    }
}

void IrcModel::refreshChannelList()
{
    QList<ChannelModel*> *allChannelsList = new QList<ChannelModel*>(),
            *oldChannelsList = &_allChannels.getList();

    foreach (ServerModel *server, _servers)
    {
        QList<ChannelModel*> serverChannels = server->_channels.values();
        qSort(serverChannels.begin(), serverChannels.end(), channelLessThan);
        *allChannelsList += serverChannels;
    }

    _allChannels.setList(allChannelsList);
    delete oldChannelsList;
}

void IrcModel::backendsConnectedToServer()
{
    if (_currentChannelIndex == -1)
    {
        qDebug() << "this is the first connection, and it succeeded";
        setCurrentChannelIndex(0);
        emit readyToDisplay();
    }
}

bool IrcModel::isOnline() const
{
    return _networkConfigurationManager->isOnline();
}

void IrcModel::attemptConnection()
{
    // Deleting the previous connection object
    if (_networkSession)
        _networkSession->deleteLater();

    qDebug() << "attempting connection";

    // Trying to establish a connection
    const bool canConnect = (_networkConfigurationManager->capabilities() & QNetworkConfigurationManager::CanStartAndStopInterfaces);
    QNetworkConfiguration config = _networkConfigurationManager->defaultConfiguration();
    if (config.isValid() && canConnect)
    {
        _networkSession = new QNetworkSession(config, this);
        connect(_networkSession, SIGNAL(stateChanged(QNetworkSession::State)), this, SLOT(networkSessionStateChanged(QNetworkSession::State)));
        connect(_networkSession, SIGNAL(error(QNetworkSession::SessionError)), this, SLOT(networkSessionError(QNetworkSession::SessionError)));
        qDebug() << "opening new network session";
        _networkSession->open();
    }
}

void IrcModel::attemptConnectionLater()
{
    qDebug() << "attempting automatic reconnection in 5 seconds";
    // After 5 seconds, attempting a connection
    QTimer::singleShot(5000, this, SLOT(attemptConnection()));
}

void IrcModel::onlineStateChanged(bool online)
{
    if (online)
    {
        // If there are any connections waiting, let's connect them now

        // Queued, not yet connected
        if (_queue.count())
        {
            setIsWaitingForConnection(false);

            foreach (const IrcSettingPair &pair, _queue)
            {
                _queue.removeAll(pair);
                connectToServer(pair.first, pair.second);
            }
        }

        // Already connected but lost
        if (_servers.count())
        {
            foreach (ServerModel *server, _servers)
            {
                qDebug() << "reconnecting to server " << server->url();
                server->_ircClient->connectToServer();
            }
        }
    }
    else
    {
        qDebug() << "Network connection lost";

        foreach (ServerModel *server, _servers)
        {
            qDebug() << "disconnecting from server " << server->url();
            server->_ircClient->disconnectFromServer();
        }

        attemptConnectionLater();
    }

    emit isOnlineChanged();
}

void IrcModel::networkSessionError(QNetworkSession::SessionError error)
{
    qDebug() << "network session error";

    switch (error)
    {
    case QNetworkSession::UnknownSessionError:
    case QNetworkSession::RoamingError:
    case QNetworkSession::InvalidConfigurationError:
    case QNetworkSession::OperationNotSupportedError:
        // If we're not online, we should attempt a reconnection
        if (!_networkConfigurationManager->isOnline())
            attemptConnectionLater();
        break;
        // isOnlineChanged already takes care of this
    case QNetworkSession::SessionAbortedError:
    default:
        break;
    }
}

void IrcModel::networkSessionStateChanged(QNetworkSession::State state)
{
    switch (state)
    {
    // The connection is not available, should reconnect
    case QNetworkSession::NotAvailable:
    case QNetworkSession::Invalid:
        qDebug() << "network session is not available or invalid";
        attemptConnectionLater();
        break;
        // isOnlineChanged already takes care of these
    case QNetworkSession::Disconnected:
    case QNetworkSession::Connected:
    case QNetworkSession::Roaming:
        // isOnlineChanged already takes care of this when it's actually closed
    case QNetworkSession::Closing:
        // isOnlineChanged already takes care of this when it's actually connected
    case QNetworkSession::Connecting:
    default:
        break;
    }
}
