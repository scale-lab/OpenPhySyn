FROM centos:centos6 AS builder

# install gcc 6
RUN yum -y install centos-release-scl && \
    yum -y install devtoolset-6 devtoolset-6-libatomic-devel
ENV CC=/opt/rh/devtoolset-6/root/usr/bin/gcc \
    CPP=/opt/rh/devtoolset-6/root/usr/bin/cpp \
    CXX=/opt/rh/devtoolset-6/root/usr/bin/g++ \
    PATH=/opt/rh/devtoolset-6/root/usr/bin:$PATH \
    LD_LIBRARY_PATH=/opt/rh/devtoolset-6/root/usr/lib64:/opt/rh/devtoolset-6/root/usr/lib:/opt/rh/devtoolset-6/root/usr/lib64/dyninst:/opt/rh/devtoolset-6/root/usr/lib/dyninst:/opt/rh/devtoolset-6/root/usr/lib64:/opt/rh/devtoolset-6/root/usr/lib:$LD_LIBRARY_PATH

# install dependencies
RUN yum install -y wget git zlib-devel tcl-devel tk-devel swig bison flex

# Installing cmake for build dependency
RUN wget https://cmake.org/files/v3.13/cmake-3.13.4-Linux-x86_64.sh && \
    chmod +x cmake-3.13.4-Linux-x86_64.sh  && \
    ./cmake-3.13.4-Linux-x86_64.sh --skip-license --prefix=/usr/local

# download CUDD
RUN wget https://www.davidkebo.com/source/cudd_versions/cudd-3.0.0.tar.gz && \
    tar -xvf cudd-3.0.0.tar.gz && \
    cd cudd-3.0.0 && \
    ./configure && \
    make && \
    make install

# download Boost
RUN wget http://downloads.sourceforge.net/project/boost/boost/1.67.0/boost_1_67_0.tar.gz \
    && tar xfz boost_1_67_0.tar.gz \
    && rm boost_1_67_0.tar.gz \
    && cd boost_1_67_0 \
    && ./bootstrap.sh --prefix=/usr/local --with-libraries=program_options \
    && ./b2 install \
    && rm -rf boost_1_67_0

COPY . /PhyKnight
RUN mkdir /PhyKnight/build
WORKDIR /PhyKnight/build
# RUN cmake -DCMAKE_INSTALL_PREFIX=/build -DCUDD=/usr/local ..
RUN cmake -DCMAKE_INSTALL_PREFIX=/build ..
RUN make

# Run enviornment
FROM centos:centos6 AS runner
RUN yum update -y && yum install -y tcl-devel
COPY --from=builder /PhyKnight/build/Phy /build/Phy
RUN useradd -ms /bin/bash openroad
USER openroad
WORKDIR /home/openroad
