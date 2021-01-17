FROM jmckenna/rootana
RUN yum -y install lesstif-devel libXmu libXmu-devel 
COPY . /agdaq
WORKDIR /agdaq 
RUN /bin/bash -c "source /rootana/thisrootana.sh; source /agdaq/agconfig.sh; make"
CMD echo "$AGRELEASE"
