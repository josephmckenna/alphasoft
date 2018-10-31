FROM jmckenna/rootana
COPY . /agdaq
WORKDIR agdaq 
RUN cd agdaq && source agconfig.sh && cd ana && make
