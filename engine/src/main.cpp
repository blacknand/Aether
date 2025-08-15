#include "core/OrderBook.h"

#include "network/AetherServer.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "[ERROR] Usage: ./main <securities.csv>";
        return -1;
    }
     
    RunServer(argv[1]);
    return 0;
}