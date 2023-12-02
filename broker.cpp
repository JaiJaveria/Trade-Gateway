#include "boost/asio.hpp"
#include "Message.h"
#include <iostream>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>{
private:
    tcp::socket m_socket;
    boost::asio::streambuf m_buffer;
    void process(){
        auto self(shared_from_this());
        boost::asio::async_read_until(
            m_socket, m_buffer, "$$",
            [this,self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if(!ec){
                    std::istream request_stream(&m_buffer);
                    std::string request;
                    std::getline(request_stream, request, '\0'); // Read until null termination

                    std::cout << "Received request: " << request << std::endl;
                    processMessage(request);
                }
                else{
                    std::cout << "Error reading request: " << ec.message() << std::endl;
                }
            }
        );
    }
public:
    Session(tcp::socket socket) : m_socket(std::move(socket)) {}
    void run(){
        process();
    }
};
class Server{
public:
    Server(boost::asio::io_context& io_context, short port)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)){
        do_accept();

    }
private:
    tcp::acceptor m_acceptor;
    void do_accept(){
        m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket){
            if(!ec){
                std::cout <<" creating session on "
                        << socket.remote_endpoint().address().to_string() 
                    << ":" << socket.remote_endpoint().port() << '\n';
                std::make_shared<Session>(std::move(socket))->run();
            }
            else{
                std::cout << "error: "<< ec.message();
            }
            do_accept();
        });
    }
};

int main(){
    boost::asio::io_context io;
    Server s(io, 25000);
    io.run();
}