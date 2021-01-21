#include <sacn_output.hpp>
#include <asio_standalone_or_boost.hpp>
#include <thread>
#include <chrono>

int main()
{
    sACNcpp::sACNOutput output("sACN-cpp", nullptr, 5, "192.168.10.107");

    if(!output.start())
        exit(1);

    if(!output.addUniverse(1))
        exit(1);

    for(int i = 0; i < 512; i++)
    {
        output[1]->dmx().set(i,i);
    }

    while(1) std::this_thread::sleep_for(std::chrono::seconds(5));
}