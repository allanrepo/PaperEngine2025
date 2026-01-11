#pragma once

#include <list>
#include <iostream>
#include <string>
#include <functional>


namespace event
{
    // delegate interface
    template <typename R, typename... Args>
    class IDelegate
    {
    public:
        virtual ~IDelegate() {}

        virtual bool IsActive() const = 0;
        virtual void Deactivate() = 0;

        virtual R operator ()(const Args&...) = 0;

        virtual bool Equals(const IDelegate<R, Args...>* other) const = 0;
    };

    // delegate implementation
    // follows function signature of return value R, class type C, and variadic argument Args
    template <typename R, typename C, typename... Args>
    class Delegate : public IDelegate<R, Args...>
    {
    private:
        R(C::* m_pFunc)(Args...);
        C* m_pInst;

        // flag used to check if this delegate should be invoked. if true, it can be invoked
        bool active;

    public:
        Delegate(R(C::* func)(Args...), C* inst):
            m_pFunc(func),
            m_pInst(inst),
            active(true)
        {
        }

        virtual ~Delegate()
        {
        }

        virtual bool IsActive() const
        {
            return active;
        }

        virtual void Deactivate()
        {
            active = false;
        }

        virtual R operator ()(const Args&... a) override
        {
            return (m_pInst->*m_pFunc)(a...);
        }

        // if delegate's calling object and function pointer is same, then they are equal
        bool Equals(const IDelegate<R, Args...>* other) const override
        {
            auto otherDelegate = dynamic_cast<const Delegate<R, C, Args...>*>(other);
            return otherDelegate && otherDelegate->m_pFunc == m_pFunc && otherDelegate->m_pInst == m_pInst;
        }
    };

    // specialized implementation of delegate where there is no class type. 
    // this handles generic functions as delegates
    template <typename R, typename... Args>
    class Delegate<R, void, Args...> : public IDelegate<R, Args...>
    {
    private:
        R(*m_pFunc)(Args...);

        bool active = true;

    public:
        Delegate(R(*func)(Args...)):
            m_pFunc(func)
        {
        }

        virtual ~Delegate()
        {
        }

        virtual bool IsActive() const
        {
            return active;
        }

        virtual void Deactivate()
        {
            active = false;
        }

        virtual R operator ()(const Args&... a) override
        {
            return (*m_pFunc)(a...);
        }

        bool Equals(const IDelegate<R, Args...>* other) const override
        {
            auto otherDelegate = dynamic_cast<const Delegate<R, void, Args...>*>(other);
            return otherDelegate && otherDelegate->m_pFunc == m_pFunc;
        }
    };

    // specialization for lambdas or std::function. 
	// note: don't credit me for this implementation. got this from copilot. getting lazy now :P
    template <typename R, typename... Args>
    class Delegate<R, std::function<R(Args...)>, Args...> : public IDelegate<R, Args...>
    {
    private:
        std::function<R(Args...)> m_func;
        bool active = true;

    public:
        Delegate(std::function<R(Args...)> func) : m_func(std::move(func)) {}

        virtual ~Delegate() {}

        virtual bool IsActive() const override { return active; }
        virtual void Deactivate() override { active = false; }

        virtual R operator()(const Args&... a) override
        {
            return m_func(a...);
        }

        bool Equals(const IDelegate<R, Args...>* other) const override
        {
            // Equality for lambdas is tricky — usually you don’t compare them.
            // For simplicity, always return false (or compare addresses if stored).
            return false;
        }
    };


    // this is the "view" wrapper of delegate class. instead of creating an instance of delegate class, application creates this and pass to Event class
    // the Event class will internally create an instance of delegate class with information contained in this "view" class. 
    // doing this, allows Event class to fully manage the lifetime of delegate objects.
    template<typename R, typename C, typename... Args>
    class Handler
    {
    private:
        R(C::* m_pFunc)(Args...);
        C* m_pInst;

        template <typename... EArgs>
        friend class Event;

    public:
        Handler(C* inst, R(C::* func)(Args...))
        {
            m_pFunc = func;
            m_pInst = inst;
        }
    };

