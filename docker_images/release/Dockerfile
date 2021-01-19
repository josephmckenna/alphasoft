FROM jmckenna/rootana
COPY . /agdaq
WORKDIR /agdaq 
RUN pwd && ls && source /agdaq/agconfig.sh && mkdir build && cd build && cmake ../ && make -j4 && make install
CMD echo "$AGRELEASE"
