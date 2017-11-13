XDOCK_YAML=crossdock/docker-compose.yml

JAEGER_COMPOSE_URL=https://raw.githubusercontent.com/jaegertracing/jaeger/master/docker-compose/jaeger-docker-compose.yml
XDOCK_JAEGER_YAML=crossdock/jaeger-docker-compose.yml

CMAKE_OPTIONS=-DCMAKE_BUILD_TYPE=Release
CMAKE_OPTIONS=-DBUILD_TESTING=OFF
CMAKE_OPTIONS+=-DJAEGERTRACING_BUILD_EXAMPLES=OFF
CMAKE_OPTIONS+=-DJAEGERTRACING_BUILD_CROSSDOCK=ON

.PHONY: crossdock-bin
crossdock-bin:
	rm -rf build
	mkdir build
	cd build && cmake $(CMAKE_OPTIONS) .. && time make crossdock
	mv build/crossdock crossdock

.PHONY: crossdock
crossdock: crossdock-bin crossdock-download-jaeger
	docker-compose -f $(XDOCK_YAML) -f $(XDOCK_JAEGER_YAML) kill go
	docker-compose -f $(XDOCK_YAML) -f $(XDOCK_JAEGER_YAML) rm -f go
	docker-compose -f $(XDOCK_YAML) -f $(XDOCK_JAEGER_YAML) build go
	docker-compose -f $(XDOCK_YAML) -f $(XDOCK_JAEGER_YAML) run crossdock 2>&1 | tee run-crossdock.log
	grep 'Tests passed!' run-crossdock.log

.PHONY: crossdock-fresh
crossdock-fresh: crossdock-bin crossdock-download-jaeger
	docker-compose -f $(XDOCK_JAEGER_YAML) -f $(XDOCK_YAML) kill
	docker-compose -f $(XDOCK_JAEGER_YAML) -f $(XDOCK_YAML) rm --force
	docker-compose -f $(XDOCK_JAEGER_YAML) -f $(XDOCK_YAML) pull
	docker-compose -f $(XDOCK_JAEGER_YAML) -f $(XDOCK_YAML) build
	docker-compose -f $(XDOCK_JAEGER_YAML) -f $(XDOCK_YAML) run crossdock

.PHONY: crossdock-logs
crossdock-logs: crossdock-download-jaeger
	docker-compose -f $(XDOCK_JAEGER_YAML) -f $(XDOCK_YAML) logs

.PHONY: crossdock-download-jaeger
crossdock-download-jaeger:

