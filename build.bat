echo QingLiaoChatServerBuilder

vcpkg install kcp:x64-windows
vcpkg install openssl:x64-windows
vcpkg install asio:x64-windows
vcpkg install cpp-httplib:x64-windows
vcpkg install websocketpp:x64-windows

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

pause
