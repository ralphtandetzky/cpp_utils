struct Client1; 
struct Client2;
typedef Visitor<Client1, Client2> BaseClientVisitor;
struct Client1 : BaseClientVisitor::Visitable<Client1> { };
struct Client2 : BaseClientVisitor::Visitable<Client2> { };

struct Client2a;
struct Client2b;
typedef Visitor<Client2a, Client2b> Client2Visitor;
struct Client2a : Client2, Client2Visitor::Visitable<Clien2a> { };
struct Client2b : Client2, Client2Visitor::Visitable<Clien2b> { };

struct AllClientsVisitor 
	: BaseClientVisitor
	, Client2Visitor
{
	virtual void visit( Client2 & t ) final
	{
		t.accept( static_cast<Client2Visitor&>(*this) );
	}
};

struct ConstAllClientsVisitor 
	: BaseClientVisitor::ConstVisitor
	, Client2Visitor::ConstVisitor
{
	virtual void visit( const Client2 & t ) final
	{
		t.accept( static_cast<Client2Visitor::ConstVisitor&>(*this) );
	}
};
