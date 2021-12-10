//-----------------------------------------------------------------------------
// Some TNM utilities
// Created: 15 Oct 2020 HBP
//-----------------------------------------------------------------------------
#include <boost/regex.hpp>
#include <boost/python/type_id.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <stdlib.h>
//-----------------------------------------------------------------------------
std::string tnm_write_code(std::string getter_classname,
			   std::string getter_objectname,
			   std::string method,
			   std::string otype,
			   std::string rtype,
			   int maxcount,
			   int count)
{
  // check if object is a simple type
  boost::regex getsimpletype("^(float|double|int|long|unsigned"
			     "|size_t|short|bool|char|string|std::string)");
  boost::smatch matchtype;
  std::string otype_lower = otype;
  boost::algorithm::to_lower(otype_lower);
  bool simpletype = boost::regex_search(otype_lower, 
					matchtype, 
					getsimpletype);
  
  // check if object is a vector type or a singleton
  // TODO: make this more robust
  bool vectortype= maxcount > 1;
  
  char record[10000];
  std::string methodstr;
  if ( vectortype )
    {
      // we have a vector: either of objects or simple types
      if ( simpletype )
	methodstr = ""; // there is no spoon!
      else
	methodstr = std::string(".") + method;
      
      sprintf(record,
	      "struct %s\n"
	      "{\n"
	      "  void get(const void* oaddr, const void* vaddr)\n"
	      "  {\n"
	      "    const std::vector<%s>* o = "
	      "(const std::vector<%s>*)oaddr;\n"
	      "    std::vector<%s>* v = (std::vector<%s>*)vaddr;\n"
	      "    v->clear();\n"
	      "    try\n"
	      "      {\n"
	      "        size_t n = min(o->size(), (size_t)%d);\n"
	      "        for(size_t c = 0;  c < n; c++)\n"
	      "          v->emplace_back( o->at(c)%s );\n"
	      "      }\n"
	      "    catch (...)\n"
	      "      {\n"
              "        edm::LogWarning(\"FAILEDCALL\")\n" 
              "          << \"%s %s\" << std::endl;\n"
	      "      }\n"
	      "  }\n"
	      "};\n"
	      "%s %s;\n"
	      "long unsigned int* addr%d = (long unsigned int*)%s;\n"
	      "*addr%d = (long unsigned int)&%s;\n",
	      getter_classname.c_str(),
	      otype.c_str(), otype.c_str(),
	      rtype.c_str(), rtype.c_str(),
	      maxcount,
	      methodstr.c_str(), getter_classname.c_str(), method.c_str(),
	      getter_classname.c_str(), getter_objectname.c_str(),
	      count, "0x%lx",
	      count, getter_objectname.c_str());
    }
  else
    {
      // we have a singleton: either an object or a simple type
      if ( simpletype )
	methodstr = "*o"; // there is no spoon!
      else
	methodstr = std::string("o->") + method;
      
      sprintf(record,
	      "struct %s\n"
	      "{\n"
	      "  void get(const void* oaddr, const void* vaddr)\n"
	      "  {\n"
	      "    const %s* o = (const %s*)oaddr;\n"
	      "    std::vector<%s>* v = (std::vector<%s>*)vaddr;\n"
	      "    v->clear();\n"
	      "    try\n"
	      "      {\n"
	      "        v->emplace_back( %s );\n"
	      "      }\n"
	      "    catch (...)\n"
	      "      {\n"
              "        edm::LogWarning(\"FAILEDCALL\")\n" 
              "          << \"%s %s\" << std::endl;\n"
	      "      }\n"
	      "  }\n"
	      "};\n"
	      "%s %s;\n"
	      "long unsigned int* addr%d = (long unsigned int*)%s;\n"
	      "*addr%d = (long unsigned int)&%s;\n",
	      getter_classname.c_str(),
	      otype.c_str(), otype.c_str(),
	      rtype.c_str(), rtype.c_str(),
	      methodstr.c_str(), getter_classname.c_str(), method.c_str(),
	      getter_classname.c_str(), getter_objectname.c_str(),
	      count, "0x%lx",
	      count, getter_objectname.c_str());      
    }
  return std::string(record);
}
