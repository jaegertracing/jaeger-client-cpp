/*
 * Copyright (c) 2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAEGERTRACING_NET_SOCKET_H
#define JAEGERTRACING_NET_SOCKET_H

#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/net/URI.h"
#include <errno.h>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <ostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

namespace jaegertracing {
namespace net {

class Socket {
  public:
    Socket()
        : _handle(-1)
        , _family(-1)
        , _type(-1)
    {
    }

    Socket(const Socket&) = delete;

    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& socket)
        : _handle(socket._handle)
        , _family(socket._family)
        , _type(socket._type)
    {
        socket._handle = -1;
    }

    Socket& operator=(Socket&& rhs)
    {
        _handle = rhs._handle;
        _family = rhs._family;
        _type = rhs._type;
        return *this;
    }

    ~Socket() { close(); }

    void open(int family, int type)
    {
        const auto handle = ::socket(family, type, 0);
        if (handle < 0) {
            std::ostringstream oss;
            oss << "Failed to open socket"
                   ", family="
                << family << ", type=" << type;
            throw std::system_error(errno, std::system_category(), oss.str());
        }
        _handle = handle;
        _family = family;
        _type = type;
    }

    void bind(const IPAddress& addr)
    {
        const auto returnCode =
            ::bind(_handle,
                   reinterpret_cast<const ::sockaddr*>(&addr.addr()),
                   addr.addrLen());
        if (returnCode != 0) {
            std::ostringstream oss;
            oss << "Failed to bind socket to address"
                   ", addr=";
            addr.print(oss);
            throw std::system_error(errno, std::system_category(), oss.str());
        }
    }

    void bind(const std::string& ip, int port)
    {
        const auto addr = IPAddress::v4(ip, port);
        bind(addr);
    }

    void connect(const IPAddress& serverAddr)
    {
        const auto returnCode =
            ::connect(_handle,
                      reinterpret_cast<const ::sockaddr*>(&serverAddr.addr()),
                      serverAddr.addrLen());
        if (returnCode != 0) {
            std::ostringstream oss;
            oss << "Cannot connect socket to remote address ";
            serverAddr.print(oss);
            throw std::runtime_error(oss.str());
        }
    }

    IPAddress connect(const std::string& serverURIStr)
    {
        return connect(URI::parse(serverURIStr));
    }

    IPAddress connect(const URI& serverURI)
    {
        auto result =
            resolveAddress(serverURI._host, serverURI._port, AF_INET, _type);
        for (const auto* itr = result.get(); itr; itr = itr->ai_next) {
            const auto returnCode =
                ::connect(_handle, itr->ai_addr, itr->ai_addrlen);
            if (returnCode == 0) {
                return IPAddress(*itr->ai_addr, itr->ai_addrlen);
            }
        }
        std::ostringstream oss;
        oss << "Cannot connect socket to remote address ";
        serverURI.print(oss);
        throw std::runtime_error(oss.str());
    }

    static constexpr auto kDefaultBacklog = 128;

    void listen(int backlog = kDefaultBacklog)
    {
        const auto returnCode = ::listen(_handle, backlog);
        if (returnCode != 0) {
            throw std::system_error(
                errno, std::system_category(), "Failed to listen on socket");
        }
    }

    Socket accept()
    {
        ::sockaddr_storage addrStorage;
        ::socklen_t addrLen = sizeof(addrStorage);
        const auto clientHandle = ::accept(
            _handle, reinterpret_cast<::sockaddr*>(&addrStorage), &addrLen);
        if (clientHandle < 0) {
            throw std::system_error(
                errno, std::system_category(), "Failed to accept on socket");
        }

        Socket clientSocket;
        clientSocket._handle = clientHandle;
        clientSocket._family =
            (addrLen == sizeof(::sockaddr_in)) ? AF_INET : AF_INET6;
        clientSocket._type = SOCK_STREAM;
        return clientSocket;
    }

    void close() noexcept
    {
        if (_handle >= 0) {
            ::close(_handle);
            _handle = -1;
        }
    }

    int handle() { return _handle; }

  private:
    int _handle;
    int _family;
    int _type;
};

static constexpr auto kUDPPacketMaxLength = 65000;

}  // namespace net
}  // namespace jaegertracing

#endif  // JAEGERTRACING_NET_SOCKET_H
