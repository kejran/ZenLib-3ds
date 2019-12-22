#pragma once

#include <memory>

namespace ZenLoad {
  class ZenParser;
  };

namespace Daedalus {

class ZString final {
  public:
    ZString();
    ZString(const char* s);
    ZString(std::string&& s);
    ZString(std::unique_ptr<char[]>&& s);
    ZString(ZenLoad::ZenParser& input);

    ZString(const ZString& other)=default;
    ZString& operator = (const ZString& other) = default;

    ZString(ZString&& other);
    ZString& operator = (ZString&& other);

    bool operator == (const char*    r) const;
    bool operator != (const char*    r) const;

    bool operator == (const ZString& r) const;
    bool operator != (const ZString& r) const;

    bool operator <  (const ZString& r) const;
    bool operator <= (const ZString& r) const;
    bool operator >  (const ZString& r) const;
    bool operator >= (const ZString& r) const;

    bool operator <  (const char* r) const;
    bool operator <= (const char* r) const;
    bool operator >  (const char* r) const;
    bool operator >= (const char* r) const;

    ZString operator + (const ZString& r) const;

    size_t      find(const char* src) const;

    const char* c_str() const;
    bool        empty() const;
    size_t      size()  const;

    static ZString toStr(int32_t v);
    static ZString toStr(int64_t v);
    static ZString toStr(float   v);

  private:
    /* Comprimise between efficentcy and redability:
     * std::shared_ptr<std::string> is very much pointer to pointer, and better to have shared_ptr for char array
     * but char array implementation is... very har to read kind :)
     * */
    std::shared_ptr<std::string> val;
  };

}
