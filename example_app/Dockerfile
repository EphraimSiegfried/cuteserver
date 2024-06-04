# Setup frontend
FROM node:17-alpine as node-build
WORKDIR /app/frontend
COPY frontend/package.json .
COPY frontend/package-lock.json .
RUN npm install
COPY /frontend/ .
RUN npm run build

# Setup backend
FROM gcc:9 as cmake-build
WORKDIR /app/backend
RUN apt-get update && apt-get install -y cmake libjson-c-dev
COPY backend/CMakeLists.txt .
COPY backend/main.c .
RUN cmake . && make

FROM cutie

RUN set -ex;         \
    apt-get update;  \
    apt-get install -y libjson-c-dev

WORKDIR /content
COPY config.toml /
COPY --from=node-build /app/frontend/dist/ .
COPY --from=cmake-build /app/backend/main.cgi ./cgi-bin/

