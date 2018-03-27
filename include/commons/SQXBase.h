#ifndef COMMONS_SQXBASE_H
#define COMMONS_SQXBASE_H

#include <stdexcept>

class SQXBase {
public:
    struct load_exception : public std::runtime_error {
        explicit load_exception(const std::string &what) : std::runtime_error(what) {}
    };

    virtual sqlite::cryptosqlite_database *db() = 0;

protected:
    virtual void process() {};
};

#endif //COMMONS_SQXBASE_H
