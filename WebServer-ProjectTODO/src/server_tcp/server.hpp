#include <asio.hpp>
#include <functional>

class TCP_Server 
{
private:
    const uint16_t _PORT = 6969;
    std::function<asio::awaitable<bool>(std::string*, std::string*)> _request_handler;

    asio::awaitable<void> session(asio::ip::tcp::socket socket);
    asio::awaitable<void> listener();
public:
    TCP_Server(std::function<asio::awaitable<bool>(std::string*, std::string*)> request_handler);
    void start();
    void start(asio::io_context &context);
};