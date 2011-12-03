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
#include <Irc>
#include <IrcSession>
#include <IrcSender>
#include <IrcUtil>
#include <IrcGlobal>
#include <IrcMessage>
#include <IrcCommand>

#include "servermodel.h"
#include "ircmodel.h"
#include "appsettings.h"

ServerModel::ServerModel(IrcModel *parent, const QString &url, IrcSession *backend) :
    QObject((QObject*)parent),
    _url(url),
    _backend(backend),
    _defaultChannel(0)
{
    _settings = new AppSettings(this);
    if (_backend)
    {
        connect(_backend, SIGNAL(connected()), this, SLOT(backendConnectedToServer()));
        connect(_backend, SIGNAL(disconnected()), this, SLOT(backendDisconnectedFromServer()));
        connect(_backend, SIGNAL(messageReceived(IrcMessage*)), this, SLOT(backendReceivedMessage(IrcMessage*)));
        _backend->setHost(url);
        _backend->open();
    }
}

ServerModel::~ServerModel()
{
    if (_backend)
    {
        _backend->sendCommand(IrcCommand::createQuit(_settings->quitMessage()));
        _backend->deleteLater();
    }
}

void ServerModel::backendConnectedToServer()
{
    qDebug() << "backend of " << url() << " is now connected to server";

    foreach (QString channelName, _autoJoinChannels)
        joinChannel(channelName);
}

void ServerModel::backendDisconnectedFromServer()
{
    qDebug() << "backend for " << url() << " has been disconnected from the server";
}

void ServerModel::backendReceivedMessage(IrcMessage *message)
{
    QString channelName;

    switch (message->type())
    {
    case IrcMessage::Private:
        channelName = ((IrcPrivateMessage*)message)->target().startsWith('#')
                ? ((IrcPrivateMessage*)message)->target() // This is a channel message
                : ((IrcPrivateMessage*)message)->sender().name(); // This is a private message

        if (((IrcPrivateMessage*)message)->isAction())
        {
            // This is a CTCP action
            findOrCreateChannel(channelName)->receiveCtcpAction(message->sender().name(), ((IrcPrivateMessage*)message)->message());
        }
        else if (((IrcPrivateMessage*)message)->isRequest())
        {
            // This is a CTCP request
            findOrCreateChannel(channelName)->receiveCtcpRequest(message->sender().name(), ((IrcPrivateMessage*)message)->message());
        }
        else
        {
            // This is a normal message
            findOrCreateChannel(channelName)->receiveMessage(message->sender().name(), ((IrcPrivateMessage*)message)->message());
        }
        break;
    case IrcMessage::Join:
        // This is a join message
        if (_channels[((IrcJoinMessage*)message)->channel()])
            _channels[((IrcJoinMessage*)message)->channel()]->receiveJoined(message->sender().name());
        break;
    case IrcMessage::Part:
        // This is a part message
        if (_channels[((IrcPartMessage*)message)->channel()])
            _channels[((IrcPartMessage*)message)->channel()]->receiveParted(message->sender().name(), ((IrcPartMessage*)message)->reason());
        break;
    case IrcMessage::Nick:
        // This is a nick change message
        // TODO: display this in all channels the user is on.
        break;
    case IrcMessage::Quit:
        // This is a quit message
        // TODO: display this in all channels the user is on.
        break;
    case IrcMessage::Invite:
        // This is an invite message
        // TODO
        break;
    case IrcMessage::Topic:
        // This is a topic message
        if (_channels[((IrcTopicMessage*)message)->channel()])
            _channels[((IrcTopicMessage*)message)->channel()]->receiveTopic(((IrcTopicMessage*)message)->topic());
        break;
    case IrcMessage::Notice:
        if (((IrcNoticeMessage*)message)->target().startsWith('#'))
        {
            // This is a channel message
            findOrCreateChannel(((IrcNoticeMessage*)message)->target())->receiveMessage(message->sender().name(), ((IrcNoticeMessage*)message)->message());
        }
        else
        {
            // This is a private message
            findOrCreateChannel(message->sender().name())->receiveMessage(message->sender().name(), ((IrcNoticeMessage*)message)->message());
        }
        break;
    case IrcMessage::Kick:
    case IrcMessage::Mode:
    case IrcMessage::Ping:
    case IrcMessage::Pong:
    case IrcMessage::Error:
        break;
    case IrcMessage::Numeric:
        processNumericMessage((IrcNumericMessage*)message);
        break;
    case IrcMessage::Unknown:
    default:
        qDebug() << "Unknown message received from" << _backend->host();
        break;
    }
}

ChannelModel *ServerModel::findOrCreateChannel(const QString &channelName)
{
    if (!_channels[channelName])
        addModelForChannel(channelName);
    return _channels[channelName];
}

