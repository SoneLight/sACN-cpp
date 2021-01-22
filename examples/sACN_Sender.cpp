#include <sacn-cpp.hpp>
#include <asio_standalone_or_boost.hpp>
#include <thread>
#include <chrono>

int main()
{
    sACNcpp::sACNOutput output("sACN-cpp");

    if(!output.start())
        exit(1);

    if(!output.addUniverse(1))
        exit(1);

    output[1]->dmx().set(2,255);

    while(1) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        output[1]->dmx().set(1,output[1]->dmx()[1]+1);
    }
}