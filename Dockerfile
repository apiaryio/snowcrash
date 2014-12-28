FROM ubuntu:12.04
MAINTAINER Lukas Linhart "lukas@apiary.io"

RUN ["apt-get", "-yq", "install", "sudo"]

ADD ./provisioning.sh /

RUN ["sh", "/provisioning.sh"]

ADD ./ /snowcrash

WORKDIR /snowcrash

RUN ["./configure", "--include-integration-tests"]

CMD ["make", "test"]
