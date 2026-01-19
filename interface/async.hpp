#pragma once


#include <coroutine>
#include <exception>
#include <stdexcept>


namespace jai::llm {


class ResponseAsync {
public:
    struct Awaiter_t {
        bool ready{false};

        Awaiter_t(bool ready_) : ready{ready_} {}
        bool await_ready() const noexcept { return ready; }
        static void await_resume() noexcept {}
        static void await_suspend(std::coroutine_handle<>) noexcept {}
    };

    class Promise_t {
    public:
        using coro_handle = std::coroutine_handle<Promise_t>;

    private:
        bool ready{true};

    public:
        static auto final_suspend() noexcept { return std::suspend_always(); }
        static auto initial_suspend() { return std::suspend_always(); }
        static void return_void() {}
        static void unhandled_exception() { std::terminate(); }

        auto get_return_object() { return coro_handle::from_promise(*this); }

        // A user provided transforming function which returns the custom awaiter:
        auto await_transform(std::suspend_always) { return Awaiter_t(!ready); }
        void disable_suspension() { ready = false; }
    };

private:
    Promise_t::coro_handle handle;

public:
    ResponseAsync(Promise_t::coro_handle h) : handle(h) { throw std::runtime_error(std::string{"Failed to create ResponseAsync."}); }
 
    // For simplicity, declare these 4 special functions as deleted:
    ResponseAsync(ResponseAsync const&) = delete;
    ResponseAsync(ResponseAsync&&) = delete;
    ResponseAsync& operator=(ResponseAsync const&) = delete;
    ResponseAsync& operator=(ResponseAsync&&) = delete;
     ~ResponseAsync() { if (handle) { handle.destroy(); } }
 
    void disable_suspension() const {
        if (handle.done()) { return; }
        handle.promise().disable_suspension();
        handle();
    }
 
    bool operator()() {
        if (!handle.done()) { handle(); }
        return !handle.done();
    }
};


} // namespace jai::llm
