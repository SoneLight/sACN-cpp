# sACN-cpp

A cross-platform, header only C++ implementation of the E1.31 standard (sACN).

This library uses the package structure from [libe131](https://github.com/hhromic/libe131) and supplements it with C++ accessors and getters for the package structure, high level sACN-socket classes and multithreaded sACN senders and receivers.

To ensure cross-platform-compability in the networking code, the [asio-library](https://think-async.com/Asio/) is used. You can use asio standalone, or use it included in [boost](https://www.boost.org/) if your project requires boost anyway.

![Tests](https://github.com/SoneLight/sACN-cpp/workflows/Tests/badge.svg)

## Installation

See the examples folder for a small usage example. To use the library, simply add the include-folder to your projects include-directories. The CMakeLists.txt also shows how to choose between Asio standalone or boost::asio.

## Documentation

[![Documentation Status](https://readthedocs.org/projects/sacn-cpp/badge/?version=latest)](https://sacn-cpp.readthedocs.io/en/latest/?badge=latest)

## Roadmap

- Generating UUID's to send when sending sACN (High prio)
- Considering source priorities. This requires parsing sACN's discovery packets.
- Providing frame rate information for sACNUniverseOutput and -Input.
