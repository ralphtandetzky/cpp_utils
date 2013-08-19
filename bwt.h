#pragma once

#include <string>

std::pair<std::string,size_t> burrows_wheeler_transform( std::string s );
std::string burrows_wheeler_transform_inverse( std::string s, size_t index );

