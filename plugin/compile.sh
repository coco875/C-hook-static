g++ -I`gcc -print-file-name=plugin`/include -o plugin.dll plugin.cc `gcc -print-file-name=plugin`/cc1.exe.a -shared -m64 -fPIC -fno-rtti -Wl,--export-all-symbols 