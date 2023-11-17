#pragma once
#include "argument.h"
#include <vector>
#include <any>
#include <fmt/core.h>

class Clap final {
    std::string progname_;
    std::vector<Arg> data_{};
public:
    Clap() = default;
    template<typename... T>
    explicit Clap(std::string progname, T... args) : progname_{std::move(progname)}{
        (..., data_.push_back(args));
        fmt::print("Liczba parametrów: {}\n", data_.size());
        data_.shrink_to_fit();
    }

    Clap& add(Arg arg) noexcept {
        data_.push_back(std::move(arg));
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& s, Clap const& c) noexcept;
};

std::ostream& operator<<(std::ostream& s, Clap const& c) noexcept {
    std::cout << c.progname_ << " (" << c.data_.size() << ")\n";

    for (auto const& arg : c.data_) {
        std::cout << '\t' << arg << '\n';
    }
    return s;
}