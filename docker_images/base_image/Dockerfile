FROM cern/cc7-base

LABEL maintainer.name="Joseph McKenna"
#LABEL maintainer.email=""

COPY packages packages

RUN echo -e "[eos-citrine]\nname=EOS 4.0 Version\nbaseurl=https://storage-ci.web.cern.ch/storage-ci/eos/citrine/tag/el-7/x86_64/\ngpgcheck=0\n" > /etc/yum.repos.d/eos.repo \
 && echo -e "[eos-citrine-dep]\nname=EOS 4.0 Dependencies\nbaseurl=https://storage-ci.web.cern.ch/storage-ci/eos/citrine-depend/el-7/x86_64/\ngpgcheck=0\n" >> /etc/yum.repos.d/eos.repo \
 && yum update -q -y \
 && yum install -y epel-release \
 && yum install -y $(cat packages) \
 && localedef -i en_US -f UTF-8 en_US.UTF-8 \
 && rm -f /packages
