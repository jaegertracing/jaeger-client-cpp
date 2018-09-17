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

#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/platform/Hostname.h"

#include <ifaddrs.h>
#include <sys/types.h>

namespace jaegertracing {
namespace net {
namespace {

struct IfAddrDeleter : public std::function<void(ifaddrs*)> {
    void operator()(ifaddrs* ifAddr) const
    {
        if (ifAddr) {
            ::freeifaddrs(ifAddr);
        }
    }
};

}  // anonymous namespace

IPAddress IPAddress::localIP(int family)
{
    try {
        return versionFromString(platform::hostname(), 0, family);
    } catch (...) {
        // Fall back to returning the first matching interface
    }

    return localIP([family](const ifaddrs* ifAddr) {
        return ifAddr->ifa_addr != nullptr &&
               ifAddr->ifa_addr->sa_family == family;
    });
}

IPAddress IPAddress::localIP(std::function<bool(const ifaddrs*)> filter)
{
    auto* ifAddrRawPtr = static_cast<ifaddrs*>(nullptr);
    getifaddrs(&ifAddrRawPtr);
    std::unique_ptr<ifaddrs, IfAddrDeleter> ifAddr(ifAddrRawPtr);
    for (auto* itr = ifAddr.get(); itr; itr = itr->ifa_next) {
        if (filter(itr)) {
            const auto family = ifAddr->ifa_addr->sa_family;
            const auto addrLen = (family == AF_INET) ? sizeof(::sockaddr_in)
                                                     : sizeof(::sockaddr_in6);
            return IPAddress(*itr->ifa_addr, addrLen);
        }
    }
    return IPAddress();
}

IPAddress
IPAddress::versionFromString(const std::string& ip, int port, int family)
{
    ::sockaddr_storage addrStorage;
    std::memset(&addrStorage, 0, sizeof(addrStorage));

    auto* addrBuffer = static_cast<void*>(nullptr);
    if (family == AF_INET) {
        ::sockaddr_in& addr = *reinterpret_cast<::sockaddr_in*>(&addrStorage);
        addr.sin_family = family;
        addr.sin_port = htons(port);
        addrBuffer = &addr.sin_addr;
    }
    else {
        assert(family == AF_INET6);
        ::sockaddr_in6& addr = *reinterpret_cast<::sockaddr_in6*>(&addrStorage);
        addr.sin6_family = family;
        addr.sin6_port = htons(port);
        addrBuffer = &addr.sin6_addr;
    }

    const auto returnCode = inet_pton(family, ip.c_str(), addrBuffer);
    if (returnCode == 0) {
        auto result = resolveAddress(ip, port, family);
        assert(result);
        std::memcpy(&addrStorage, result->ai_addr, result->ai_addrlen);
    }
    return IPAddress(addrStorage,
                     family == AF_INET ? sizeof(::sockaddr_in)
                                       : sizeof(::sockaddr_in6));
}

std::unique_ptr<::addrinfo, AddrInfoDeleter>
resolveAddress(const std::string& host, int port, int family, int type)
{
    ::addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = type;

    std::string service;
    if (port != 0) {
        service = std::to_string(port);
    }

    auto* servInfoPtr = static_cast<::addrinfo*>(nullptr);
    const auto returnCode =
        getaddrinfo(host.c_str(), service.c_str(), &hints, &servInfoPtr);
    std::unique_ptr<::addrinfo, AddrInfoDeleter> servInfo(servInfoPtr);
    if (returnCode != 0) {
        std::ostringstream oss;
        oss << "Error resolving address: " << gai_strerror(returnCode);
        throw std::runtime_error(oss.str());
    }

    return servInfo;
}

}  // namespace net
}  // namespace jaegertracing
