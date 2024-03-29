#pragma once

#include "Error.h"

template <typename T, typename E = Error>
class Result
{
public:
    Result(T val) : value(std::move(val)), b_error(false), b_has_value(true) {}
    Result(E err) : error(std::move(err)), b_error(true), b_has_value(true) {}
    Result() : b_error(false), b_has_value(false) {};

    Result(const Result& other) 
        : b_error(other.b_error)
        , b_has_value(other.b_has_value)
    {
        if (!b_has_value) return;

        if (b_error)
            error = std::move(other.error);
        else
            value = std::move(other.value);
    }

    inline const T& operator*() const { return value; }
    inline const E& get_error() const { return error; }

    bool is_error() const { return b_error; }
    bool has_value() const { return b_has_value; }

    ~Result()
    {
        if (!b_has_value) return;

        if (b_error)
            error.~E();
        else
            value.~T();
    }

private:
    const union
    {
        T value;
        E error;
    };

    bool b_error;
    bool b_has_value;
};