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

IPAddress IPAddress::host(int family)
{
    return host([family](const ifaddrs* ifAddr) {
        return ifAddr->ifa_addr->sa_family == family;
    });
}

IPAddress IPAddress::host(std::function<bool(const ifaddrs*)> filter)
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

}  // namespace net
}  // namespace jaegertracing
