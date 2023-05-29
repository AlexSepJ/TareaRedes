#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>

const int puerto = 77777;
const int clientes_max = 2;
const int tamano_tablero = 15;

std::mutex mtx;

struct Jugador {
    int socket;
    std::string nombre;
    std::vector<std::vector<char>> tablero;
};

std::vector<Jugador> jugadores;
std::vector<std::thread> threads;

void Conexion(int socket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesLeidos = recv(socket, buffer, sizeof(buffer), 0);

    std::string nombreJugador(buffer);
    nombreJugador = nombreJugador.substr(0, nombreJugador.find('\n'));

    mtx.lock();
    Jugador nuevoJugador;
    nuevoJugador.socket = socket;
    nuevoJugador.nombre = nombreJugador;
    nuevoJugador.tablero.resize(tamano_tablero, std::vector<char>(tamano_tablero, ' '));
    jugadores.push_back(nuevoJugador);
    mtx.unlock();

    std::cout << "Jugador " << nombreJugador << " conectado." << std::endl;

    if (jugadores.size() == clientes_max) {
        // Iniciar el juego
        std::string mensaje = "El juego ha comenzado!\n";
        send(jugadores[0].socket, mensaje.c_str(), mensaje.size(), 0);
        send(jugadores[1].socket, mensaje.c_str(), mensaje.size(), 0);
        
        bool terminarJuego = false;

        while (!terminarJuego) {
            // Turno del primer jugador
            memset(buffer, 0, sizeof(buffer));
            int bytesLeidos = recv(jugadores[0].socket, buffer, sizeof(buffer), 0);
            if (bytesLeidos <= 0) {
                terminarJuego = true;
                break;
            }

            // Procesar el ataque del primer jugador
            int fila = buffer[0] - '0';
            int col = buffer[1] - '0';

            if (jugadores[1].tablero[fila][col] == 'X') {
                jugadores[1].tablero[fila][col] = 'O';
                mensaje = "Acierto!\n";
                send(jugadores[0].socket, mensaje.c_str(), mensaje.size(), 0);
                send(jugadores[1].socket, mensaje.c_str(), mensaje.size(), 0);
            } else {
                jugadores[1].tablero[fila][col] = ' ';
                mensaje = "Fallo!\n";
                send(jugadores[0].socket, mensaje.c_str(), mensaje.size(), 0);
                send(jugadores[1].socket, mensaje.c_str(), mensaje.size(), 0);
            }

            // Verificar si el primer jugador ganó
            bool jugador1Gana = true;
            for (int i = 0; i < tamano_tablero; ++i) {
                for (int j = 0; j < tamano_tablero; ++j) {
                    if (jugadores[1].tablero[i][j] == 'X') {
                        jugador1Gana = false;
                        break;
                    }
                }
                if (!jugador1Gana)
                    break;
            }

            if (jugador1Gana) {
                mensaje = "¡El jugador " + jugadores[0].nombre + " ha ganado!\n";
                send(jugadores[0].socket, mensaje.c_str(), mensaje.size(), 0);
                send(jugadores[1].socket, mensaje.c_str(), mensaje.size(), 0);
                terminarJuego = true;
                break;
            }

            // Turno del segundo jugador
            memset(buffer, 0, sizeof(buffer));
            bytesLeidos = recv(jugadores[1].socket, buffer, sizeof(buffer), 0);
            if (bytesLeidos <= 0) {
                terminarJuego = true;
                break;
            }

            // Procesar el ataque del segundo jugador
            fila = buffer[0] - '0';
            col = buffer[1] - '0';

            if (jugadores[0].tablero[fila][col] == 'X') {
                jugadores[0].tablero[fila][col] = 'O';
                mensaje = "Acierto!\n";
                send(jugadores[0].socket, mensaje.c_str(), mensaje.size(), 0);
                send(jugadores[1].socket, mensaje.c_str(), mensaje.size(), 0);
            } else {
                jugadores[0].tablero[fila][col] = ' ';
                mensaje = "Fallo!\n";
                send(jugadores[0].socket, mensaje.c_str(), mensaje.size(), 0);
                send(jugadores[1].socket, mensaje.c_str(), mensaje.size(), 0);
            }

            // Verificar si el segundo jugador ganó
            bool jugador2Gana = true;
            for (int i = 0; i < tamano_tablero; ++i) {
                for (int j = 0; j < tamano_tablero; ++j) {
                    if (jugadores[0].tablero[i][j] == 'X') {
                        jugador2Gana = false;
                        break;
                    }
                }
                if (!jugador2Gana)
                    break;
            }

            if (jugador2Gana) {
                mensaje = "¡El jugador " + jugadores[1].nombre + " ha ganado!\n";
                send(jugadores[0].socket, mensaje.c_str(), mensaje.size(), 0);
                send(jugadores[1].socket, mensaje.c_str(), mensaje.size(), 0);
                terminarJuego = true;
                break;
            }
        }

        // Cerrar la conexión y finalizar el juego
        close(jugadores[0].socket);
        close(jugadores[1].socket);

        std::cout << "Juego terminado." << std::endl;
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket" << std::endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(puerto);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error al enlazar el socket" << std::endl;
        return -1;
    }

    if (listen(serverSocket, clientes_max) == -1) {
        std::cerr << "Error al escuchar en el socket" << std::endl;
        return -1;
    }

    std::cout << "Servidor iniciado, esperando jugadores" << std::endl;

    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);

        if (clientSocket == -1) {
            std::cerr << "Error al aceptar la conexión del cliente" << std::endl;
            return -1;
        }

        std::thread t(Conexion, clientSocket);
        threads.push_back(std::move(t));

        if (threads.size() >= clientes_max) {
            for (auto& thread : threads)
                thread.join();
            break;
        }
    }

    close(serverSocket);
    return 0;
}