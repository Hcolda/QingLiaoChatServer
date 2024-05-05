echo QingLiaoChatServerBuilder

:: 设置vcpkg环境变量
:: set vcpkg path
set VCPKG_PATH=F:\Desktop\c++\vcpkg

vcpkg install kcp:x64-windows
vcpkg install openssl:x64-windows
vcpkg install asio:x64-windows
vcpkg install cpp-httplib:x64-windows
vcpkg install websocketpp:x64-windows

git clone https://github.com/mariadb-corporation/mariadb-connector-cpp.git
cd mariadb-connector-cpp
mkdir build
cd build
cmake ..
cmake --build . --config Release -j8

cd ../../
mkdir build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cd build
cmake --build . --config Release -j8

pause
