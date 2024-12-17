#ifndef SINGLETON_H
#define SINGLETON_H
#include <Arduino.h>

// Base Singleton class template
template <typename T>
class Singleton
{
public:
    // Deleted copy constructor and assignment operator
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

    // Provides access to the singleton instance
    static T &getInstance()
    {
        static T instance; // Guaranteed to be created only once
        return instance;
    }

protected:
    // Protected constructor and destructor to prevent direct instantiation
    Singleton() {}
    ~Singleton() {}
};

#endif // SINGLETON_H