FROM jmckenna/rootana
COPY . /agdaq
WORKDIR agdaq 
CMD source agconfig.sh && cd ana && make
