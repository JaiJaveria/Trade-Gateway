#include "boost/asio.hpp"
#include <iostream>
using tcp = boost::asio::ip::tcp;
class Strategy{
public:
    tcp::socket onLogin(tcp::socket socket){
        std::string data("35=A|49=gateway|56=broker|108=30$$");
        auto result=boost::asio::write(socket, boost::asio::buffer(data));
        return std::move(socket);
    }
};
int main(){
    boost::asio::io_service io;
    tcp::socket socket(io);
    tcp::resolver resolver(io);
    Strategy *strat= new Strategy();
    boost::asio::connect(socket, resolver.resolve("127.0.0.1", "25000"));
    socket = strat->onLogin(std::move(socket));

    boost::system::error_code ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket.close();

}