void ServerModel::addModelForChannel(const QString &sender)
{
    if (!_channels[sender])
    {
        _channels[sender] = new ChannelModel(this, sender, _backend);
        if (_channels.count() == 1)
            _defaultChannel = _channels[sender];
        emit this->channelsChanged();
    }
}

void ServerModel::removeModelForChannel(const QString &sender)
{
    if (_channels[sender])
    {
        if (_channels[sender] == static_cast<IrcModel*>(parent())->currentChannel())
            static_cast<IrcModel*>(parent())->setCurrentChannelIndex(static_cast<IrcModel*>(parent())->currentChannelIndex() - 1);
        _channels.remove(sender);
        emit this->channelsChanged();
    }
}

void ServerModel::displayError(const QString &error)
{
    if (_defaultChannel)
        _defaultChannel->appendError(error);
}

void ServerModel::processNumericMessage(IrcNumericMessage *message)
{
    if (message->code() == Irc::RPL_ENDOFNAMES)
    {
        _channels[message->parameters()[1]]->updateUserList();
    }
    else if (message->code() == Irc::RPL_NAMREPLY || message->code() == Irc::RPL_NAMREPLY_)
    {
        _channels[message->parameters()[2]]->_soFarReceivedUserNames += message->parameters().at(3).split(' ', QString::SkipEmptyParts);
    }
    else if (message->code() == Irc::RPL_MOTD)
    {
        if (_defaultChannel)
            _defaultChannel->receiveMotd(message->parameters().at(1));
    }
    else if (message->code() == Irc::ERR_NICKNAMEINUSE)
    {
        QString newNick = _backend->nickName() + "_";
        displayError("The nickname '" + _backend->nickName() + "'' is already in use. Trying '" + newNick + "'.");
        changeNick(newNick);
    }
    else if (message->code() == Irc::ERR_NICKCOLLISION)
    {
        displayError("Nick name collision!");
    }
    else if (message->code() == Irc::ERR_BANLISTFULL)
    {
        displayError("Ban list is full.");
    }
    else if (message->code() == Irc::ERR_BANNEDFROMCHAN)
    {
        displayError("You are banned from this channel.");
    }
    else if (message->code() == Irc::ERR_CANNOTSENDTOCHAN)
    {
        displayError("You can't send messages to this channel.");
    }
    else if (message->code() == Irc::ERR_CHANNELISFULL)
    {
        displayError("Channel is full.");
    }
    else if (message->code() == Irc::ERR_CHANOPRIVSNEEDED)
    {
        displayError("Channel operator privileges are needed.");
    }
    else if (message->code() == Irc::ERR_INVITEONLYCHAN)
    {
        displayError("You can only join this channel if you're invited.");
    }
    else if (message->code() == Irc::ERR_NOSUCHCHANNEL)
    {
        displayError("There is no such channel.");
    }
    else if (message->code() == Irc::ERR_NOSUCHNICK)
    {
        displayError("There is no such nickname.");
    }
    else if (message->code() == Irc::ERR_UNKNOWNCOMMAND)
    {
        displayError("Unknown command.");
    }
    else if (message->code() >= 400)
    {
        displayError("An error occoured! Error code is: " + QString::number(message->code()));
    }
    else
    {
        //qDebug() << message->code() << "received from" << url() << "command is" << message->command() << "parameters are" << message->parameters();
    }
}

void ServerModel::joinChannel(const QString &channelName)
{
    qDebug() << "joining channel " << channelName;

    if (!_channels[channelName])
    {
        addModelForChannel(channelName);
        _backend->sendCommand(IrcCommand::createJoin(channelName));
        //_backend->sendCommand(IrcCommand::createNames(channelName));
    }
}

void ServerModel::partChannel(const QString &channelName)
{
    qDebug() << "parting channel " << channelName;

    if (_channels[channelName])
    {
        removeModelForChannel(channelName);
        _backend->sendCommand(IrcCommand::createPart(channelName, _settings->partMessage()));
    }
}

void ServerModel::queryUser(const QString &userName)
{
    qDebug() << "querying user " << userName;
    addModelForChannel(userName);
}

void ServerModel::closeUser(const QString &userName)
{
    qDebug() << "closing user " << userName;
    removeModelForChannel(userName);
}

void ServerModel::changeNick(const QString &nick)
{
    qDebug() << "changing nick to " << nick;
    _backend->setNickName(nick);
}

void ServerModel::msgUser(const QString &userName, const QString &msg)
{
    qDebug() << "msging user " << userName;
    _backend->sendCommand(IrcCommand::createMessage(userName, msg));
}

void ServerModel::kickUser(const QString &user, const QString &channel, const QString &message)
{
    qDebug() << "kick user" << user << " from " << channel;
    if (message.length())
        _backend->sendCommand(IrcCommand::createKick(channel, user, message));
    else
        _backend->sendCommand(IrcCommand::createKick(channel, user, _settings->kickMessage()));
}
