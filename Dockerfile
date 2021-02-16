FROM jmckenna/alphadocker
WORKDIR /alphasoft
COPY . /alphasoft
RUN pwd \
&& ls * \
&& source /alphasoft/agconfig.sh \
&& mkdir /build \
&& cd /build \
&& cmake3 /alphasoft \
&& cat GitInfo.h \
&& make -j4 \
&& make install
CMD echo "$AGRELEASE"
#RUN ["root","-l"]