#ifndef MAP_WRAPPER_H
#define MAP_WRAPPER_H

#include <map>
#include <string>
#include <stdexcept>

class MapWrapper
{
public:
    // Add a key-value pair to the map
    void add(const std::string &key, double value)
    {
        data_[key] = value;
    }

    // Get the value associated with a key
    double get(const std::string &key) const
    {
        auto it = data_.find(key);
        if (it != data_.end())
        {
            return it->second;
        }
        throw std::runtime_error("Key not found");
    }

    // Check if the map contains a key
    bool contains(const std::string &key) const
    {
        return data_.find(key) != data_.end();
    }

    // Remove a key-value pair from the map
    void remove(const std::string &key)
    {
        data_.erase(key);
    }

    // Get the size of the map
    size_t size() const
    {
        return data_.size();
    }

    // Clear the map
    void clear()
    {
        data_.clear();
    }

    // Get the underlying map (const version)
    const std::map<std::string, double> &getMap() const
    {
        return data_;
    }

private:
    std::map<std::string, double> data_;
};

#endif // MAP_WRAPPER_H