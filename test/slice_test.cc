#include <iostream>
#include "../include/slice.h"

int main(){
    handy::Slice msg1("  im laji  ");
    handy::Slice msg2("\nyueyue is genius\nruirui is pig\n");
    handy::Slice msg3("c++ is very good");
    std::cout << "data: " << msg3.data() << ", begin: " << msg3.begin() 
        << ", end: " << msg3.end() << ", front: " << msg3.front() 
        << ", back: " << msg3.back() << ", size: " << msg3.size() << std::endl;
    bool flag = msg1!=msg3;
    std::cout << "msg1 != msg3: " << flag << std::endl;
    std::cout << "[4]: " << msg3[4] << std::endl;
    std::cout << "sub: " << msg3.sub(7,-5).data() << std::endl;
    std::cout << "eat: " << msg3.eat(4).data() << std::endl;
    std::cout << "eated data: " << msg3.data() << std::endl;
    //msg3.resize(10);
    std::cout << "data: " << msg3.data() << ", begin: " << msg3.begin() 
        << ", end: " << msg3.end() << ", front: " << msg3.front() 
        << ", back: " << msg3.back() << ", size: " << msg3.size() << std::endl;
    std::cout << msg3.empty() << std::endl;
    msg3.clear();
    std::cout << msg3.empty() << std::endl;
    std::cout << msg1.data() << msg1.trimSpace().data() << std::endl;
    /*std::cout << "data: " << msg1.data() << ", size: " << msg1.size() << std::endl;
    std::cout << "data: " << msg1.eatWord().data() << ", size: " << msg1.eatWord().data() << std::endl;
    std::cout << "data: " << msg2.data() << ", size: " << msg2.size() << std::endl;
    std::cout << "data: " << msg2.eatLine().data() << ", size: " << msg2.eatLine().size() << std::endl;
    */
    return 0;
}