#include "formula_parser.h"

#include "../cpp_utils/math_constants.h"
#include "../cpp_utils/std_make_unique.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace cu {
namespace { // unnamed

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;

    struct PlaceHolder {};
    struct ExpressionNode;
    typedef boost::variant<
        boost::recursive_wrapper<ExpressionNode>
        , double, PlaceHolder>
        Expression;
    typedef std::function<double(const std::vector<double> &)> nAryFunc;
    struct ExpressionNode
    {
        std::vector<Expression> subs;
        nAryFunc op;
    };

} // unnamed namespace
} // namespace cu

BOOST_FUSION_ADAPT_STRUCT(
    cu::ExpressionNode,
    (std::vector<cu::Expression>, subs)
    (cu::nAryFunc, op)
    )

namespace cu {
namespace { // unnamed

    template <typename Iterator>
    struct FormulaGrammar
            : qi::grammar<Iterator, Expression(), ascii::space_type>
    {
        FormulaGrammar() : FormulaGrammar::base_type(expression)
        {
            variable.add
                    ("x",PlaceHolder())
                    ;
            constant.add
                    ("e",cu::e)
                    ("pi",cu::pi)
                    ;
            #define ADD(string,func) \
                (string, [](const std::vector<double> &xs) \
                { \
                    assert( xs.size() == 1 ); \
                    return ((double(*)(double))func)( \
                        xs.at(0) ); \
                })
            unFuncSym.add
                    ADD("sqrt", sqrt)
                    ADD("abs",abs)
                    ADD("exp",exp)
                    ADD("ln",log)
                    ADD("cbrt",cbrt)
                    ADD("sin",sin)
                    ADD("cos",cos)
                    ADD("tan",tan)
                    ADD("arcsin",asin)
                    ADD("arccos",acos)
                    ADD("arctan",atan)
                    ADD("sinh",sinh)
                    ADD("cosh",cosh)
                    ADD("tanh",tanh)
                    ADD("arsinh",asinh)
                    ADD("arcosh",acosh)
                    ADD("artanh",atanh)
                    ADD("erf",erf)
                    ADD("gamma",tgamma)
                    ADD("ceil",ceil)
                    ADD("floor",floor)
                    ADD("trunc",trunc)
                    ADD("round",round)
                ;
            #undef ADD
            #define ADD(string,func) \
                (string, [](const std::vector<double> &xs) \
                { \
                    assert( xs.size() == 2 ); \
                    return ((double(*)(double,double))func)( \
                    xs.at(0), xs.at(1) ); \
                })
            binFuncSym.add
                    ADD("min",fmin)
                    ADD("max",fmax)
                    ADD("hypot",hypot)
                ;
            #undef ADD

            expression %= sum;
            sum %= (diff % '+') >> addOp;
            addOp = qi::eps[ qi::_val =
                [](const std::vector<double> &xs)->double
                {
                    return std::accumulate( xs.begin()+1, xs.end(), xs.at(0),
                                            std::plus<double>() );
                }];
            diff %= (neg % '-') >> subOp;
            subOp = qi::eps[ qi::_val =
                [](const std::vector<double> &xs)->double
                {
                    return std::accumulate( xs.begin()+1, xs.end(), xs.at(0),
                                            std::minus<double>() );
                }];
            neg %= ( '-' >> prod >> negOp )
                 | (        prod >> idOp  );
            negOp = qi::eps[ qi::_val =
                [](const std::vector<double> &xs )->double
                { return -xs.at(0); } ];
            prod %= (quot % '*') >> mulOp;
            mulOp = qi::eps[ qi::_val =
                [](const std::vector<double> &xs)->double
                {
                    return std::accumulate( xs.begin()+1, xs.end(), xs.at(0),
                                            std::multiplies<double>() );
                }];
            quot %= (power % '/') >> divOp;
            divOp = qi::eps[ qi::_val =
                [](const std::vector<double> &xs)->double
                {
                    return std::accumulate( xs.begin()+1, xs.end(), xs.at(0),
                                        std::divides<double>() );
                }];
            power %= (atom % '^') >> powOp;
            powOp = qi::eps[ qi::_val =
                [](const std::vector<double> &xs)->double
                {
                    auto index = xs.size()-1;
                    double power = xs[index];
                    while ( index != 0 )
                        power = pow( xs[--index], power );
                    return power;
                }];
            atom %= ( qi::double_ >> idOp )
                  | ( '(' >> expression >> ')' >> idOp )
                  | function
                  | ( variable >> idOp )
                  | ( constant >> idOp );
            idOp = qi::eps[ qi::_val =
                [](const std::vector<double> &xs)->double
                {
                    return xs.at(0);
                }];
            function %= unFunc | binFunc;
            unFunc = unFuncSym[ phoenix::at_c<1>(qi::_val) = qi::_1 ]
                    >> '(' >> expression[
                       phoenix::push_back(phoenix::at_c<0>(qi::_val),qi::_1)]
                    >> ')';
            binFunc = binFuncSym[ phoenix::at_c<1>(qi::_val) = qi::_1 ]
                    >> '(' >> expression[
                       phoenix::push_back(phoenix::at_c<0>(qi::_val),qi::_1)]
                    >> ',' >> expression[
                       phoenix::push_back(phoenix::at_c<0>(qi::_val),qi::_1)]
                    >> ')';
        }

        qi::rule<Iterator, Expression    (), ascii::space_type> expression;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> sum;
        qi::rule<Iterator, nAryFunc      (), ascii::space_type> addOp;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> diff;
        qi::rule<Iterator, nAryFunc      (), ascii::space_type> subOp;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> neg;
        qi::rule<Iterator, nAryFunc      (), ascii::space_type> negOp;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> prod;
        qi::rule<Iterator, nAryFunc      (), ascii::space_type> mulOp;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> quot;
        qi::rule<Iterator, nAryFunc      (), ascii::space_type> divOp;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> power;
        qi::rule<Iterator, nAryFunc      (), ascii::space_type> powOp;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> atom;
        qi::rule<Iterator, nAryFunc      (), ascii::space_type> idOp;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> function;
        qi::symbols<char,PlaceHolder> variable;
        qi::symbols<char,double> constant;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> unFunc;
        qi::symbols<char,nAryFunc> unFuncSym;
        qi::rule<Iterator, ExpressionNode(), ascii::space_type> binFunc;
        qi::symbols<char,nAryFunc> binFuncSym;
    };


