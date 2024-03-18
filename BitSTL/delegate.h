/*
 * 委托 
 */
#ifndef DELEGATE_H
#define DELEGATE_H

namespace bitstl
{
    template<typename RetVal, typename Param1, typename Param2>
    struct delegate_twoparam
    {
    private:
        void* p_; // 类成员函数对应的类指针
        using F = RetVal(*)(void*, Param1, Param2);
        F f_; // 函数指针

        template<typename ClassName, RetVal(ClassName::*ClassFunc)(Param1, Param2)>
        static RetVal class_func_stub(void* p, Param1 pa1, Param2 pa2)
        {
            ClassName* cp = (ClassName*)p;
            return (cp->*ClassFunc)(pa1, pa2);
        }

        template<RetVal(*Func)(Param1, Param2)>
        static RetVal func_stub(void* p, Param1 pa1, Param2 pa2)
        {
            return (Func)(pa1, pa2);
        }

        static delegate_twoparam create(void *p, F f)
        {
            delegate_twoparam tmp;
            tmp.p_ = p;
            tmp.f_ = f;
            return tmp;
        }

    public:
        delegate_twoparam()
        {
            p_ = nullptr;
            f_ = nullptr;
        }

        ~delegate_twoparam() {}

        template<typename ClassName, RetVal(ClassName::*ClassFunc)(Param1, Param2)>
        static delegate_twoparam from_class_func(ClassName* cp)
        {
            return create(cp, &class_func_stub<ClassName, ClassFunc>);
        }

        template<RetVal(*Func)(Param1, Param2)>
        static delegate_twoparam from_func()
        {
            return create(nullptr, &func_stub<Func>);
        }

        template<typename ClassName, RetVal(ClassName::* ClassFunc)(Param1, Param2)>
        void bind_class_func(ClassName* cp)
        {
            p_ = cp;
            f_ = &class_func_stub<ClassName, ClassFunc>;
        }

        template<RetVal(*Func)(Param1, Param2)>
        void bind_func()
        {
            p_ = nullptr;
            f_ = &func_stub<Func>;
        }

        void unbind()
        {
            p_ = nullptr;
            f_ = nullptr;
        }

        bool isbound()
        {
            return f_ != nullptr;
        }

        RetVal operator()(Param1 pa1, Param2 pa2)
        {
            return (*f_)(p_, pa1, pa2);
        }

        RetVal execute(Param1 pa1, Param2 pa2)
        {
            return (*this)(pa1, pa2);
        }

        // 返回值为void且已绑定时才执行
        bool execute_ifbound(Param1 pa1, Param2 pa2)
        {
            if (is_void_v<RetVal> && isbound())
            {
                (*this)(pa1, pa2);
                return true;
            }
            else
            {
                return false;
            }
        }
    };
}

#endif // !DELEGATE_H

