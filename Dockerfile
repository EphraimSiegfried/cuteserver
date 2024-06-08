# Stage 1: Cutie-Builder
FROM gcc:9 as cutie-builder

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

#Stage 2: Setup frontend
FROM node:17-alpine as node-build
WORKDIR /app/frontend
COPY example_app/frontend/package.json .
COPY example_app/frontend/package-lock.json .
RUN npm install
COPY example_app/frontend/ .
RUN npm run build

#Stage 3: Setup backend
FROM gcc:9 as cmake-build
WORKDIR /app/backend
RUN apt-get update && apt-get install -y cmake libjson-c-dev
COPY example_app/backend/CMakeLists.txt .
COPY example_app/backend/main.c .
RUN cmake . && make

# stage 4: final
FROM debian:buster

# install tini for handling control-c
RUN apt-get update && apt-get install -y tini

EXPOSE 8888

COPY --from=cutie-builder /usr/local/bin /usr/local/bin

#TODO: location of entrypoint statement ? 

WORKDIR /content
COPY example_app/config.toml /
COPY --from=node-build /app/frontend/dist/ .
COPY --from=cmake-build /app/backend/main.cgi ./cgi-bin/

ENTRYPOINT ["/usr/bin/tini", "--", "cuteserver", "-a", "0.0.0.0", "-p", "8888", "-c", "/config.toml"]

