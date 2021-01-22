#include <sacn_input.hpp>
#include <asio_standalone_or_boost.hpp>
#include <thread>
#include <chrono>

int main()
{
    sACNcpp::sACNInput input;

    if(!input.start())
        exit(1);

    if(!input.addUniverse(1))
        exit(1);

    if(!input.addUniverse(2))
        exit(1);

    while(1)
    {
        //input[1]->dmx().print();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}