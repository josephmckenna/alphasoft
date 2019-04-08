FROM jmckenna/rootana
COPY . /agdaq
WORKDIR /agdaq 
CMD soure /agdaq/agconfig.sh
RUN make
CMD echo "$AGRELEASE"
