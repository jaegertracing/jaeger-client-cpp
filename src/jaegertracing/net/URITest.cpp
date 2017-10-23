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

#include <gtest/gtest.h>

#include "jaegertracing/net/URI.h"

namespace jaegertracing {
namespace net {

TEST(URI, testMatch)
{
    ASSERT_EQ(0, URI::parse("http://localhost")._port);
    ASSERT_EQ(80, URI::parse("http://localhost:80")._port);
}

TEST(URI, testAuthority)
{
    ASSERT_EQ("localhost", URI::parse("http://localhost").authority());
    ASSERT_EQ("localhost:80", URI::parse("http://localhost:80").authority());
}

TEST(URI, testPrint)
{
    std::ostringstream oss;
    oss << URI::parse("localhost");
    ASSERT_EQ(
        "{ scheme=\"\""
        ", host=\"\""
        ", port=0"
        ", path=\"localhost\""
        ", query=\"\" }",
        oss.str());
}

TEST(URI, queryEscape)
{
    ASSERT_EQ("hello%20world", URI::queryEscape("hello world"));
    ASSERT_EQ("hello-world", URI::queryEscape("hello-world"));
    ASSERT_EQ("hello.world", URI::queryEscape("hello.world"));
    ASSERT_EQ("hello_world", URI::queryEscape("hello_world"));
    ASSERT_EQ("hello~world", URI::queryEscape("hello~world"));
}

TEST(URI, queryUnescape)
{
    ASSERT_EQ("hello world", URI::queryUnescape("hello%20world"));
    ASSERT_EQ("hello%2world", URI::queryUnescape("hello%2world"));
    ASSERT_EQ("hello%world", URI::queryUnescape("hello%world"));
    ASSERT_EQ("hello world", URI::queryUnescape("hello w%6Frld"));
    ASSERT_EQ("hello world", URI::queryUnescape("hello w%6frld"));
}

TEST(URI, testResolveAddress)
{
    ASSERT_NO_THROW(resolveAddress("http://localhost", SOCK_STREAM));
    ASSERT_NO_THROW(resolveAddress("http://localhost:80", SOCK_STREAM));
    ASSERT_NO_THROW(resolveAddress("http://123456", SOCK_STREAM));
    ASSERT_THROW(resolveAddress("http://localhost", -1), std::runtime_error);
}

}  // namespace net
}  // namespace jaegertracing
