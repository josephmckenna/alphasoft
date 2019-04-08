FROM jmckenna/rootana
COPY . /agdaq
WORKDIR agdaq 
RUN ls
RUN source agconfig.sh && make
