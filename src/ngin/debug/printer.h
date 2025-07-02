#ifndef PRINTER_H
#define PRINTER_H

#include <string>

namespace ngin {
namespace debug {

class Printer {
public:
    Printer() {}
    // Adds a log message to the queue with the current timestamp and type
    virtual void info(const std::string& message, std::string type = "", unsigned int indent = 0) {}

private:
};

}
}

#endif // PRINTER_H