#include <iostream>
#include <array>

#include "server.hpp"

// HTTP/1.1 200 OK\r\nContent-Length: 12\r\nContent-Type: text/plain; charset=utf-8\r\n\r\nHello World!

TCP_Server::TCP_Server(std::function<asio::awaitable<bool>(std::string*, std::string*)> request_handler) : _request_handler(request_handler) {}

asio::awaitable<void> TCP_Server::session(asio::ip::tcp::socket socket)
{
    std::array<char, 2048> buffer;
    std::string request, response{};

    while (true)
    {
        std::size_t len = co_await socket.async_read_some(
            asio::buffer(buffer),
            asio::use_awaitable
        );

        request = std::string(buffer.data(), len);

        std::cout << "Client sent {" << len << "} bytes:\n";// << request << '\n';

        if (request == "EXIT")
        {
            std::cout << "Client disconnected\n";
            break;
        }

        if(_request_handler) 
        {
            bool ready = co_await _request_handler(&request, &response);
            if(!ready)
            {
                continue;
            }
        }
        
        co_await asio::async_write(
            socket,
            asio::buffer(response),
            asio::use_awaitable
        );
    }
}


asio::awaitable<void> TCP_Server::listener()
{
    auto executor = co_await asio::this_coro::executor;

    asio::ip::tcp::acceptor acceptor(
        executor,
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), _PORT)
    );

    std::cout << "Server started on port " << _PORT << '\n';
    
    while(true)
    {
        asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);

        std::cout << "New client connected\n";
    
        asio::co_spawn(
            executor,
            session(std::move(socket)),
            asio::detached
        );
    
    }
}

void TCP_Server::start()
{
    try
    {
        asio::io_context context;

        asio::co_spawn(
            context,
            listener(),
            asio::detached 
        );

        context.run();
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}

void TCP_Server::start(asio::io_context &context)
{
    try
    {
        asio::co_spawn(
            context,
            listener(),
            asio::detached 
        );

        context.run();
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}