#include "boost/asio.hpp"
#include <iostream>
using tcp = boost::asio::ip::tcp;
class Gateway : public std::enable_shared_from_this<Gateway>{
private:
    tcp::socket socket;
    tcp::resolver resolver;
    boost::asio::streambuf m_buffer;

public:
    Gateway(boost::asio::io_service& io) : socket(io), resolver(io) {}
    void onLogin(){
        std::string data("8=FIX.4.4|9=100|35=A|49=gateway|56=broker|108=30|$");
        auto result=boost::asio::write(socket, boost::asio::buffer(data));
    }
    void process(){
        boost::asio::async_read_until(
            socket, m_buffer, "$",
            [this](boost::system::error_code ec, std::size_t /*length*/)
            {
                if(!ec){
                    std::istream request_stream(&m_buffer);
                    std::string request;
                    std::getline(request_stream, request, '\0'); // Read until null termination

                    std::cout << "Received data: " << request << std::endl;
                    process();
                }
                else{
                    std::cout << "Error reading request: " << ec.message() << std::endl;
                }
            }
        );
    }
    void onConnect(){
    boost::asio::connect(socket, resolver.resolve("127.0.0.1", "25000"));
   } 
};
int main(){
    boost::asio::io_service io;
    Gateway *gateway= new Gateway(io);
    gateway->onConnect();
    gateway->onLogin();
    gateway->process();
    
    io.run();
    // boost::system::error_code ec;
    // socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    // socket.close();

}