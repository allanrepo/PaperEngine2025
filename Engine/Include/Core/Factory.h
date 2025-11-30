#pragma once

#include <functional>
#include <Core/Singleton.h>

namespace core
{
    template<typename K, typename T, typename H = std::hash<K>, typename... Args>
    class Factory : public Singleton<Factory<K, T, H, Args...>>
    {
        friend class Singleton<Factory<K, T, H, Args...>>;

    public:
        void Register(const K& key, std::function<std::unique_ptr<T>(Args...)> creator)
        {
            creators[key] = std::move(creator);
        }

        void Clear()
        {
            creators.clear();
        }

        std::unique_ptr<T> Create(const K& name, Args&&... args) const
        {
            auto it = creators.find(name);
            if (it != creators.end())
            {
                return (it->second)(std::forward<Args>(args)...);
            }
            return nullptr;
        }

        bool Has(const K& key) const 
        {
            return creators.find(key) != creators.end();
        }

    protected:
        // the map uses templated key because key can be a simple string or int, or it can be a pair or complex structure.
        // the value is a function pointer that will be called to create the object. it's signature is 'unique_ptr<T> <Create>()'
        std::unordered_map<K, std::function<std::unique_ptr<T>(Args...)>, H> creators;
        Factory() = default;
    };

}

