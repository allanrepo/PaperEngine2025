#pragma once
#include <string>
#include <unordered_map>
#include <iostream>

namespace engine::container
{
    // design consideration:
    // this class mimics the Dictionary from C#.
    // copyable and movable to match STL container design
    template<typename Key = std::string, typename Value = std::string>
    class Dictionary
    {
    public:
		Dictionary() = default;
		~Dictionary() = default;

        // allow copy
        Dictionary(const Dictionary&) = default;
        Dictionary& operator=(const Dictionary&) = default;

        // allow move
        Dictionary(Dictionary&&) noexcept = default;
        Dictionary& operator=(Dictionary&&) noexcept = default;

        using iterator = typename std::unordered_map<Key, Value>::iterator;
        using const_iterator = typename std::unordered_map<Key, Value>::const_iterator;

        // Iterator access
        iterator begin() { return registry.begin(); }
        iterator end() { return registry.end(); }
        const_iterator begin() const { return registry.begin(); }
        const_iterator end() const { return registry.end(); }
        const_iterator cbegin() const { return registry.cbegin(); }
        const_iterator cend() const { return registry.cend(); }

        // Registers a value with a key. Returns true if inserted, false if key already exists.
        bool Register(const Key& key, Value&& value) 
        {
            return registry.emplace(key, std::move(value)).second;
        }

        // forces value to be stored at given key, replacing any existing value if key already exists
        bool Set(const Key& key, const Value& value)
        {
            return registry.emplace(key, value).second;
        }

        // forces value to be stored at given key, replacing any existing value if key already exists
        void Set(const Key& key, Value&& value)
        {
            registry.insert_or_assign(key, std::move(value));
        }

        void Overwrite(const Key& key, const Value& value)
        {
            registry.insert_or_assign(key, value);
        }

        // Retrieves a pointer to the value, or nullptr if not found.
        Value& Get(const Key& key) 
        {
            auto it = registry.find(key);
            if (it == registry.end())
            {
                throw std::out_of_range("Key not found in Dictionary");
            }
            return it->second;
        }

        // Const version
        const Value& Get(const Key& key) const 
        {
            auto it = registry.find(key);
            if (it == registry.end())
            {
                throw std::out_of_range("Key not found in Dictionary");
            }
            return it->second;
        }

        bool TryGetValue(const Key& key, Value& outValue) const
        {
            auto it = registry.find(key);
            if (it != registry.end())
            {
                outValue = it->second;
                return true;
            }
            return false;
        }

        bool Has(const Key& key) const
        {
            return registry.find(key) != registry.end();
        }

        // Removes a value by key. Returns true if removed.
        bool Unregister(const Key& key) 
        {
            return registry.erase(key) > 0;
        }

        void Clear()
        {
            registry.clear();
        }

        // Provides mutable access to a value by key, inserting a default if not found.
        Value& operator[](const Key& key)
        {
            return registry[key]; // inserts default if key doesn't exist
        }

        // Provides read-only access. Throws if key not found.
        const Value& operator[](const Key& key) const
        {
            return Get(key);
        }

        const size_t Size() const
        {
            return registry.size();
        }

    private:
        std::unordered_map<Key, Value> registry;
    };

    static bool TestDictionary()
    {
        Dictionary<> dict;
        std::string value;

        std::cout << "Try to get value of key 'hello' but dictionary is still empty" << std::endl;
        if (dict.TryGetValue("hello", value))
        {
            std::cout << "ERROR: There is a value from key 'hello." << std::endl;
            return false;
        }

        std::cout << "Registering key 'hello' with value 'world'" << std::endl;
        dict.Register("hello", "world");

        std::cout << "Now try again to get value of key 'hello'" << std::endl;
        if (!dict.TryGetValue("hello", value))
        {
            std::cout << "ERROR: did not find value from key 'hello." << std::endl;
            return false;
        }
        std::cout << "value from key 'hello' : " << value << ", is it correct?" << std::endl;

        std::cout << "Using operator [] to add key 'cat' and value 'dog'" << std::endl;
        dict["cat"] = "dog";

        try
        {
            std::cout << "value of key 'cat' read: " << dict["cat"] << ". is value correct?" << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "ERROR: Exception occurred reading value of key 'cat'. " << std::endl;
            std::cout << e.what() << std::endl;
            return false;
        }

        const Dictionary<>& cdict = dict;

        std::cout << "Our dictionary is now a constant and we are going to use operator [] to read a key 'car'. our dictionary does not have this yet.'" << std::endl;
        bool exceptionHappened = false;
        try
        {
            std::cout << cdict["car"] << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception occurred reading value of key 'car'." << "This is expected " << std::endl;
            std::cout << e.what() << std::endl;
            exceptionHappened = true;
        }
        if (!exceptionHappened)
        {
            std::cout << "ERROR: Exception did not occur reading value of key 'car'." << std::endl;
            return false;
        }

        std::cout << "TEST SUCCESSFUL!" << std::endl;
        return true;
    }
}
