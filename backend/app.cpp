#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    hostname[255] = '\0';

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "ERROR: Failed to create socket" << std::endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "ERROR: Failed to set socket options" << std::endl;
        return 1;
    }

    // Bind to port 8080
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "ERROR: Failed to bind to port 8080" << std::endl;
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "ERROR: Failed to listen on port 8080" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 8080 (hostname: " << hostname << ")" << std::endl;

    // Accept connections in loop
    while(true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd < 0) {
            std::cerr << "ERROR: Failed to accept connection" << std::endl;
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        // Proper HTTP response with correct headers
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: " + std::to_string(32 + strlen(hostname)) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += "Served by backend: " + std::string(hostname) + "\n";

        send(client_fd, response.c_str(), response.length(), 0);
        
        // Ensure all data is sent
        shutdown(client_fd, SHUT_WR);
        close(client_fd);
        
        std::cout << "Served request from " << client_ip << std::endl;
    }

    return 0;
}
