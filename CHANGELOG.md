Changes by Version
==================

0.5.0 (unreleased)
------------------
- Nothing yet


0.4.2 (2018-08-14)
------------------
- Fix `tracer.Inject(..., HTTPHeadersWriter&)` (#107)
- Upgrade dynamic loading API (#120)


0.4.1 (2018-05-16)
------------------
- Add example application (#101)
- Improve CMake installation to allow use of lib64 directory (#102)
- Fix CMake config for OpenTracing dependency (#103)
- Fix tracer inject for HTTP headers (#107)


0.4.0 (2018-05-07)
------------------
- Build shared plugin for Linux amd64 using Docker (#82)
- Fix UDP test compilation error (#88)
- Fix usage of propagation headers config (#91)
- Fix dynamic load build error (#92)
- Use Thrift 0.11.0 (#94)


0.3.0 (2018-04-17)
------------------
- Use LogRecord in Span::FinishWithOptions (#58)
- Flush pending spans in RemoteReporter destructor (#59)
- Add support for dynamic loading (#64)
- Fix unhandled exception when Jaeger agent unavailable (#80)
- Fix potential race condition in concurrent baggage access (#83)


0.2.0 (2018-01-25)
------------------
- Fix bug in localIP and revert change in TR1 tuple definition (#31)
- Add language prefix to Jaeger client version tag (#35)
- Fix yaml-cpp issues (#39)


0.1.0 (2017-11-29)
------------------
- Don't use forwarding reference (#11)
- Fix overflow bug (#13)
- Set default-constructed timestamp to current time in StartSpan (#18)


0.0.8 (2017-11-20)
------------------
- Fix host IP formatting and improve local IP API (#4)
- Use JSON instead of Thrift's TJSONProtocol (#12)
