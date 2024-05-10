FROM alpine:3.19 AS build

RUN apk update && apk add \
    cmake \
    make \
    g++ \
    libc-dev


COPY CMakeLists.txt /webs/ 
COPY src/ /webs/src
COPY deps/ /webs/deps


COPY data/ /webs/data
COPY config.toml /webs/config.toml

EXPOSE 80
# VOLUME ["/cuteserver/data", "/cuteserver/config", "/etc/apache2/external"]

RUN cd /webs && cmake . && make install

ENTRYPOINT [ "cuteserver 80 0 /webs/config.toml" ]

