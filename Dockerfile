FROM ubuntu:18.04

RUN apt-get update && apt-get -y install cmake clang git build-essential zlib1g-dev libssl-dev libcurl4-openssl-dev \
                        libgflags-dev libprotobuf-dev libprotoc-dev protobuf-compiler  libgoogle-glog-dev \
                        libleveldb-dev libbz2-dev libzstd-dev libsnappy-dev libjemalloc-dev

RUN mkdir /code
WORKDIR /code

RUN git clone https://github.com/aws/aws-sdk-cpp.git
RUN git clone https://github.com/harishb2k/rocksdb-cloud.git
RUN git clone https://github.com/apache/incubator-brpc.git

WORKDIR /code/aws-sdk-cpp/
RUN git checkout tags/1.7.150

WORKDIR /code/incubator-brpc/
RUN git checkout 5f7e048

RUN apt-get install -y
WORKDIR /code/aws-sdk-cpp
RUN cmake . -DENABLE_TESTING=OFF -DAUTORUN_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release '-DBUILD_ONLY=s3;kinesis;transfer' -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations"
RUN make -j8 install

WORKDIR /code/incubator-brpc/
RUN cmake . -DWITH_GLOG=ON
RUN make -j8 install

WORKDIR /code/rocksdb-cloud/
RUN rm CMakeLists.txt
RUN cp CMakeLists_Linux.txt CMakeLists.txt
RUN cmake .
RUN make -j8 install

RUN rm -rf /code

