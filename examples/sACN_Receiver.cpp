#include <sacn_universe_input.hpp>
#include <asio_standalone_or_boost.hpp>
#include <thread>
#include <chrono>

int main()
{
    std::shared_ptr<asio::io_context> iocontext= std::make_shared<asio::io_context>();
    sACNcpp::sACNUniverseInput input(2, iocontext);

    input.start();

    while(1)
    {
        input.dmx().print();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}