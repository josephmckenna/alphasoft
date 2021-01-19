FROM jmckenna/alphadocker
COPY . /alphasoft
WORKDIR /alphasoft
RUN pwd \
&& ls \
&& source alphasoft/agconfig.sh \
&& mkdir build \
&& cd build \
&& cmake ../alphasoft \
&& make -j4 \
&& make install
CMD echo "$AGRELEASE"
