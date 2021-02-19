#include <iostream>
#include "../include/util.h"

int main(){
    const char* str1 = "123";
    const char* str2 = "456";
    int64_t timemicro = handy::util::timeMicro();
    int64_t steadymicro = handy::util::steadyMicro();

    std::cout << handy::util::atoi(str2) - handy::util::atoi2(str1,str1+3) << std::endl;

    std::cout << timemicro << " " << steadymicro << " " 
        << handy::util::readableTime(timemicro/1000000) << " " 
        << handy::util::readableTime(steadymicro/1000000) << std::endl;
    
    return 0;
}