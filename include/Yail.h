#ifndef __YAIL_H__
#define __YAIL_H__

#include <iostream>
#include <list>
#include <fstream>
#include <syslog.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/unordered_set.hpp>

typedef std::string String;
typedef std::ostream Ostream;
typedef std::istream Istream;
typedef std::stringstream Stringstream;
typedef std::ostringstream Ostringstream;
typedef std::istringstream Istringstream;

#define MkString(args) ({ Ostringstream oss; oss << args; oss.str(); })
#define MkCString(args) MkString(args).c_str()

String __getClassName(String prettyFunction);

namespace Yail {
#define Map    std::map
#define Vector std::vector
#define List   std::list
#define Queue  std::queue
#define Pair   std::pair
#define UnorderedSet boost::unordered_set

#define SPtr boost::shared_ptr
#define WPtr boost::weak_ptr
#define DynamicPointerCast boost::dynamic_pointer_cast
#define StaticPointerCast  boost::static_pointer_cast

/* Do not define USE_WPTR using "do {} while(0)" like:
  {                                               \
    SPtr<sPtrType> sPtr = (wPtr).lock();          \
    do { doItBefore } while(0);                   \
    if(sPtr) do { doItIfSPtrIsNotNull } while(0); \
    else do { doItIfSPtrIsNull } while(0);        \
    do { doItAfter } while(0);                    \
  }

  In case
  for( ; ; ) {
    USE_WPTR(MyType, mySPtr, myWPtr, { if(...) break; }, {}, {});
  }
  User expects to escape from the "for" loop,
  but the actual behavior is escaping from the internal "do{}while(0)" loop,
  so the "for" loop will continue.
*/

#define USE_WPTR(sPtrType, sPtr, wPtr, \
                 doItBefore, doItIfSPtrIsNotNull, doItIfSPtrIsNull, doItAfter) \
  { \
    SPtr<sPtrType> sPtr = (wPtr).lock(); \
    { doItBefore } \
    if(sPtr) { doItIfSPtrIsNotNull } \
    else { doItIfSPtrIsNull } \
    { doItAfter } \
  }

#if __cplusplus >= 201103L
// for c++11 or later
#define override override
#define nullPtr(type) nullptr
#else
#define override
#define nullPtr(T) SPtr<T>()
#endif

class YObject {
  public:
    YObject() {}
    virtual ~YObject() {}
    void init() {}
    virtual String className() { return className_; }

  protected:
    WPtr<YObject> wPtr_;
    template <typename T> SPtr<T> getThisPtr() {
      return DynamicPointerCast<T>(wPtr_.lock());
    }

  private:
    String className_;
};

#define USE_BOOST_MAKE_SHARED 0

template <typename T, typename... Args>
inline SPtr<T> CreateObject(Args&&... args) {
#if USE_BOOST_MAKE_SHARED
  SPtr<T> shPtr = boost::make_shared<T>();
#else
  SPtr<T> shPtr(new T());
#endif
  shPtr->setThisPtr(shPtr);
  shPtr->init(std::forward<Args>(args)...);
  return shPtr;
}

#define EXTENDS(typeName) public typeName
#define IMPLEMENTS(typeName) public typeName

#define YAIL_CLASS(typeName) class typeName
#define YAIL_INTERFACE(typeName) struct typeName

