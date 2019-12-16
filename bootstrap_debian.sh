#!/usr/bin/sh
echo <<EOF | sudo tee /etc/apt/sources.list
deb https://deb.debian.org/debian testing main non-free contrib
deb https://deb.debian.org/debian testing-updates main non-free contrib
deb http://security.debian.org/debian-security testing-security main non-free contrib
EOF

sudo apt update
sudo apt upgrade -y

# these packages are extraneous but nice to have
sudo apt install -y \
	 mosh \
	 rsync \
	 mosh \
	 tmux \
	 locales \
	 && true

touch .bash_profile

sudo apt install -y \
	 bash \
	 clang \
	 libtbb-dev \
	 libboost-python-dev \
	 libboost-thread-dev \
	 make \
	 git \
	 time \
	 python3.8-dev \
	 && true