    // also "view" wrapper for delegate class, but for specialized one where it takes generic function as delegate
    template <typename R, typename... Args>
    class Handler<R, void, Args...>
    {
    private:
        R(*m_pFunc)(Args...);

        template <typename... EArgs>
        friend class Event;

    public:
        Handler(R(*func)(Args...))
        {
            m_pFunc = func;
        }
    };

    // Handler wrapper for lambdas
    // note: don't credit me for this implementation. got this from copilot. getting lazy now :P
    template <typename R, typename... Args>
    class Handler<R, std::function<R(Args...)>, Args...>
    {
    private:
        std::function<R(Args...)> m_func;

        template <typename... EArgs>
        friend class Event;

    public:
        Handler(std::function<R(Args...)> func) : m_func(std::move(func)) {}
    };

    /*
    * the deduction guide is a fix suggested by chatgpt on an issue where if i create an instance of event::Handler with a regular function, i must explicitly specify the template parameters
    * root cause: 
    * The compiler must deduce template parameters for Handler<R, void, Args...> from the constructor argument.
    * BUT — class template argument deduction (CTAD) does not apply to partial specializations.
    * C++ only supports deduction guides for the primary template — not its partial specializations.
    * NOTE: C++ 17 is required
    */
    // deduction guide for free function handlers
    template<typename R, typename... Args>
    Handler(R(*)(Args...)) -> Handler<R, void, Args...>;

    // deduction guide for member function handlers
    template<typename R, typename C, typename... Args>
    Handler(C*, R(C::*)(Args...)) -> Handler<R, C, Args...>;

    // Deduction guide for lambdas
    template <typename R, typename... Args>
    Handler(std::function<R(Args...)>) -> Handler<R, std::function<R(Args...)>, Args...>;

	// this deduction guide is for lambdas without std::function wrapper. what an amazing trick copilot came up with :P
	// but this is limited to only void return type lambdas. i am commenting for now because i don't really want to support this yet...
    // well i modified it to be generic and not just void() signature. does it work?
    // TODO: test this update by me
    template <typename F, typename R, typename... Args>
    Handler(F f) -> Handler<void, std::function<R(Args...)>>;

    // this is the event class
    template<typename... Args>
    class Event
    {
    private:
        std::list<IDelegate<void, Args...>*> m_subscribers;
        std::list<typename std::list<IDelegate<void, Args...>*>::iterator> m_unsubscribers;
    public:
        Event() = default;

        virtual ~Event()
        {
            Clear();
        }

        void Clear()
        {
            // we store pointers. explicitly destroy the objects the pointers points to first
            for (IDelegate<void, Args...>* subscriber : m_subscribers)
            {
                if (subscriber)
                {
                    delete subscriber;
                }
            }

            // then we clear the list. safe now as the objects all the pointers in list points to are now destroyed
            m_subscribers.clear();
        }

        size_t Size() const
        {
            return m_subscribers.size();
        }

        void operator ()(const Args&... args)
        {
            // notify all listeners
            for (IDelegate<void, Args...>* subscriber : m_subscribers)
            {
                if (subscriber && subscriber->IsActive())
                {
                    (*subscriber)(args...);
                }
            }

            // Sweep deferred removals
            for (auto it : m_unsubscribers)
            {
                delete* it;                 // Destroy the delegate
                m_subscribers.erase(it);    // Remove from listener list
            }
            m_unsubscribers.clear();
        }

        template <typename C>
        void operator += (Handler<void, C, Args...> handler)
        {
            IDelegate<void, Args...>* dlgt = new Delegate<void, C, Args...>(handler.m_pFunc, handler.m_pInst);

            m_subscribers.push_back(dlgt); 
        }

        void operator += (Handler<void, void, Args...> handler)
        {
            IDelegate<void, Args...>* dlgt = new Delegate<void, void, Args...>(handler.m_pFunc);

            m_subscribers.push_back(dlgt);
        }

        // Add operator += for lambda handlers
        // note: don't credit me for this implementation. got this from copilot. getting lazy now :P
        // note: function signature is limited to void return type for now
        void operator += (Handler<void, std::function<void(Args...)>, Args...> handler)
        {
            IDelegate<void, Args...>* dlgt = new Delegate<void, std::function<void(Args...)>, Args...>(handler.m_func);

            m_subscribers.push_back(dlgt);
        }

