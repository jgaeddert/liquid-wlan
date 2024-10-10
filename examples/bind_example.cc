// Test generation of wlan frame
#include <iostream>

#include "wlanframegen.hh"

int main(int argc, char*argv[])
{
    liquid::wlan::framegen fg;
    std::cout << fg << std::endl;
    printf("done.\n");
    return 0;
}

