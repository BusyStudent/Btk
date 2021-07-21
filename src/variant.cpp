#include "./build.hpp"

#include <Btk/variant.hpp>
#include <type_traits>

namespace Btk{
    //Destroy if destructable
    template<class T>
    void var_destroy(T &v){
        if constexpr(std::is_destructible_v<T>){
            v.~T();
        }
    }
    template<class T>
    void var_swap(T &v1,T &v2){
        //Just swap it
        if constexpr(std::is_trivially_move_assignable_v<T>){
            std::swap(v1,v2);
        }
        else{
            //We use stl swap algo
            v1.swap(v2);
        }
    }
    
    void Variant::clear(){
        switch(type){
            //destroy the data if needed
            #define BTK_VARIANT_FILEDS(X) \
                case Type::_##X:{ \
                    var_destroy(data._##X);\
                    break;\
                }
            //Ex
            BTK_VARIANT_BODY
            #undef BTK_VARIANT_FILEDS
            default:
                break;
        }
        type = Type::Unknown;
    }
    void Variant::swap(Variant &var){
        if(empty()){
            //Self is empty
            //Do move
            #define BTK_VARIANT_FILEDS(X) \
                    case Type::_##X:{ \
                    new (static_cast<void*>(&data)) X(\
                        std::move(var.data._##X)\
                    );\
                    break;\
                }
            switch(var.type){
                BTK_VARIANT_BODY
                default:
                    break;
            }
            //Clear another
            type = var.type;
            var.type = Type::Unknown;
            #undef BTK_VARIANT_FILEDS
        }
        else if(var.empty()){
            //Args is empty
            var.swap(*this);
        }
        //Not both are empty
        else if(var.type == type){
            //Is same type
            //Just use var swap
            #define BTK_VARIANT_FILEDS(X) \
                    case Type::_##X:{ \
                    var_swap(data._##X,var.data._##X);\
                    break;\
                }
            switch(type){
                BTK_VARIANT_BODY
                default:
                    break;
            }
            #undef BTK_VARIANT_FILEDS
        }
        else{
            //Is not same type
            //Create a tmp var
            Variant tmp = std::move(var);
            var = std::move(*this);
            *this = std::move(tmp);
        }
    }
    Variant::Variant(const Variant &var){
        //Do Copy
        #define BTK_VARIANT_FILEDS(X) \
            case Type::_##X:{ \
                emplace(var.data._##X);\
            break;\
        }
        switch(var.type){
            BTK_VARIANT_BODY
            default:
                break;
        }
        #undef BTK_VARIANT_FILEDS
    }
    //Debug functions
    //Helper
    template<class T>
    inline void _variant_print(std::ostream &os,const T &value){
        if constexpr(std::is_same_v<u8string,T> or std::is_same_v<u16string,T>){
            //Add "" if is string
            os.put('"');
            os << value;
            os.put('"');
        }
        else if constexpr(std::is_same_v<StringList,T> or std::is_same_v<StringRefList,T>){
            //Is String List or StringRefList
            os.put('[');

            for(auto iter = value.begin();iter != value.end();++iter){
                os << "\n    ";
                os.put('"');
                os << *iter;
                os.put('"');
                //If not the last one,add ,
                if(iter != (value.end() - 1)){
                    os.put(',');
                }
                else{
                    os.put('\n');
                }
            }
            os.put(']');
        }
        else{
            os << value;
        }
    }
    std::ostream &operator <<(std::ostream &os,const Variant &var){
        //Make output function
        #define BTK_VARIANT_FILEDS(T) \
            case Variant::Type::_##T:{\
                os << "Variant("#T",";\
                _variant_print(os,var.data._##T);\
                os << ')';\
                break;\
            }
        switch(var.type){
            BTK_VARIANT_BODY
            default:
                os << "Variant(Unknown,NULL)";
                break;
        }
        #undef BTK_VARIANT_FILEDS
        return os;
    }

}