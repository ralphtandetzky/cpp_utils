#include "user_parameter.h"


namespace cu
{

//namespace {
//    class AssignVisitor : public UserParameterVisitor
//    {
//    public:
//        AssignVisitor( const UserParameter & src )
//            : src(src)
//        {
//        }

//    protected:
//        virtual void visit( RealUserParameter & param ) override
//        {
//            doVisit(param);
//        }

//    private:
//        const UserParameter & src;
//    };
//} // unnamed namespace

void assign(
        UserParameter & dest
        , const UserParameter & src )
{
    assign<UserParameterVisitor>( dest, src );
}

} // namespace cu
