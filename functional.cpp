#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
using namespace std;

template <class>
class Functional;

template <class R, class ...Args>
class Functional<R(Args...)> {

private:
    struct FuncBase {
        virtual R operator()(Args ...args)  = 0;
        virtual ~FuncBase() = default;
    };


    template <class F>
    struct FuncImpl: FuncBase {
        F m_f;

       FuncImpl(F f): m_f(std::move(f))  {}

       virtual R operator()(Args ...args) {
            return m_f(std::forward<Args>(args)...);
       }

    };
    shared_ptr<FuncBase> m_ptr;
public:

    Functional() = default;

    template<class F>
    Functional(F f): m_ptr(make_shared<FuncImpl<F>>(std::move(f))) {}

    R operator()(Args ...args) const {
        if (!m_ptr)
            throw std::runtime_error("function not initialized");
        return (*m_ptr)(std::forward<Args>(args)...);
    }

};
 
struct Foo
{
    Foo(int num) : num_(num) {}
    void print_add(int i) const { std::cout << num_ + i << '\n'; }
    int num_;
};
 
void print_num(int i)
{
    std::cout << i << '\n';
}
 
struct PrintNum
{
    void operator()(int i) const
    {
        std::cout << i << '\n';
    }
};
 
int main()
{
    // store a free function
    Functional<void(int)> f_display = print_num;
    f_display(-9);
 
    // store a lambda
    Functional<void()> f_display_42 = []() { print_num(42); };
    f_display_42();
 
    // store a call to a function object
    Functional<void(int)> f_display_obj = PrintNum();
    f_display_obj(18);

}