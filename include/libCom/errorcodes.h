//
// Created by pcpcpc on 27.10.2015.
//

#ifndef CORE_ERRORCODES_H
#define CORE_ERRORCODES_H

enum class IOResult {
    // internal error codes
    ERROR_INVALID_PACKAGE,
    ERROR_INTERNAL,
    ERROR_SIZE,
    ERROR_INVALID_HEADER,

    // server results
    STATUS_SUCCESS = 128,
    STATUS_NO_MESSAGES = 129,
    STATUS_MORE_MESSAGES = 130
};

#endif //CORE_ERRORCODES_H
