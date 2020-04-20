#ifndef __SERIALIZABLE_H__
#define __SERIALIZABLE_H__

#include <boost/property_tree/json_parser.hpp>
#include <Yail.h>

namespace Yail {

  typedef boost::property_tree::ptree Ptree;
  typedef boost::property_tree::ptree PropertyTree;

  class Serializable {
    public:
      virtual ~Serializable() {}
      virtual void marshal(Ptree& node) = 0;
      virtual void unmarshal(Ptree& node) = 0;
  };

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

  } // end of namespace JSONSerializer

}

#endif

