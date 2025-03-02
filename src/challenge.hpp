#pragma once

namespace argon {

inline std::string solveChallenge(int value) {
    return std::to_string(value ^ 0x5F3759DF);
}

}