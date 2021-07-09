ARCH=`arch`
if [[ "$ARCH" == "ppc64le" ]] 
then
  apt-get install make gcc g++ wget libncurses5-dev -y
  wget https://cmake.org/files/v3.11/cmake-3.11.0.tar.gz
	tar -xzvf cmake-3.11.0.tar.gz 
	cd cmake-3.11.0 
	./bootstrap
  make
  make install 
	cd .. 
  apt remove make wget libncurses5-dev -y
  rm -rf cmake-3.11.0.tar.gz cmake-3.11.0 
else
  mkdir cmake-download 
  cd cmake-download 
  curl -O https://cmake.org/files/v3.10/cmake-3.10.0-rc5-Linux-x86_64.sh
  bash cmake-3.10.0-rc5-Linux-x86_64.sh --skip-license 
  cd ..
fi
