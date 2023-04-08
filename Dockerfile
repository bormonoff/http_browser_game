FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \ 
    pip3 install conan==1.59.0

COPY ./conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing -s compiler.libcxx=libstdc++11 -s build_type=Release

COPY ./src /app/src
COPY ./data /app/data
COPY ./static /app/static
COPY ./tests /app/tests
COPY CMakeLists.txt /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .
    
FROM ubuntu:22.04 as run

RUN groupadd -r www && useradd -r -g www www
USER www

COPY --from=build /app/build/bin/game_server /app/
COPY ./data /app/data
COPY ./static /app/static

ENTRYPOINT ["/app/game_server",\
            "--config-file", "/app/data/config.json",\
            "--www-root", "/app/static",\
            "--state-file", "/var/save",\
            "--save-period", "500",\
            "--randomize-spawn-points",\
            "--tick-period", "10"] 
