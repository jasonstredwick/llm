#include <iostream>
#include <zlib.h>
#include <curl/curl.h>
#include <nghttp2/nghttp2.h>

int main() {
    std::cout << "Dependency Header Verification:" << std::endl;
    std::cout << " - zlib version: " << ZLIB_VERSION << std::endl;
    std::cout << " - curl version: " << LIBCURL_VERSION << std::endl;
    std::cout << " - nghttp2 version: " << NGHTTP2_VERSION << std::endl;
    
    std::cout << "\nSUCCESS: All headers included correctly from Unified Sysroot." << std::endl;
    return 0;
}
