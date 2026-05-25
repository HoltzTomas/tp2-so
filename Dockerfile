FROM agodio/itba-so-multi-platform:3.0
LABEL maintainer="ITBA SO"
RUN apt-get update && apt-get install -y nasm qemu-system-x86
COPY . /root/tp2-so
WORKDIR /root/tp2-so
