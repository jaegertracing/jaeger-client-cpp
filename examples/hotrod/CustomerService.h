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

#ifndef JAEGERTRACING_EXAMPLES_HOTROD_CUSTOMERSERVICE_H
#define JAEGERTRACING_EXAMPLES_HOTROD_CUSTOMERSERVICE_H

#include <string>

namespace jaegertracing {
namespace examples {
namespace hotrod {

class Customer {
  public:
    Customer() = default;

    Customer(const std::string& id,
             const std::string& name,
             const std::string& location)
        : _id(id)
        , _name(name)
        , _location(location)
    {
    }

    const std::string& id() const { return _id; }

    void setID(const std::string& id) { _id = id; }

    const std::string& name() const { return _name; }

    void setName(const std::string& name) { _name = name; }

    const std::string& location() const { return _location; }

    void setLocation(const std::string& location) { _location = location; }

  private:
    std::string _id;
    std::string _name;
    std::string _location;
};

class CustomerService {
  public:
    virtual ~CustomerService() = default;

    virtual Customer get(const std::string& customerID) = 0;
};

}  // namespace hotrod
}  // namespace examples
}  // namespace jaegertracing

#endif  // JAEGERTRACING_EXAMPLES_HOTROD_CUSTOMERSERVICE_H
