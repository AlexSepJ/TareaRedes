#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>

const int puerto = 77777;
const int tamano_tablero = 15;

void mostrarTablero(const std::vector<std::vector<char>>& tablero) {
    std::cout << "   ";
    for (int i = 0; i < tamano_tablero; ++i)
        std::cout << i << " ";
    std::cout << std::endl;

    for (int i = 0; i < tamano_tablero; ++i) {
        std::cout << i << " |";
        for (int j = 0; j < tamano_tablero; ++j)
            std::cout << tablero[i][j] << "|";
        std::cout << std::endl;
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error al crear el socket" << std::endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(puerto);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        std::cerr << "Dirección inválida o no soportada" << std::endl;
        return -1;
    }

    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error al conectarse al servidor" << std::endl;
        return -1;
    }

    std::cout << "Ingrese su nombre: ";
    std::string playerName;
    std::getline(std::cin, playerName);

    send(clientSocket, playerName.c_str(), playerName.size(), 0);

    std::cout << "Esperando a que el juego comience" << std::endl;

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesLeidos = recv(clientSocket, buffer, sizeof(buffer), 0);
    std::cout << buffer;

    std::vector<std::vector<char>> tablero(tamano_tablero, std::vector<char>(tamano_tablero, ' '));

    while (true) {
        mostrarTablero(tablero);

        int fila, col;
        std::cout << "Ingrese las coordenadas de ataque (fila columna): ";
        std::cin >> fila >> col;

        if (fila < 0 || fila >= tamano_tablero || col < 0 || col >= tamano_tablero) {
            std::cerr << "Coordenadas inválidas, inténtelo de nuevo" << std::endl;
            continue;
        }

        buffer[0] = fila + '0';
        buffer[1] = col + '0';
        send(clientSocket, buffer, 2, 0);

        memset(buffer, 0, sizeof(buffer));
        bytesLeidos = recv(clientSocket, buffer, sizeof(buffer), 0);
        std::cout << buffer;

        if (std::string(buffer).find("¡El jugador") != std::string::npos)
            break;
    }

    close(clientSocket);
    return 0;
}