#include "bwt.h"
#include <vector>
#include <string>
#include <algorithm>

namespace cu {

namespace // unnamed
{

	struct positions_comp
	{
		positions_comp( const size_t & size, const std::string & s )
		: size(size), s(s) {}
	
		bool operator()( size_t lhs, size_t rhs )
		{
			if ( lhs == rhs )
				return false;
			const size_t orig_lhs = lhs;
			while ( s[lhs] == s[rhs] )
			{
				lhs = (lhs+1)%size;
				rhs = (rhs+1)%size;
				if ( lhs == orig_lhs )
					break;
			}
			return s[lhs] < s[rhs];
		}
	
		const size_t & size;
		const std::string & s;
	};

} // unnamed namespace

std::pair<std::string,size_t> burrows_wheeler_transform( std::string s )
{
	const size_t size = s.size();
	std::vector<size_t> positions(size);
	for ( size_t i = 0; i < size; ++i )
		positions[i] = i;
	
	sort( positions.begin(), positions.end(), positions_comp(size,s) );
	*find( positions.begin(), positions.end(), 0 ) = size;
	
	std::string r = s;
	for ( size_t i = 0; i < size; ++i )
		r[i] = s[positions[i]-1];
		
	return std::make_pair(r,size-positions[0]);
}


namespace // unnamed
{

	struct chain_comp
	{
		bool operator()( 
			const std::pair<char,size_t> & lhs, 
			const std::pair<char,size_t> & rhs )
		{
			return lhs.first < rhs.first;
		}
	};

} // unnamed namespace


std::string burrows_wheeler_transform_inverse( std::string s, size_t index )
{
	if ( s.empty() )
		return s;

	const size_t size = s.size(); 
	std::vector<std::pair<char,size_t> > chain;
	for ( size_t i = 0; i < size; ++i ) 
		chain.push_back( std::make_pair(s[i], i) );
	stable_sort( chain.begin(), chain.end(), chain_comp() );

	std::string::iterator it = s.begin();
	size_t i = 0;
	do
	{
		*it++ = chain[i].first ;
		i = chain[i].second;
	}
	while ( i != 0 );

	rotate( s.begin(), s.begin()+index, s.end() );
		
	return s;
}

} // namespace cu
