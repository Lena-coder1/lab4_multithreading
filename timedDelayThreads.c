#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define sleep(x) Sleep(1000 * (x))
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define PORT 8080

// Structure to hold client data for threading
typedef struct {
    SOCKET client_socket;
    int client_id;
} client_data;

void handle_client(SOCKET client_socket, int client_id) {
    char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello Client!";
    
    printf("[Server] Handling client %d...\n", client_id);
    
    // Simulating "bottleneck" or heavy processing
    printf("[Server] Processing request...\n");
    sleep(5); // the fake bottleneck: the server is "busy" for 5 sec before responding to the client
    
    send(client_socket, message, (int)strlen(message), 0);
    printf("[Server] Response sent to client %d. Closing connection.\n", client_id);
    
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}


// Thread function to handle each client in a separate thread
void *client_thread(void *arg) {
    client_data *data = (client_data *)arg;
    handle_client(data->client_socket, data->client_id);
    free(data);
    return NULL;
}


int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    int client_count = 0;

    server_socket = socket(AF_INET, SOCK_STREAM, 0); // this creates a tcp sockets where af_inet is IPv4 and Sock_stream is TCP
    
    server_addr.sin_family = AF_INET; // use IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // binding all available interfaces
    server_addr.sin_port = htons(PORT); //converts the port number to network byte order 8080

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)); // bind the socket to the specified address and port 
    listen(server_socket, 5); // listen for incoming connections with a backlog of 5 sec

    printf("Server listening on port %d...\n", PORT);
    printf("NOTE: This server is SEQUENTIAL. It can only handle one client at a time!\n\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket != INVALID_SOCKET) {
            client_count++;
            
            //threaded version to handle mutiple clients concurrently:
            pthread_t tid;
            client_data *data = malloc(sizeof(client_data));
            data->client_socket = client_socket;
            data->client_id = client_count;

            pthread_create(&tid, NULL, client_thread, data);
            pthread_detach(tid);
        }
    }

#ifdef _WIN32
    closesocket(server_socket);
    WSACleanup();
#else
    close(server_socket);
#endif
    return 0;
}
