#include <sacn_universe_output.hpp>
#include <asio_standalone_or_boost.hpp>
#include <thread>
#include <chrono>

int main()
{
    std::shared_ptr<asio::io_context> iocontext= std::make_shared<asio::io_context>();
    sACNUniverseOutput output(2, "sACN-cpp", iocontext);

    for(int i = 0; i < 512; i++)
    {
        output.dmx().set(i,i);
    }

    output.start();

    while(1) std::this_thread::sleep_for(std::chrono::seconds(5));
}