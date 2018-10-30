FROM jmckenna/rootana
RUN . agconfig.sh && cd ana && make
