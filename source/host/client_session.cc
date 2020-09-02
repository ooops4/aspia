//
// Aspia Project
// Copyright (C) 2020 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#include "host/client_session.h"

#include "base/logging.h"
#include "base/net/network_channel_proxy.h"
#include "host/client_session_desktop.h"
#include "host/client_session_file_transfer.h"

namespace host {

ClientSession::ClientSession(
    proto::SessionType session_type, std::unique_ptr<base::NetworkChannel> channel)
    : session_type_(session_type),
      channel_(std::move(channel))
{
    DCHECK(channel_);

    // All sessions are executed in one thread. We can safely use a global counter to get session IDs.
    // Session IDs must start with 1.
    static uint32_t id_counter = 0;
    id_ = ++id_counter;
}

// static
std::unique_ptr<ClientSession> ClientSession::create(
    proto::SessionType session_type, std::unique_ptr<base::NetworkChannel> channel)
{
    if (!channel)
        return nullptr;

    switch (session_type)
    {
        case proto::SESSION_TYPE_DESKTOP_MANAGE:
        case proto::SESSION_TYPE_DESKTOP_VIEW:
            return std::unique_ptr<ClientSessionDesktop>(
                new ClientSessionDesktop(session_type, std::move(channel)));

        case proto::SESSION_TYPE_FILE_TRANSFER:
            return std::unique_ptr<ClientSessionFileTransfer>(
                new ClientSessionFileTransfer(std::move(channel)));

        default:
            return nullptr;
    }
}

void ClientSession::start(Delegate* delegate)
{
    state_ = State::STARTED;

    delegate_ = delegate;
    DCHECK(delegate_);

    channel_->setListener(this);
    channel_->resume();

    onStarted();
}

void ClientSession::stop()
{
    state_ = State::FINISHED;
    delegate_->onClientSessionFinished();
}

void ClientSession::setVersion(const base::Version& version)
{
    version_ = version;
}

void ClientSession::setUserName(std::string_view username)
{
    username_ = username;
}

std::u16string ClientSession::peerAddress() const
{
    return channel_->peerAddress();
}

void ClientSession::setSessionId(base::SessionId session_id)
{
    session_id_ = session_id;
}

std::shared_ptr<base::NetworkChannelProxy> ClientSession::channelProxy()
{
    return channel_->channelProxy();
}

void ClientSession::sendMessage(base::ByteArray&& buffer)
{
    channel_->send(std::move(buffer));
}

void ClientSession::onConnected()
{
    NOTREACHED();
}

void ClientSession::onDisconnected(base::NetworkChannel::ErrorCode error_code)
{
    LOG(LS_WARNING) << "Client disconnected with error: "
                    << base::NetworkChannel::errorToString(error_code);

    state_ = State::FINISHED;
    delegate_->onClientSessionFinished();
}

} // namespace host
