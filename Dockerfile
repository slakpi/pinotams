FROM pinotams-build-base:latest AS build

ARG CONF_FILE
WORKDIR /src
COPY . .
RUN mkdir build && \
  cd build && \
  cmake .. -DCMAKE_INSTALL_PREFIX=/opt/pinotams && \
  make
RUN echo "$CONF_FILE" > pinotams.conf

FROM ubuntu:focal

RUN apt update && \
  apt upgrade -y && \
  apt install -y libcurl4 libjansson4 libsqlite3-0 libuuid1 libmhash2 \
  libpcre2-8-0 libpcre2-16-0 libpcre2-32-0 libpcre2-posix2

WORKDIR /opt/pinotams
COPY --from=build /src/build/pinotams bin/pinotams
COPY --from=build /src/pinotams.conf etc/pinotams.conf
RUN mkdir -p var/pinotams

CMD ["bin/pinotams", "-s"]
