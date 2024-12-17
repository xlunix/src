FROM ubuntu:24.10 AS  builder
# USER root
# RUN \
#     useradd -d /dev/null suricata && \
#     groupadd -r suricata && \
#     usermod suricata -a -G  suricata &&\ 
#     usermod suricata -a -G  root 
# RUN rm -rf /var/lib/apt/lists/*
# RUN apt-get clean
RUN  apt-get update && apt-get upgrade -y

RUN apt-get  -y install  libpcre2-dev build-essential autoconf \
automake libtool libpcap-dev libnet1-dev libyaml-0-2 libyaml-dev \
pkg-config zlib1g zlib1g-dev libcap-ng-dev libcap-ng0 make \
libmagic-dev libjansson-dev rustc cargo jq git-core python3 curl liblz4-dev gdb build-essential postgresql \
cbindgen libbpf-dev libecpg-dev libpq-dev postgresql-server-dev-all libnotify-bin

WORKDIR /src
ENV PATH="$PATH:/root/.cargo/bin"
RUN  cargo install --force cbindgen
RUN git clone https://github.com/OISF/suricata.git
WORKDIR /src/suricata
RUN git clone https://github.com/OISF/libhtp.git
COPY ./detect-engine-loader.c ./src
RUN ./autogen.sh 
RUN ./configure --enable-debug \
                --prefix=/usr/ \
                --sysconfdir=/etc  \
                --localstatedir=/var/ 
RUN make -j6

RUN make install install-conf DESTDIR=/install_suricata

RUN useradd --no-create-home --system --shell /sbin/nologin suricata

RUN mkdir /var/log/suricata  && \
    mkdir -p /var/lib/suricata && \
    mkdir -p /var/run/suricata &&\
    cp -r /install_suricata/etc /install_suricata/usr / &&\
    cp -r /install_suricata/var/log /var &&\
    cp -r /install_suricata/var/lib /var &&\
    chgrp -R suricata /etc/suricata &&\
    chmod -R g+r /etc/suricata &&\
    chown -R  suricata:suricata /var/log/suricata &&\
    chgrp -R suricata /var/log/suricata &&\
    chmod -R g+rw /var/log/suricata &&\
    chgrp -R suricata /var/lib/suricata &&\
    chmod -R g+srw /var/lib/suricata &&\
    chgrp -R suricata /var/run/suricata && \
    chmod -R g+srw /var/run/suricata

RUN ldconfig

WORKDIR /src/sql
COPY ./sql .
COPY ./sql/pg_hba.conf /etc/postgresql/16/main/
    
RUN make clean
RUN make all
RUN make install

WORKDIR /
COPY ./suricata.yaml /

# запуск сурикаты
COPY ./run_0.sh /

#для проверки второго пункта
COPY ./run_1.sh /

#для проверки третьего пункта
COPY ./run_2.sh /

RUN chmod +x /run_0.sh /run_1.sh /run_2.sh


