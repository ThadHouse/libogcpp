#pragma once

#include <ogc/lwp.h>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>

namespace gc
{
class thread
{
public:
  template <class Callable, class... Args>
  explicit thread(Callable &&f, Args &&... args)
  {
    StartThread(makeState(__make_invoker(std::forward<Callable>(f), std::forward<args>(args)...)));
  }

  thread() noexcept : m_thread{0} {}
  ~thread() noexcept
  {
    if (joinable())
    {
      void *param = nullptr;
      LWP_JoinThread(m_thread, &param);
    }
  }
  thread(thread &&other) noexcept
  {
    m_thread = other.m_thread;
    other.m_thread = 0;
  }
  thread &operator=(thread &&other) noexcept
  {
    if (joinable())
    {
      void *param = nullptr;
      LWP_JoinThread(m_thread, &param);
    }
    m_thread = other.m_thread;
    other.m_thread = 0;
    return *this;
  }
  thread(const thread &) = delete;
  thread &operator=(const thread &) = delete;

  bool joinable() { return m_thread != 0; }
  void join()
  {
    void *param = nullptr;
    LWP_JoinThread(m_thread, &param);
  }

private:
  lwp_t m_thread;

  struct State
  {
    virtual ~State();
    virtual void m_run() = 0;
  };
  using StatePtr = std::unique_ptr<State>;

  template <typename Callable>
  struct StateImpl : public State
  {
    Callable m_func;
    StateImpl(Callable &&f) : m_func(std::forward<Callable>(f)) {}
    void m_run() override { m_func(); }
  };

  void StartThread(StatePtr state)
  {
    LWP_CreateThread(&m_thread, [](void *param) -> void * {
      auto state = reinterpret_cast<gc::thread::State *>(param);
      state->m_run();
      return nullptr;
    },
                     state.get(), nullptr, 0, 50);
  }

  template <typename Callable>
  static StatePtr makeState(Callable &&f)
  {
    using impl = StateImpl<Callable>;
    return std::make_unique<impl>(std::forward<Callable>(f));
  }

  // A call wrapper that does INVOKE(forwarded tuple elements...)
  template <typename _Tuple>
  struct _Invoker
  {
    _Tuple _M_t;

    template <size_t _Index>
    static std::__tuple_element_t<_Index, _Tuple> &&
    _S_declval();

    template <size_t... _Ind>
    auto
    _M_invoke(std::_Index_tuple<_Ind...>) noexcept(noexcept(std::__invoke(_S_declval<_Ind>()...)))
        -> decltype(std::__invoke(_S_declval<_Ind>()...))
    {
      return std::__invoke(std::get<_Ind>(std::move(_M_t))...);
    }

    using _Indices = typename std::_Build_index_tuple<std::tuple_size<_Tuple>::value>::__type;

    auto
    operator()() noexcept(noexcept(std::declval<_Invoker &>()._M_invoke(_Indices())))
        -> decltype(std::declval<_Invoker &>()._M_invoke(_Indices()))
    {
      return _M_invoke(_Indices());
    }
  };

  template <typename... _Tp>
  using __decayed_tuple = std::tuple<typename std::decay<_Tp>::type...>;

  // Returns a call wrapper that stores
  // tuple{DECAY_COPY(__callable), DECAY_COPY(__args)...}.
  template <typename _Callable, typename... _Args>
  static _Invoker<__decayed_tuple<_Callable, _Args...>>
  __make_invoker(_Callable &&__callable, _Args &&... __args)
  {
    return {__decayed_tuple<_Callable, _Args...>{
        std::forward<_Callable>(__callable), std::forward<_Args>(__args)...}};
  }
};
} // namespace gc
