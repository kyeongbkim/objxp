#ifndef __JSON_SERIALIZER_H__
#define __JSON_SERIALIZER_H__

#include <boost/property_tree/json_parser.hpp>
#include <Yail.h>

namespace Yail {
namespace JSONSerializer {
  template<class Ptree>
  void serialize(
      std::basic_ostream< typename Ptree::key_type::value_type > &stream,
      const Ptree &pt, bool pretty = false) {
    boost::property_tree::json_parser::write_json(stream, pt, pretty);
  }

  template<class Ptree>
  void deserialize(
      std::basic_istream< typename Ptree::key_type::value_type > &stream,
      Ptree &pt) {
    boost::property_tree::json_parser::read_json(stream, pt);
  }
}
}

#endif