        template <typename C>
        void operator -= (Handler<void, C, Args...> handler)
        {
            Delegate<void, C, Args...> temp(handler.m_pFunc, handler.m_pInst);

            for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
            {
                if ((*it)->Equals(&temp))
                {
                    (*it)->Deactivate();            // Prevent dispatch
                    m_unsubscribers.push_back(it);   // Store iterator for deferred removal
                    break;
                }
            }
        }

        void operator -= (Handler<void, void, Args...> handler)
        {
            Delegate<void, void, Args...> temp(handler.m_pFunc);

            for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
            {
                if ((*it)->Equals(&temp))
                {
                    (*it)->Deactivate();            // Prevent dispatch
                    m_unsubscribers.push_back(it);   // Store iterator for deferred removal
                    break;
                }
            }
        }

        // Add operator -= for lambda handlers
        // note: don't credit me for this implementation. got this from copilot. getting lazy now :P
		// note: function signature is limited to void return type for now
        void operator -= (Handler<void, std::function<void(Args...)>, Args...> handler)
        {
            Delegate<void, std::function<void(Args...)>, Args...> temp(handler.m_func);

            for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
            {
                if ((*it)->Equals(&temp))
                {
                    (*it)->Deactivate();
                    m_unsubscribers.push_back(it);
                    break;
                }
            }
        }

    };



    namespace Test
    {
        class TestClass
        {
        public:
            void MethodNoReturnNoArgs()
            {
                std::cout << "MethodNoReturnNoArgs" << std::endl;
            }

            void MethodNoReturnOneArgConstInt(const int i)
            {
                std::cout << "MethodNoReturnOneArgConstInt >> int=" << i << std::endl;
            }

            void MethodNoReturnOneArgInt(int i)
            {
                std::cout << "MethodNoReturnOneArgInt >> int=" << i << std::endl;
            }

            void MethodNoReturnTwoArgsStringDouble(std::string str, double d)
            {
                std::cout << "MethodNoReturnTwoArgsStringDouble >> string=" << str << ", double = " << d << std::endl;
            }

            void MethodNoReturnTwoArgsConstStringConstDouble(const std::string str, const double d)
            {
                std::cout << "MethodNoReturnTwoArgsConstStringConstDouble >> string=" << str << ", double = " << d << std::endl;
            }

            const double MethodRetDoubleTwoArgIntAndDouble(const int i, const double d)
            {
                return i + d;
            }
        };

        inline void FuncNoReturnNoArgs()
        {
            std::cout << "FuncNoReturnNoArgs" << std::endl;
        }

        inline void FuncNoReturnOneArgInt(int i)
        {
            std::cout << "FuncNoReturnOneArgInt >> int=" << i << std::endl;
        }

        inline void FuncNoReturnOneArgConstInt(const int i)
        {
            std::cout << "FuncNoReturnOneArgConstInt >> int=" << i << std::endl;
        }

        inline void FuncNoReturnTwoArgsStringDouble(std::string str, double d)
        {
            std::cout << "FuncNoReturnTwoArgsStringFloat >> string=" << str << ", double = " << d << std::endl;
        }

        inline void FuncNoReturnTwoArgsConstStringConstDouble(const std::string str, const double d)
        {
            std::cout << "FuncNoReturnTwoArgsConstStringConstDouble >> string=" << str << ", double = " << d << std::endl;
        }

        inline const int FuncRetIntOneArgInt(const int i)
        {
            return i;
        }

        inline void TestDelegate()
        {
            {
                int i = 23;
                Delegate<const int, void, int> dlgtFuncRetIntArgInt(&FuncRetIntOneArgInt);
                int rslt = dlgtFuncRetIntArgInt(i);
                std::cout << "Created and invoked delegate with regular function with argument int=" << i << " and return value of " << rslt << std::endl;
                if (rslt != i)
                {
                    throw std::runtime_error("Invalid value: " + std::to_string(rslt) + ". Must be " + std::to_string(i));
                }
            }
            {
                TestClass tc;
                int i = 3;
                double d = 1.4;
                Delegate<const double, TestClass, const int, const double> dgltMethodRetDoubleArgIntDouble(&TestClass::MethodRetDoubleTwoArgIntAndDouble, &tc);

                double rslt = dgltMethodRetDoubleArgIntDouble(i, d);
                std::cout << "Created and invoked delegate with class method function with argument int=" << i << " and double=" << d << " and returns the sum of both arguments as double= " << rslt << std::endl;
                if (rslt != d + i)
                {
                    throw std::runtime_error("Invalid value: " + std::to_string(rslt) + ". Must be " + std::to_string(i + d));
                }
            }

        }

