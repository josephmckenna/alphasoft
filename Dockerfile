FROM jmckenna/rootana
COPY . /agdaq
WORKDIR agdaq 
RUN source agconfig.sh && cd ana && make
