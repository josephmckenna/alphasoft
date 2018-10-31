FROM jmckenna/rootana
COPY . /agdaq
WORKDIR agdaq 
CMD cd agdaq && source agconfig.sh && cd ana && make
