#include <string>
#include <sstream>

#include "format.h"

using std::string;

string Filler(int n) {
    return n < 10 ? "0" : "";
}

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int s = seconds % 60;
    std::stringstream ss;
    ss << Filler(hours) << hours << ":" << Filler(minutes) << minutes << ":" << Filler(s) << s;
    return ss.str();
}