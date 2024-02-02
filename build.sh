echo QingLiaoChatServerBuilder

VCPKG_PATH=/bin/vcpkg

vcpkg install kcp
vcpkg install openssl
vcpkg install asio
vcpkg install cpp-httplib
vcpkg install websocketpp

git clone https://github.com/mariadb-corporation/mariadb-connector-cpp.git
cd mariadb-connector-cpp
mkdir build
cd build
cmake ..
cmake --build . --config Release -j8

cd ../../
mkdir build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_PATH/scripts/buildsystems/vcpkg.cmake
cd build
cmake --build . --config Release -j8
