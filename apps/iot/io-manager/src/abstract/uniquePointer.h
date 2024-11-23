// Minimal unique_ptr implementation
template <typename T>
class UniquePtr
{
private:
    T *ptr;

public:
    // Constructor
    explicit UniquePtr(T *p = nullptr) : ptr(p) {}

    // Destructor
    ~UniquePtr()
    {
        delete ptr;
    }

    // Delete copy constructor and assignment operator to prevent copying
    UniquePtr(const UniquePtr &) = delete;
    UniquePtr &operator=(const UniquePtr &) = delete;

    // Allow move semantics
    UniquePtr(UniquePtr &&other) noexcept : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }

    UniquePtr &operator=(UniquePtr &&other) noexcept
    {
        if (this != &other)
        {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // Access underlying pointer
    T *get() const
    {
        return ptr;
    }

    // Dereference operator
    T &operator*() const
    {
        return *ptr;
    }

    // Arrow operator
    T *operator->() const
    {
        return ptr;
    }

    // Release ownership
    T *release()
    {
        T *temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Reset pointer
    void reset(T *p = nullptr)
    {
        delete ptr;
        ptr = p;
    }
};
