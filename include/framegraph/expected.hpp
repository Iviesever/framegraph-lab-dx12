#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace framegraph {
template<class E> struct UnexpectedValue { E error; };
template<class E> UnexpectedValue<E> unexpected(E error) { return {std::move(error)}; }

template<class T, class E> class Expected {
public:
    Expected() requires std::is_default_constructible_v<T> : storage_(std::in_place_index<0>) {}
    Expected(const T& value) : storage_(std::in_place_index<0>, value) {}
    Expected(T&& value) : storage_(std::in_place_index<0>, std::move(value)) {}
    Expected(UnexpectedValue<E> value) : storage_(std::in_place_index<1>, std::move(value.error)) {}
    bool has_value() const noexcept { return storage_.index() == 0; }
    explicit operator bool() const noexcept { return has_value(); }
    T& operator*() & { return std::get<0>(storage_); }
    const T& operator*() const& { return std::get<0>(storage_); }
    T&& operator*() && { return std::get<0>(std::move(storage_)); }
    T* operator->() { return std::addressof(std::get<0>(storage_)); }
    const T* operator->() const { return std::addressof(std::get<0>(storage_)); }
    E& error() & { return std::get<1>(storage_); }
    const E& error() const& { return std::get<1>(storage_); }
private:
    std::variant<T,E> storage_;
};

template<class E> class Expected<void,E> {
public:
    Expected() = default;
    Expected(UnexpectedValue<E> value) : error_(std::move(value.error)) {}
    bool has_value() const noexcept { return !error_; }
    explicit operator bool() const noexcept { return has_value(); }
    E& error() & { return *error_; }
    const E& error() const& { return *error_; }
private:
    std::optional<E> error_;
};
}
