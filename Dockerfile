FROM jmckenna/rootana
CMD source agconfig.sh
WORKDIR ana
RUN make 
