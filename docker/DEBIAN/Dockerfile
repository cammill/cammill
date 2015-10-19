FROM debian:jessie

RUN apt-get update && apt-get install -y make git lsb-release clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev help2man lintian sudo

RUN cd /usr/src/ && \
    git clone https://github.com/cammill/cammill.git && \
    cd cammill && \
    make clean all && \
    make install

# Replace 1000 with your user / group id
RUN export uid=1000 gid=1000 && \
    mkdir -p /home/cammill && \
    echo "cammill:x:${uid}:${gid}:Developer,,,:/home/cammill:/bin/bash" >> /etc/passwd && \
    echo "cammill:x:${uid}:" >> /etc/group && \
    echo "cammill ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/cammill && \
    chmod 0440 /etc/sudoers.d/cammill && \
    chown ${uid}:${gid} -R /home/cammill && \
    usermod -a -G video cammill

USER cammill
ENV HOME /home/cammill

CMD /usr/bin/cammill -bm 1 /usr/share/doc/cammill/examples/test.dxf



