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

#include <algorithm>

#include <gtest/gtest.h>

#include <spdlog/sinks/ostream_sink.h>

#include "jaegertracing/utils/ErrorUtil.h"

namespace jaegertracing {
namespace utils {
namespace {

bool endsWith(const std::string& str, const std::string& suffix)
{
    if (str.size() < suffix.size()) {
        return false;
    }

    return std::equal(
        std::begin(suffix), std::end(suffix), std::end(str) - suffix.size());
}

}  // anonymous namespace

TEST(ErrorUtil, test)
{
    std::ostringstream oss;
    const auto sink =
        std::make_shared<spdlog::sinks::ostream_sink_st>(oss, true);
    const auto logger = spdlog::create("test_logger", sink);
    std::runtime_error stdEx("runtime error");
    std::system_error sysEx(-1, std::generic_category());
    Transport::Exception transportEx("test", 5);
    for (auto i = 0; i < 4; ++i) {
        try {
            switch (i) {
            case 0:
                throw stdEx;
            case 1:
                throw sysEx;
            case 2:
                throw transportEx;
            default:
                ASSERT_EQ(3, i);
                throw 5;
            }
        } catch (...) {
            ErrorUtil::logError(*logger, "test", spdlog::level::warn);
            const auto logStr = oss.str();
            const auto logStrNoDate = logStr.substr(logStr.find(']') + 1);
            oss.clear();
            oss.str("");

            switch (i) {
            case 0: {
                ASSERT_EQ(" [test_logger] [warning] test: runtime error\n",
                          logStrNoDate);
            } break;
            case 1: {
                ASSERT_TRUE(endsWith(logStrNoDate, ", code=-1\n"));
            } break;
            case 2: {
                ASSERT_EQ(" [test_logger] [warning] test: test, numFailed=5\n",
                          logStrNoDate);
            } break;
            default: {
                ASSERT_EQ(3, i);
                ASSERT_EQ(" [test_logger] [warning] test\n", logStrNoDate);
            } break;
            }
        }
    }
}

}  // namespace utils
}  // namespace jaegertracing
