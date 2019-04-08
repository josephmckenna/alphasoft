FROM jmckenna/rootana
COPY . /agdaq
WORKDIR /agdaq 
RUN /bin/bash -c "source /agdaq/agconfig.sh"
RUN /bin/bash -c "source /rootana/thisrootana.sh"
RUN make
CMD echo "$AGRELEASE"
