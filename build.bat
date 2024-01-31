echo QingLiaoChatServerBuilder

git clone https://github.com/microsoft/vcpkg
cd vcpkg
vcpkg install kcp
vcpkg install openssl
vcpkg install asio
vcpkg install cpp-httplib
vcpkg install websocketpp

cd ../
git clone https://github.com/mariadb-corporation/mariadb-connector-cpp.git
cd mariadb-connector-cpp
mkdir build
cd build
cmake ..
cmake --build . --config Release

cd ../../
mkdir build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cd build
cmake --build . --config Release
