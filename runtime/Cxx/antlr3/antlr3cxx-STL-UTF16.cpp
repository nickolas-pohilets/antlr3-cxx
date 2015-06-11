#include <antlr3/antlr3cxx-STL-UTF16.hpp>
#include <antlr3/ConvertUTF.hpp>

typedef antlr3ex::StdUTF16StringTraits StringTraits;
typedef StringTraits::String String;
typedef StringTraits::StringLiteral StringLiteral;

template class antlr3<StringTraits>;

namespace {

    template<class SrcChar, class DstChar> struct ConvertionFunc;
    template<> struct ConvertionFunc<char, char16_t> {
        template<class SrcIt, class DstIt>
        static void convert(SrcIt& sourceStart, SrcIt sourceEnd, DstIt& targetStart, DstIt targetEnd) {
            utf::ConversionResult r = utf::Convert<utf::UTF8, utf::UTF16>(sourceStart, sourceEnd, targetStart, targetEnd, utf::ConversionFlags::LenientConversion);
            assert(r == utf::ConversionResult::ConversionOK);
        }
    };
    template<> struct ConvertionFunc<char16_t, char> {
        template<class SrcIt, class DstIt>
        static void convert(SrcIt& sourceStart, SrcIt sourceEnd, DstIt& targetStart, DstIt targetEnd) {
            utf::ConversionResult r = utf::Convert<utf::UTF16, utf::UTF8>(sourceStart, sourceEnd, targetStart, targetEnd, utf::ConversionFlags::LenientConversion);
            assert(r == utf::ConversionResult::ConversionOK);
        }
    };

    template<class Char> struct UTFType;
    template<> struct UTFType<char> { typedef utf::UTF8 t; };
    template<> struct UTFType<char16_t> { typedef utf::UTF16 t; };

    template<class SrcChar, class DstChar>
    std::basic_string<DstChar>& appendUTF(std::basic_string<DstChar>& dst, SrcChar const * src, size_t len)
    {
        typedef typename UTFType<SrcChar>::t SrcT;
        typedef typename UTFType<DstChar>::t DstT;

        size_t oldLen = dst.size();

        // 1 - Dry run - determine needed dst size
        {
            auto srcStart = reinterpret_cast<SrcT const *>(src);
            auto srcEnd = srcStart + len;
            utf::DummyWriteIterator<DstT> dstStart(0), dstEnd;
            ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
            size_t dstCapacity = dst.length() + dstStart.pos();
            dst.resize(dstCapacity);
        }

        // 2 - Actual conversion
        {
            auto srcStart = reinterpret_cast<SrcT const *>(src);
            auto srcEnd = srcStart + len;
            auto dstStart = reinterpret_cast<DstT *>(&dst[0] + oldLen);
            auto dstEnd = dstStart - oldLen + dst.capacity();
            ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
        }
        return dst;
    }
    
    template <class T>
    class OutStreamIterator
        : public std::iterator<std::output_iterator_tag, void, void, void, void>
    {
    public:
        typedef T char_type;
        typedef std::char_traits<T> traits_type;
        typedef std::basic_ostream<T> ostream_type;
    private:
        ostream_type* stream_;
    public:
        OutStreamIterator() : stream_() {}
        OutStreamIterator(ostream_type& __s) : stream_(&__s) {}
        
        OutStreamIterator& operator=(const T& __value_) {
            *stream_ << __value_;
            return *this;
        }

        OutStreamIterator& operator*()     {return *this;}
        OutStreamIterator& operator++()    {return *this;}
        OutStreamIterator& operator++(int) {return *this;}
        
        bool operator==(OutStreamIterator) const { return false; }
        bool operator<(OutStreamIterator) const { return true; }
    };
    
    template<class SrcChar, class DstChar>
    std::basic_ostream<DstChar> & outputUTF(std::basic_ostream<DstChar> & out, SrcChar const * src, size_t len) {
        typedef typename UTFType<SrcChar>::t SrcT;
        typedef typename UTFType<DstChar>::t DstT;
        auto srcStart = reinterpret_cast<SrcT const *>(src);
        auto srcEnd = srcStart + len;
        OutStreamIterator<DstChar> dstStart(out), dstEnd;
        ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
        return out;
    }
}

namespace antlr3ex {

std::string StringTraits::toUTF8(String const & s) {
    std::string retVal;
    appendToUTF8(retVal, s);
    return std::move(retVal);
}

std::string StringTraits::toUTF8(StringLiteral s) {
    std::string retVal;
    appendToUTF8(retVal, s);
    return std::move(retVal);
}

String StringTraits::fromUTF8(std::string const & s) {
    String retVal;
    appendUTF8(retVal, s);
    return std::move(retVal);
}

String StringTraits::fromUTF8(char const * s) {
    String retVal;
    appendUTF8(retVal, s);
    return std::move(retVal);
}

std::string& StringTraits::appendToUTF8(std::string& s8, String const & s) {
    return appendUTF(s8, s.c_str(), s.size());
}

std::string& StringTraits::appendToUTF8(std::string& s8, StringLiteral s) {
    return appendUTF(s8, s.data(), s.size());
}

String& StringTraits::appendUTF8(String& s, std::string const & s8) {
    return appendUTF(s, s8.c_str(), s8.size());
}

String& StringTraits::appendUTF8(String& s, char const * s8) {
    return appendUTF(s, s8, s8 ? strlen(s8) : 0);
}

} // namespace antlr3ex

namespace std {

std::ostream& operator<<(std::ostream& s, String const & str) {
    return outputUTF(s, str.c_str(), str.size());
}

std::ostream& operator<<(std::ostream& s, StringLiteral str) {
    return outputUTF(s, str.data(), str.size());
}

} // namespace std
