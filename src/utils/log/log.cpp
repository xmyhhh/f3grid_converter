//
// Created by xmyci on 20/02/2024.
//
#include "log.h"

void log_print(std::string msg, int level, bool on) {

    for (int i = 0; i < level; i++) {
        std::cout << "   ";
    }
    std::cout << msg << "\n";
}