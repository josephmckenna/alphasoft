FROM jmckenna/rootana
COPY . /agdaq
WORKDIR /agdaq 
RUN /bin/bash -c "source /rootana/thisrootana.sh; source /agdaq/agconfig.sh; make"
CMD echo "$AGRELEASE"