    class EvaluateOnceVisitor : public boost::static_visitor<double>
    {
    public:
        explicit EvaluateOnceVisitor( double x ) : x(x) {}

        double operator()( const ExpressionNode & e ) const
        {
            std::vector<double> subResults;
            std::transform( std::begin(e.subs), std::end(e.subs),
                            std::back_inserter( subResults ),
                            boost::apply_visitor(*this) );
            return e.op( subResults );
        }

        double operator()( double d ) const
        {
            return d;
        }

        double operator()( const PlaceHolder & ) const
        {
            return x;
        }

    private:
        double x;
    };


    class EvaluateVisitor : public boost::static_visitor<std::vector<double>>
    {
    public:
        explicit EvaluateVisitor( const std::vector<double> & xs ) : xs(xs) {}

        std::vector<double> operator()( const ExpressionNode & e ) const
        {
            std::vector<std::vector<double>> subResults;
            std::transform( std::begin(e.subs), std::end(e.subs),
                            std::back_inserter( subResults ),
                            boost::apply_visitor(*this) );
            const auto argsize = subResults.size();
            std::vector<double> args( argsize, 0.0 );
            assert( args.size() > 0 );
            const auto xsize = xs.size();
            std::vector<double> result;
            for ( size_t i = 0; i < xsize; ++i )
            {
                for ( size_t j = 0; j < argsize; ++j )
                {
                    args[j] = subResults[j][i];
                }
                result.push_back( e.op(args) );
            }
            return result;
        }

        std::vector<double> operator()( double d ) const
        {
            return std::vector<double>( xs.size(), d );
        }

        std::vector<double> operator()( const PlaceHolder & ) const
        {
            return xs;
        }

    private:
        const std::vector<double> & xs;
    };

} // unnamed namespace


struct ExpressionTree::Impl
{
    Expression expr;
};


ExpressionTree::ExpressionTree()
    : m()
{
}

ExpressionTree::ExpressionTree( const std::string & s )
    : ExpressionTree()
{
    if ( parse(s) != s.size() )
        throw FormulaParseFailure();
}

ExpressionTree::~ExpressionTree()
{
}

size_t ExpressionTree::parse( const std::string & s )
{
    auto tree = std::make_unique<Impl>();
    auto first = s.begin();
    const auto last = s.end();
    if ( qi::phrase_parse( first, last
        , FormulaGrammar<decltype(first)>()
        , ascii::space, tree->expr ) )
    {
        m = std::move(tree);
        return first - s.begin();
    }
    m.reset();
    return 0;
}

double ExpressionTree::evaluate( double x ) const
{
    return boost::apply_visitor( ::cu::EvaluateOnceVisitor(x), m->expr );
}

std::vector<double> ExpressionTree::evaluate(
            const std::vector<double> & xs ) const
{
    return boost::apply_visitor( ::cu::EvaluateVisitor(xs), m->expr );
}


FormulaParseFailure::FormulaParseFailure()
    : std::runtime_error("Failed to parse mathematical expression.")
{
}

} // namespace cu
