FROM centos:centos7 AS base-dependencies
LABEL maintainer "Ahmed Agiza <ahmed_agiza@brown.edu>"

# Install dev and runtime dependencies
RUN yum group install -y "Development Tools" \
    && yum install -y https://centos7.iuscommunity.org/ius-release.rpm \
    && yum install -y wget git centos-release-scl devtoolset-8 \
    devtoolset-8-libatomic-devel tcl-devel tcl tk libstdc++ tk-devel pcre-devel \
    python36u python36u-libs python36u-devel python36u-pip && \
    yum clean -y all && \
    rm -rf /var/lib/apt/lists/*


# Install gcc 7.3
RUN curl -O https://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.gz \
    && tar xzf gcc-7.3.0.tar.gz                      \
    && cd gcc-7.3.0                                  \
    && ./contrib/download_prerequisites              \
    && cd ..                                         \
    && mkdir gcc-build                               \
    && cd gcc-build                                  \
    && ../gcc-7.3.0/configure                        \
    --prefix=/usr                                    \
    --enable-shared                                  \
    --enable-threads=posix                           \
    --enable-__cxa_atexit                            \
    --enable-clocale=gnu                             \
    --disable-multilib                               \
    --enable-languages=all                           \
    && make -j$(nproc)                               \
    && make install                                  \
    && cd ..                                         \
    && rm -rf gcc-7.3.0                              \
    && rm -rf gcc-7.3.0.tar.gz                       \
    && rm -rf gcc-build


# Install CMake
RUN wget https://cmake.org/files/v3.14/cmake-3.14.0-Linux-x86_64.sh && \
    chmod +x cmake-3.14.0-Linux-x86_64.sh  && \
    ./cmake-3.14.0-Linux-x86_64.sh --skip-license --prefix=/usr/local

# Install epel repo
RUN wget https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm && \
    yum install -y epel-release-latest-7.noarch.rpm



# Install SWIG
RUN yum remove -y swig \
    && wget https://github.com/swig/swig/archive/rel-4.0.1.tar.gz \
    && tar xfz rel-4.0.1.tar.gz \
    && rm -rf rel-4.0.1.tar.gz \
    && cd swig-rel-4.0.1 \
    && ./autogen.sh && ./configure --prefix=/usr && make -j $(nproc) && make install \
    && cd .. \
    && rm -rf swig-rel-4.0.1


FROM base-dependencies AS builder

COPY . /OpenPhySyn
WORKDIR /OpenPhySyn

# Build
RUN mkdir build
RUN cd build && cmake .. -DCMAKE_BUILD_TYPE=release && make -j 4