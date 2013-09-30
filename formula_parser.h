/// @file
///
/// @author Ralph Tandetzky
/// @date 30 Sep 2013

#pragma once

#include <memory>
#include <vector>
#include <stdexcept>

namespace cu
{

class ExpressionTree
{
public:
    ExpressionTree();

    /// @brief Contructs an expression tree from a parsed string.
    ///
    /// Throws a @c FormulaParseFailure, if the formula cannot be parsed
    /// successfully. If an other error occurs, then other kinds of
    /// exceptions may be forwarded.
    ExpressionTree( const std::string & s );
    ~ExpressionTree();

    /// @brief Parses the given string and stores the resulting
    /// expression tree in *this.
    ///
    /// The return value is the number of characters that could be parsed
    /// successfully. Hence, you can check for success with
    ///   @code
    ///     tree.parse( s ) == s.size()
    ///   @endcode
    /// If parsing fails, then no exception will be thrown unless the called
    /// code throws an exception due to another error such as memory allocation
    /// errors. If the string could be parsed partially, then the expression
    /// tree will be reset to the part it could parse. The returned value
    /// can help to identify, where the error occured in the input string.
    size_t parse( const std::string & s );

    /// Evaluates the expression tree for one value of x.
    double evaluate( double x ) const;

    /// Evaluates the expression tree for several values of x.
    std::vector<double> evaluate( const std::vector<double> & xs ) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m;
};


class FormulaParseFailure : public std::runtime_error
{
public:
    FormulaParseFailure();
};

} // namespace cu
