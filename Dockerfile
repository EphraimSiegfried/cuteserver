FROM gcc:9 as builder

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    g++ \
    libc-dev

WORKDIR /webs
COPY CMakeLists.txt .
COPY src/ src/
COPY deps/ deps/
RUN cmake . && make install

FROM debian:buster
EXPOSE 80
COPY --from=builder /usr/local/bin /usr/local/bin
ENTRYPOINT ["cuteserver", "-a", "0.0.0.0","-p","80", "-c", "/config.toml"]
