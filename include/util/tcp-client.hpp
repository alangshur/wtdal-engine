#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

/*
    The generic TCPClient class is a synchronous TCP framework for
    sending/receiving client TCP connections and sending/receiving 
    client packet communications over these connections.
*/
template <typename T>
class TCPClient {
    public:
        TCPClient();
        void send_connection(const std::string& addr, uint16_t port);
        void read_packet(T& ds_packet);
        void write_packet(const T& ds_packet);
        void force_write_packet(const T& ds_packet) noexcept;
        void close_connection();
        void force_close_connection() noexcept;
        void stop_context();

    private:
        boost::asio::io_context io_context;
        tcp::resolver resolver;
        std::shared_ptr<tcp::socket> socket_ptr;
        bool active_connection;
};

template <typename T>
TCPClient<T>::TCPClient() : io_context(), resolver(this->io_context),
    socket_ptr(nullptr), active_connection(false) {}

template <typename T>
void TCPClient<T>::send_connection(const std::string& addr, uint16_t port) {
    if (this->active_connection) throw std::runtime_error("Connection accept failed since "
        "active connection already exists.");

    // accept queued connection
    this->socket_ptr = std::make_shared<tcp::socket>(tcp::socket(this->io_context));
    tcp::resolver::query query(addr, std::to_string(port));
    tcp::resolver::results_type endpoint = this->resolver.resolve(query);
    boost::asio::connect(*(this->socket_ptr), endpoint);
    this->active_connection = true;
}

template <typename T>
void TCPClient<T>::read_packet(T& ds_packet) {
    if (!this->active_connection) throw std::runtime_error("Packet read failed since "
        "there is no active connection.");

    // read generic packet T
    boost::asio::streambuf read_buf;
    boost::asio::read(*(this->socket_ptr), 
        read_buf, boost::asio::transfer_exactly(sizeof(T)));
    std::string s_packet((std::istreambuf_iterator<char>(&read_buf)), 
        std::istreambuf_iterator<char>());

    // deserialize packet
    memcpy(&ds_packet, s_packet.c_str(), sizeof(T));
}

template <typename T>
void TCPClient<T>::write_packet(const T& ds_packet) {
    if (!this->active_connection) throw std::runtime_error("Packet write failed since "
        "there is no active connection.");

    // serialize packet
    char s_packet[sizeof(T)];
    memcpy(&s_packet[0], &ds_packet, sizeof(T));

    // write generic packet T
    boost::asio::write(
        *(this->socket_ptr), 
        boost::asio::buffer(s_packet, sizeof(T))
    );
} 

template <typename T>
void TCPClient<T>::force_write_packet(const T& ds_packet) noexcept {
    try { this->write_packet(ds_packet); }
    catch (...) {}
}

template <typename T>
void TCPClient<T>::close_connection() {
    if (!this->active_connection) throw std::runtime_error("Connection close failed since "
        "there is no active connection.");

    // close active socket
    (*(this->socket_ptr)).close();
    this->active_connection = false;
}

template <typename T>
void TCPClient<T>::force_close_connection() noexcept {
    try { this->close_connection(); }
    catch (...) {}
}

template <typename T>
void TCPClient<T>::stop_context() {
    this->io_context.stop();
}

#endif