        inline void TestEvent()
        {
            // test methods
            std::cout << "Testing Event class methods..." << std::endl;
            {
                TestClass tc;
                Event<int> evtOneArgInt;

                evtOneArgInt += event::Handler(&FuncNoReturnOneArgInt);
                evtOneArgInt += event::Handler(&FuncNoReturnOneArgConstInt);
                evtOneArgInt += event::Handler(&tc, &TestClass::MethodNoReturnOneArgInt);
                evtOneArgInt += event::Handler(&tc, &TestClass::MethodNoReturnOneArgConstInt);
                std::cout << "Added 4 listeners to event. Number of listers in event : " << evtOneArgInt.Size() << std::endl;

                evtOneArgInt(4);
                std::cout << "event has been fired. check how many lines were printed. are there " << evtOneArgInt.Size() << "?" << std::endl;

                evtOneArgInt.Clear();
                std::cout << "event has removed all its listeners. Number of listers in event : " << evtOneArgInt.Size() << ". Is it 0?" << std::endl;
            }
            {
                TestClass tc;
                Event evtNoArgs;
                evtNoArgs += event::Handler(&FuncNoReturnNoArgs);
                evtNoArgs += event::Handler(&tc, &TestClass::MethodNoReturnNoArgs);
                std::cout << "Created another event Added 2 listeners. Number of listers in event : " << evtNoArgs.Size() << ". Is this correct?" << std::endl;

                evtNoArgs();
                std::cout << "event has been fired. check how many lines were printed. are there " << evtNoArgs.Size() << "?" << std::endl;

                evtNoArgs += event::Handler(&FuncNoReturnNoArgs);
                evtNoArgs -= event::Handler(&tc, &TestClass::MethodNoReturnNoArgs);
                std::cout << "Remove 2 listeners from event. Number of listers in event : " << evtNoArgs.Size() << ". Are there 0 listeners?" << std::endl;
            }
            {
                TestClass tc;
                Event<std::string, double> evtTwoArgsStringDouble;

                evtTwoArgsStringDouble += event::Handler(&FuncNoReturnTwoArgsStringDouble);
                evtTwoArgsStringDouble += event::Handler(&FuncNoReturnTwoArgsConstStringConstDouble);
                evtTwoArgsStringDouble += event::Handler(&tc, &TestClass::MethodNoReturnTwoArgsStringDouble);
                evtTwoArgsStringDouble += event::Handler(&tc, &TestClass::MethodNoReturnTwoArgsConstStringConstDouble);
                std::cout << "Added 4 listeners to event. Number of listers in event : " << evtTwoArgsStringDouble.Size() << std::endl;

                evtTwoArgsStringDouble("Hello", 1.7);
                std::cout << "event has been fired. check how many lines were printed. are there " << evtTwoArgsStringDouble.Size() << "?" << std::endl;

                evtTwoArgsStringDouble -= event::Handler(&FuncNoReturnTwoArgsConstStringConstDouble);
                std::cout << "Remove 1 listener from event. Number of listers in event : " << evtTwoArgsStringDouble.Size() << ". Are there 3 listeners?" << std::endl;

                evtTwoArgsStringDouble.Clear();
                std::cout << "event has removed all its listeners. Number of listers in event : " << evtTwoArgsStringDouble.Size() << ". Is it 0?" << std::endl;
            }
        }

        inline void Go()
        {
            //bool bPrevLoggerState = Logger::Enable;
            //Logger::Enable = bLog;

            TestDelegate();
            TestEvent();

            //Logger::Enable = bPrevLoggerState;
            return;
        }
    }
};
