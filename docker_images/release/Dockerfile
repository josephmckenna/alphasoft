FROM jmckenna/alphadocker
COPY . /alphasoft
WORKDIR /alphasoft
RUN pwd \
&& ls \
&& source agconfig.sh \
&& mkdir build \
&& cd build \
&& cmake ../ \
&& make -j4 \
&& make install
CMD echo "$AGRELEASE"
