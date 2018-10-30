FROM jmckenna/rootana
COPY . /agdaq
RUN cd /agdaq && source agconfig.sh && cd ana && make
