#include "visitor.h"
#include <iostream>
#include <memory>
#include <vector>

struct Client1;
struct Client2;
struct Client3;
typedef Visitor<Client1, Client2, Client3> ClientVisitor;
typedef ClientVisitor::ConstVisitor ConstClientVisitor;
typedef ClientVisitor::VisitableInterface Client;
struct Client1 : ClientVisitor::Visitable<Client1> { };
struct Client2 : ClientVisitor::Visitable<Client2> { };
struct Client3 : ClientVisitor::Visitable<Client3> { };

struct PrintTypeVisitor : ConstClientVisitor
{
    virtual void visit( const Client1 & t ) { std::cout << "Client1" << std::endl; }
    virtual void visit( const Client2 & t ) { std::cout << "Client2" << std::endl; }
    virtual void visit( const Client3 & t ) { std::cout << "Client3" << std::endl; }
};

void printType( const Client & c )
{
    PrintTypeVisitor v;
    c.accept(v);
}

int main()
{
    std::vector<std::unique_ptr<Client>> v;
    v.emplace_back ( new Client1  );
    v.emplace_back ( new Client2 );
    v.emplace_back ( new Client3 );
    for ( const auto & x : v)
        printType( *x );
}
