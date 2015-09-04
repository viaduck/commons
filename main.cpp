#include <iostream>
#include "Socket.h"
#define enum_str( name ) # name

using namespace std;

std::string stringHelper(const uint8_t *data) {
    return std::string(reinterpret_cast<const char*>(data));
}

int main() {
    Socket s;
    if (s.init() != Socket::RESULT_SUCCESS)
        return 1;
    Socket::RESULT res = s.connect("127.0.0.1");
    std::cout << "Result: " << res << std::endl;

    Buffer buf;

    while (true) {
        Socket::RESULT recvRes = s.recv(&buf);

        if (recvRes == Socket::RESULT_RECVFAILED || recvRes == Socket::RESULT_CONNECTIONCLOSED)
            break;

        std::cout << "recv result: " << recvRes << std::endl;
        std::cout << "recv: " << stringHelper(buf.data()) << std::endl;

        s.send(buf);                    // echo it back

        buf.consume(buf.size());
    }


//    int *i = new int();
//    std::cout<<i<<" -> "<<(i != nullptr)<<std::endl;
//    *i = 10;
//    std::cout<<*i<<std::endl;
//    delete i;
//    std::cout<<i<<" -> "<<(i != nullptr)<<std::endl;
//    *i = 1337;
//    if (i)
//        std::cout<<"pointer is valid"<<std::endl;

//    int i = 10;
//    int *j = &i;
//    std::cout<<&i<<" "<<&j<<std::endl;
//
//    BufferTest test;
//    test.doTest();
//
//    SmartPtrTest test2;
//    test2.intTest();

    return 0;
}