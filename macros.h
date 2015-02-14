/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#define CU_CONCATENATE_TOKENS( x, y ) CU_CONCATENATE_TOKENS2( x, y )
#define CU_CONCATENATE_TOKENS2( x, y ) x ## y
#define CU_UNIQUE_IDENTIFIER CU_CONCATENATE_TOKENS( cuUniqueIdentifier, __COUNTER__ )
