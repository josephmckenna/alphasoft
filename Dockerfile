FROM jmckenna/rootana
COPY . /agdaq
WORKDIR /agdaq 
CMD source /agdaq/agconfig.sh
RUN make
CMD echo "$AGRELEASE"
