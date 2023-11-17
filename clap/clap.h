#pragma once
#include "argument.h"
#include <vector>
#include <any>

template<typename... T>
concept IsArg = requires {
    (..., std::same_as<Arg, T>);
};

class Clap final {
    std::string progname_;
    std::vector<Arg> data_;
public:
    Clap() = default;
    template<typename... T>
//    requires IsArg<T...>
    explicit Clap(std::string progname, T... args) : progname_{std::move(progname)}{
        (..., data_.push_back(args));
    }

    Clap& add(Arg arg) noexcept {
        data_.push_back(std::move(arg));
        return *this;
    }

};
