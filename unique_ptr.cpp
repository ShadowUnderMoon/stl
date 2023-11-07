#include <cassert>
#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <utility>
#include <concepts>
using namespace std;

struct B
{
    virtual ~B() = default;
    virtual void bar() {std::cout << "B::bar\n"; }
};

struct D: B
{
    D() {std::cout << "D::D\n"; }
    ~D() { std::cout << "D::~D\n"; }
    void bar() override { std::cout << "D::bar\n"; }
};


void close_file(FILE* fp)
{
    cout << "close file\n";
    fclose(fp);
}

template<class T>
struct DefaultDeleter
{
    void operator() (T* p) const
    {
        delete p;
    }

};

template<>
struct DefaultDeleter<FILE> {
    void operator()(FILE* p) const {
        fclose(p);
    }
};


template<class T, class Deleter=DefaultDeleter<T>>
class Unique_ptr
{
private:
    T* m_p;
public:
    template<class U, class E>
    friend class Unique_ptr;
    Unique_ptr(std::nullptr_t p = nullptr) {
        m_p = p;
    }
    explicit Unique_ptr(T* p) {
        m_p = p;
    }

    Unique_ptr(Unique_ptr const &that) = delete;
    Unique_ptr& operator=(Unique_ptr const& that) = delete;

    Unique_ptr(Unique_ptr&& u) {
        cout << __PRETTY_FUNCTION__ << endl;
        m_p = u.m_p;
        u.m_p = nullptr;
    }
    template<class U, class E> requires (std::convertible_to<U *, T *>)
    Unique_ptr(Unique_ptr<U, E>&& u) {
        cout << __PRETTY_FUNCTION__ << endl;
        m_p = std::exchange(u.m_p, nullptr);
    }

    Unique_ptr& operator=(Unique_ptr&& that) {
        if (this != &that) {
            if (m_p)
                Deleter{}(m_p);
            m_p = exchange(that.m_p, nullptr);
        }
        return *this;
    }

    explicit operator bool() const {
        return bool(m_p);
    }

    T* get() const {
        return m_p;
    }
    
    T* release() {
        return exchange(m_p, nullptr);
    }
    
    T& operator*() const {
        return *m_p;
    }

    T* operator ->() const {
        return m_p;
    }

    ~Unique_ptr() {
        if (m_p) {
            Deleter{}(m_p);
        }
    }
};

template<class T, class ...Args>
Unique_ptr<T> make_Unique(Args&& ...args) {
    return Unique_ptr<T>(new T(std::forward<Args>(args)...));
}


Unique_ptr<D> pass_through(Unique_ptr<D> p)
{
    p->bar();
    return p;
}

struct MyClass {
    int a, b, c;
};

struct Animal {
    virtual void speak() = 0;
    virtual ~Animal() = default;
};

struct Dog : Animal {
    int age;

    Dog(int age_) : age(age_) {
    }

    virtual void speak() {
        printf("Bark! I'm %d Year Old!\n", age);
    }
};

struct Cat : Animal {
    int &age;

    Cat(int &age_) : age(age_) {
    }

    virtual void speak() {
        printf("Meow! I'm %d Year Old!\n", age);
    }
};

int main()
{
    cout << "1) Unique ownership sematics demo\n";
    {
        Unique_ptr<D> p = make_Unique<D>();
        Unique_ptr<D> q = pass_through(std::move(p));
        assert(!p);
    }

    cout << "\n" "2) Runtime polymorphism demo\n";
    {
        Unique_ptr<B> p = make_Unique<D>();
        p->bar();
    }

    cout << "\n" "3) Custom deleter demo\n";
    ofstream("demo.txt") << "x";
    {
        Unique_ptr<FILE> fp(fopen("demo.txt", "r"));
        if (fp)
            cout << char(fgetc(fp.get())) << "\n";
    }

    // cout << "\n" "4) Custom lambda-expression deleter and exception safety demo\n";
    // try
    // {
    //     Unique_ptr<D, void(*)(D*)> p(new D, [](D* ptr) 
    //     {
    //         cout << "destroying from a custom deleter...\n";
    //         delete ptr;
    //     });
    //     throw runtime_error("");
    // }
    // catch (const std::exception&)
    // {
    //     cout << "Caught exception\n";
    // }
    std::vector<Unique_ptr<Animal>> zoo;
    int age = 3;
    zoo.push_back(make_Unique<Cat>(age));
    zoo.push_back(make_Unique<Dog>(age));
    for (auto const &a: zoo) {
        a->speak();
    }
    age++;
    for (auto const &a: zoo) {
        a->speak();
    }
    
}