# stage 1: builder
FROM gcc:9 as builder

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    g++ \
    libc-dev \
    libgomp1

WORKDIR /webs
COPY CMakeLists.txt .
COPY src/ src/
COPY deps/ deps/
RUN cmake . && make install

# stage 2: final
FROM debian:buster

# install tini for handling control-c
RUN apt-get update && apt-get install -y tini

EXPOSE 80

COPY --from=builder /usr/local/bin /usr/local/bin

RUN apt-get update && apt-get install -y \
    libgomp1

ENTRYPOINT ["/usr/bin/tini", "--", "cuteserver", "-a", "0.0.0.0", "-p", "80", "-c", "/config.toml"]
