#include "boost/asio.hpp"
#include "Message.h"
#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <functional>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>{
private:
    tcp::socket m_socket;
    boost::asio::streambuf m_buffer;
    std::string senderCompId;
    int heartbeatInterval;
    boost::asio::io_context& io;
    boost::asio::deadline_timer heartbeatTimer{io};
    void process(){
        auto self(shared_from_this());
        boost::asio::async_read_until(
            m_socket, m_buffer, "$",
            [this,self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if(!ec){
                    std::istream request_stream(&m_buffer);
                    std::string request;
                    std::getline(request_stream, request, '\0'); // Read until null termination

                    std::cout << "Received request: " << request << std::endl;
                    processMessage(request);
                    process();
                }
                else{
                    std::cout << "Error reading request: " << ec.message() << std::endl;
                }
            }
        );
    }
    void processKeyValue(int tag,std::string value){
        switch (tag)
        {
            case FIX::Tag::MsgType:{
                char val=value[0];
                switch (val)
                {
                    case FIX::MsgType::Logon:
                        break;
                    
                    default:
                        break;
                }
                break;
            }
            case FIX::Tag::HeartbeatInterval:{
                heartbeatInterval=stoi(value);
                boost::asio::deadline_timer t(io, boost::posix_time::seconds(heartbeatInterval));
                heartbeatTimer.expires_from_now(boost::posix_time::seconds(heartbeatInterval));
                auto self(shared_from_this());
                heartbeatTimer.async_wait(boost::bind(sendHeartbeat, boost::asio::placeholders::error, self));
                break;
            }
            case FIX::Tag::SenderCompId: {
                senderCompId=value;
                break;
            }   
            default:
                break;
        }
    }
    void processMessage(std::string data){
        int tag;
        bool foundTag=false;
        std::string value;
        for(int i=0; i<data.size(); i++){
            if(data[i]=='$'){
                break;
            }
            if(data[i]=='|'){
                //first process the received value
                processKeyValue(tag,value);
                tag=0;
                foundTag=false;
                value="";
                continue;
            }
            if(data[i]=='=')
            {
                foundTag=true;
                continue;
            }
            if(foundTag){
                value+=data[i];
            }
            else{
                tag*=10;
                tag+=data[i]-'0';
            }

        }
    }
public:
    Session(tcp::socket socket, boost::asio::io_context& ioC) : m_socket(std::move(socket)), io(ioC)
    {}
    void run(){
        process();
    }
    std::string getSenderCompId(){
        return senderCompId;
    }
    static void sendHeartbeat(const boost::system::error_code& /*e*/, std::shared_ptr<Session> session){
        std::string data{"8=FIX.4.4|9=100|35=0|49=broker|56="};
        data+=session->getSenderCompId();
        data+="|$";
        std::cout << "sending heartbeat: "<< data<<"\n";
        auto result=boost::asio::write(session->m_socket, boost::asio::buffer(data));
        session->heartbeatTimer.expires_from_now(boost::posix_time::seconds(session->heartbeatInterval));
        session->heartbeatTimer.async_wait(boost::bind(sendHeartbeat, boost::asio::placeholders::error, session));
                

    }
};
class Server{
public:
    Server(boost::asio::io_context& io_context, short port)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)), io(io_context){
        do_accept();

    }
private:
    tcp::acceptor m_acceptor;
    boost::asio::io_context& io;
    void do_accept(){
        m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket){
            if(!ec){
                std::cout <<" creating session on "
                        << socket.remote_endpoint().address().to_string() 
                    << ":" << socket.remote_endpoint().port() << '\n';
                auto session =std::make_shared<Session>(std::move(socket), io);
                session->run();
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