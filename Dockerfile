FROM apiaryio/base-cpp:1.0.0
MAINTAINER Lukas Linhart "lukas@apiary.io"

# RUN apt-get install -y \
#    ruby-json \
#    ruby-diff-lcs \
#     bundler
#    cucumber \
#    ruby-ffi

#      ruby-rspec \
#      ruby-rspec-expectations && \

# RUN apt-get install -y bundler

RUN locale-gen en_US.UTF-8
RUN echo "LC_ALL=en_US.UTF-8" >> /etc/default/locale
RUN dpkg-reconfigure locales

RUN apt-get install -y ntpdate

# RUN gem install bundler

ADD ./ /snowcrash

WORKDIR /snowcrash

# It's tempting to put ./configure into RUN, but then you have timestamp issues

CMD ntpdate pool.ntp.org && ./configure && make test
