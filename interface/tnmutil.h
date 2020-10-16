#ifndef TNMUTIL_H
#define TNMUTIL_H
//-----------------------------------------------------------------------------
// Some TNM utilities
// Created: 15 Oct 2020 HBP
//-----------------------------------------------------------------------------
#include <string>
//-----------------------------------------------------------------------------
std::string tnm_write_code(std::string getter_classname,
			   std::string getter_objectname,
			   std::string method,
			   std::string otype,
			   std::string rtype,
			   int maxcount,
			     int count);
#endif
