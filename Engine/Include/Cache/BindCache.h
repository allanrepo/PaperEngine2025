#pragma once
#include <Core/Singleton.h>
#include <Utilities/Logger.h>
#include <functional>

namespace cache
{
    // description:
    // This class is a global state binding optimizer.
    // It tracks the currently bound object of type T(e.g. texture, shader, input layout, render target, pipeline state) 
    // and ensures that redundant bind calls are skipped unless explicitly forced.
    // 
    // global:
    // it is singleton because binding state is inherently global in graphics APIs. 
    // for example, DirectX device contexts have one “current input layout,” one “current texture,” etc.
    // it provides convenience where you don't have to keep reference of this cache to check if a state is bindable
    //
    // safety:
    // this class has no ownership of the objects it will point to, meaning it will never set/get or call its methods.
    // so even if the object it points to is destroyed, the cache will still hold the stale pointer but won’t dereference 
    // it itself — misuse happens only if external code tries to use it.
    template<typename T>
    class BindCache : public core::Singleton<BindCache<T>>
    {
    private:
        friend class core::Singleton<ContextCache<T>>;

        T* m_state = nullptr;

    public:
        // returns true if new state is different from cached state or force is true
        bool CanBind(T* state, bool force = false) const
        {
            return force || (state != m_state);
        }

        // binds newState if needed, using provided binder
        void Bind(T* state, std::function<void(T*)> binder, bool force = false)
        {
            if (CanBind(state, force))
            {
                binder(state);
                m_state = state;
            }
        }

        void Reset()
        {
            m_state = nullptr;
        }

        void Bind(T* state, bool force = false)
        {
            if (CanBind(state, force))
            {
                m_state = state;
            }
        }

        T* Get()
        {
            return m_state;
        }
    };


}
