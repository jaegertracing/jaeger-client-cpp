FROM gcc:7.2

WORKDIR /
RUN curl -O https://cmake.org/files/v3.10/cmake-3.10.0-rc5-Linux-x86_64.sh && \
    bash cmake-3.10.0-rc5-Linux-x86_64.sh --skip-license

COPY . /app/jaegertracing
RUN rm -rf /app/jaegertracing/build && \
    mkdir /app/jaegertracing/build && \
    cd /app/jaegertracing/build && \
    cmake -DCMAKE_BUILD_TYPE=Debug -DJAEGERTRACING_BUILD_CROSSDOCK=ON .. && \
    make crossdock -j3

ENV AGENT_HOST_PORT=jaeger-agent:5775
ENV SAMPLING_SERVER_URL=http://test_driver:5778/sampling

EXPOSE 8080-8082

CMD ["/app/jaegertracing/build/crossdock"]
