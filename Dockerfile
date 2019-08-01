FROM ex-build-env

SHELL ["/bin/bash", "-lc"]
WORKDIR /root/workspace

COPY . ./

ARG BUILD_TYPE=Debug

RUN mkdir build && \
  cd build && \
  cmake .. -GNinja -DCMAKE_BUILD_TYPE="$BUILD_TYPE" && \
  ninja
