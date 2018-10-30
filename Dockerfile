FROM jmckenna/rootana
COPY . /agdaq
WORDIR agdaq 
CMD source agconfig.sh && cd ana && make
