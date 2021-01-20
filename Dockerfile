FROM jmckenna/alphadocker
WORKDIR /alphasoft
COPY . /alphasoft
RUN pwd \
&& ls \
&& source agconfig.sh \
&& mkdir build \
&& cd build \
&& cmake .. \
&& make -j4 \
&& make install
CMD echo "$AGRELEASE"
#RUN ["root","-l"]