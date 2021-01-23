# syntax=docker/dockerfile:1.2
FROM ubuntu:focal
LABEL maintainer=rectalogic

ENV container docker
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=en_US.UTF-8
RUN echo "locales locales/default_environment_locale select en_US.UTF-8" | debconf-set-selections \
    && echo "locales locales/locales_to_be_generated multiselect en_US.UTF-8 UTF-8" | debconf-set-selections \
    && echo "tzdata tzdata/Areas select Etc" | debconf-set-selections \
    && echo "tzdata tzdata/Areas/Etc select UTC" | debconf-set-selections \
    && apt-get -y update \
    && apt-get -y install locales tzdata \
    && dpkg-reconfigure locales tzdata \
    && apt-get -y install build-essential ninja-build perl python \
       apt-transport-https ca-certificates gnupg software-properties-common \
       curl xz-utils ffmpeg libsdl2-dev libgtk2.0-dev xvfb clang libclang-dev libpulse-dev libssl-dev libxml2-dev pkg-config \
       libavutil-dev libavformat-dev libavdevice-dev libavresample-dev libpostproc-dev libavfilter-dev libswscale-dev libswresample-dev \
       libgl1-mesa-dev libfontconfig1-dev libfreetype6-dev libx11-dev libx11-xcb-dev libxext-dev libxfixes-dev libxi-dev libxrender-dev \
       libxcb1-dev libxcb-glx0-dev libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev \
       libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-util-dev \
       libxcb-randr0-dev libxcb-render-util0-dev libxcb-xinerama0-dev libxkbcommon-dev libxkbcommon-x11-dev \
       fontconfig fonts-liberation imagemagick \
    && curl https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor - > /etc/apt/trusted.gpg.d/kitware.gpg \
    && apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main' \
    && apt-get -y update \
    && apt-get -y install cmake \
    && rm -rf /var/lib/apt/lists/*

RUN export QT_VER=6.0.0 \
    && curl -L http://download.qt.io/official_releases/qt/6.0/${QT_VER}/single/qt-everywhere-src-${QT_VER}.tar.xz | tar xJf - \
    && mkdir qt6-build \
    && cd qt6-build && ../qt-everywhere-src-${QT_VER}/configure && cmake --build . --parallel && cmake --install . \
    && cd .. && rm -rf qt6-build qt-everywhere-src-${QT_VER}
RUN export MLT_VER=6.24.0 \
    && curl -L https://github.com/mltframework/mlt/releases/download/v${MLT_VER}/mlt-${MLT_VER}.tar.gz | tar xzf - \
    && cd mlt-${MLT_VER} && ./configure && make && make install \
    && cd .. && rm -rf mlt-${MLT_VER}

RUN ldconfig

ENTRYPOINT ["/usr/bin/xvfb-run"]
CMD ["/usr/local/bin/melt"]