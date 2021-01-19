FROM jmckenna/rootana
COPY . /agdaq
WORKDIR /agdaq 
RUN /bin/bash -c "source /agdaq/agconfig.sh && mkdir build && cd build && cmake ../ && make -j4 && make install"
CMD echo "$AGRELEASE"