#define YAIL_BEGIN_INTERFACE(typeName) \
  struct typeName { \
    virtual ~typeName() {}
#define YAIL_END_INTERFACE };

#define YAIL_BEGIN_CLASS_COMMON(typeName, args...) \
    public: \
      typeName() { \
        className_ = __getClassName(__PRETTY_FUNCTION__); \
        this->__new__ ## typeName (); \
        if(gObjStatsEnabled) UpdateObjStats(className_, 1); \
      } \
      ~typeName() { \
        this->__del__ ## typeName (); \
        if(gObjStatsEnabled) UpdateObjStats(className_, -1); \
      } \
      static String __staticClassName() { \
        return __getClassName(__PRETTY_FUNCTION__); \
      } \
      String className() override { return className_; } \
      void setThisPtr(SPtr<typeName> shPtr) { wPtr_ = shPtr; } \
      int useCount() { return getThisPtr<typeName>().use_count(); } \
      typedef typeName SelfType; \
    private:

#define YAIL_BEGIN_CLASS(typeName, args...) \
  class typeName : args { \
    private: \
      void __new__ ## typeName() {} \
      void __del__ ## typeName() {} \
      String className_; \
    YAIL_BEGIN_CLASS_COMMON(typeName, args)

#define YAIL_BEGIN_CLASS_EXT(typeName, args...) \
  class typeName : args { \
    private: String className_; \
    YAIL_BEGIN_CLASS_COMMON(typeName, args)

#define YAIL_END_CLASS };

}

extern int gTraceLevel;
extern void SetTraceLevel(int n);

extern bool gObjStatsEnabled;
extern void EnableObjStats(bool val);
extern void UpdateObjStats(String typeName, int inc);
extern String GetObjStats(bool verbose=false);
extern std::shared_ptr<std::ofstream> logOutFile;

enum log_dest_type {
  LOG_DEST_COUT_DEFAULT,
  LOG_DEST_COUT_FILE,
  LOG_DEST_SYSLOG
};
extern log_dest_type LOG_DEST;
extern bool LOG_TRANSACTION;
extern void setLogTransaction(bool b);
extern void setLogDestination(log_dest_type dest, String destFile="");
extern void __TRACE(int level, String msg);
#if 0
#if !defined(TRACE_SYSLOG)
#define __TRACE(level, msg) do{ std::cout << msg; } while(0)
#else
#define __TRACE(level, msg) do{ \
  if (level >= LOG_DEBUG) \
    syslog(LOG_DEBUG, "%s", msg); \
  else \
    syslog(level, "%s", msg); \
} while(0)
#endif
#endif

#define SET_LOG(level, dest, outFile) \
  do { \
    SetTraceLevel(level); \
    setLogDestination(dest, outFile); \
  } while (0)

#define _TRACE(level, args...) \
  do { \
    if(level <= gTraceLevel) { \
      struct tm *sTm; \
      time_t now = time (0); \
      char timebuff[20]; \
      sTm = gmtime (&now); \
      strftime (timebuff, sizeof(timebuff), "%Y-%m-%dT%H:%M:%S", sTm); \
      String prettyFunction(__PRETTY_FUNCTION__); \
      size_t end   = prettyFunction.find('('); \
      size_t begin = prettyFunction.find_last_of(' ', end); \
      std::stringstream ss; \
      if (LOG_DEST != LOG_DEST_SYSLOG) \
        ss << timebuff << " "; \
      if (LOG_DEST == LOG_DEST_COUT_DEFAULT) \
        ss << " [" << prettyFunction.substr(begin+1,end-begin) + "):" << __LINE__ << "] "; \
      ss << args << std::flush; \
      __TRACE(level, ss.str()); \
    } \
  } while(0)

#define TRACE0(args...) _TRACE(0, ##args)
#define TRACE1(args...) _TRACE(1, ##args)
#define TRACE2(args...) _TRACE(2, ##args)
#define TRACE3(args...) _TRACE(3, ##args)
#define TRACE4(args...) _TRACE(4, ##args)
#define TRACE5(args...) _TRACE(5, ##args)
#define TRACE6(args...) _TRACE(6, ##args)
#define TRACE7(args...) _TRACE(7, ##args)
#define TRACE8(args...) _TRACE(8, ##args)
#define TRACE9(args...) _TRACE(9, ##args)

#define TRACET(args...) \
  do { \
    if (LOG_TRANSACTION) \
      _TRACE(LOG_INFO, ##args); \
  } while(0)

#define TARG(varStr, varName) varStr ":" << varName

#define YASSERT(expression, str) do { \
  if(!(expression)) { \
    TRACE0("Assertion failed: \"" #expression "\", " \
           __FILE__ << ":" << __LINE__ << " " << MkString(str)); \
    assert(false); \
  } \
} while(0)

#endif

