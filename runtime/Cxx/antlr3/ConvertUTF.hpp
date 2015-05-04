/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8.  Header file.

    Several functions are included here, forming a complete set of
    conversions between the three formats.  UTF-7 is not included
    here, but is handled in a separate source file.

    Each of these routines takes pointers to input buffers and output
    buffers.  The input buffers are const.

    Each routine converts the text between *sourceStart and sourceEnd,
    putting the result into the buffer between *targetStart and
    targetEnd. Note: the end pointers are *after* the last item: e.g. 
    *(sourceEnd - 1) is the last item.

    The return result indicates whether the conversion was successful,
    and if not, whether the problem was in the source or target buffers.
    (Only the first encountered problem is indicated.)

    After the conversion, *sourceStart and *targetStart are both
    updated to point to the end of last text successfully converted in
    the respective buffers.

    Input parameters:
	sourceStart - pointer to a pointer to the source buffer.
		The contents of this are modified on return so that
		it points at the next thing to be converted.
	targetStart - similarly, pointer to pointer to the target buffer.
	sourceEnd, targetEnd - respectively pointers to the ends of the
		two buffers, for overflow checking only.

    These conversion functions take a ConversionFlags argument. When this
    flag is set to strict, both irregular sequences and isolated surrogates
    will cause an error.  When the flag is set to lenient, both irregular
    sequences and isolated surrogates are converted.

    Whether the flag is strict or lenient, all illegal sequences will cause
    an error return. This includes sequences such as: <F4 90 80 80>, <C0 80>,
    or <A0> in UTF-8, and values above 0x10FFFF in UTF-32. Conformant code
    must check for illegal sequences.

    When the flag is set to lenient, characters over 0x10FFFF are converted
    to the replacement character; otherwise (when the flag is set to strict)
    they constitute an error.

    Output parameters:
	The value ANTLR3_T("sourceIllegal") is returned from some routines if the input
	sequence is malformed.  When ANTLR3_T("sourceIllegal") is returned, the source
	value will point to the illegal value that caused the problem. E.g.,
	in UTF-8 when a sequence is malformed, it points to the start of the
	malformed sequence.  

    Author: Mark E. Davis, 1994.
    Rev History: Rick McGowan, fixes & updates May 2001.
		 Fixes & updates, Sept 2001.

------------------------------------------------------------------------ */

/* ---------------------------------------------------------------------
    The following 4 definitions are compiler-specific.
    The C standard does not guarantee that wchar_t has at least
    16 bits, so wchar_t is no less portable than unsigned short!
    All should be unsigned values to avoid sign extension during
    bit mask & shift operations.
------------------------------------------------------------------------ */


// Changes for ANTLR3 - Jim Idle, January 2008.
// builtin types defined for Unicode types changed to
// aliases for the types that are system determined by
// ANTLR at compile time.
//
// typedef unsigned long	UTF32;	/* at least 32 bits */
// typedef unsigned short	UTF16;	/* at least 16 bits */
// typedef unsigned char	UTF8;	/* typically 8 bits */
// typedef unsigned char	Boolean; /* 0 or 1 */

#ifndef _ANTLR3_CONVERTUTF_HPP
#define _ANTLR3_CONVERTUTF_HPP

#include <cstdint>
#include <cstddef>

namespace utf {

typedef std::uint32_t	UTF32;	/* at least 32 bits */
typedef std::uint16_t	UTF16;	/* at least 16 bits */
typedef std::uint8_t	UTF8;	/* typically 8 bits */

enum class ConversionResult
{
	ConversionOK, 		/* conversion successful */
	SourceExhausted,	/* partial character in source, but hit end */
	TargetExhausted,	/* insuff. room in target for conversion */
	SourceIllegal		/* source sequence is illegal/malformed */
};

enum class ConversionFlags
{
    StrictConversion,
    LenientConversion
};

/* Some fundamental constants */
UTF32 const UNI_REPLACEMENT_CHAR = 0x0000FFFD;
UTF32 const UNI_MAX_BMP = 0x0000FFFF;
UTF32 const UNI_MAX_UTF16 = 0x0010FFFF;
UTF32 const UNI_MAX_UTF32 = 0x7FFFFFFF;
UTF32 const UNI_MAX_LEGAL_UTF32 = 0x0010FFFF;

UTF32 const UNI_SUR_HIGH_START = 0xD800;
UTF32 const UNI_SUR_HIGH_END = 0xDBFF;
UTF32 const UNI_SUR_LOW_START = 0xDC00;
UTF32 const UNI_SUR_LOW_END = 0xDFFF;
UTF32 const halfShift = 10;
UTF32 const halfBase = 0x0010000UL;
UTF32 const halfMask = 0x3FFUL;

extern const signed char trailingBytesForUTF8[];
extern const UTF32 offsetsFromUTF8[];
extern const UTF8 firstByteMark[];

inline bool IsValidCP(UTF32 ch) {
    return ch < UNI_SUR_HIGH_START || (ch > UNI_SUR_LOW_END && ch <= UNI_MAX_LEGAL_UTF32);
}

template<class Ch> struct CodeUnitForChar;
template<> struct CodeUnitForChar<char> { typedef UTF8 type; };
template<> struct CodeUnitForChar<char16_t> { typedef UTF16 type; };
template<> struct CodeUnitForChar<char32_t> { typedef UTF32 type; };

template<class T> class Traits;

template<> class Traits<UTF8>
{
public:
    template<class It8>
    static ConversionResult Read(It8& source, It8 sourceEnd, UTF32& ch, ConversionFlags flags) {
        short extraBytesToRead = trailingBytesForUTF8[UTF8(*source)];
        if (extraBytesToRead < 0) {
            if (flags == ConversionFlags::LenientConversion) {
                ch = UNI_REPLACEMENT_CHAR;
                It8 srcOrig = source;
                while (true) {
                    ++source;

                    if (!(source < sourceEnd)) {
                        // Found end of data while consuming illegal bytes
                        // Return source exhausted in case if next chunk starts with invalid data.
                        // All continious range of invalid chars should be replaced by single replacement
                        // char regardless of splitting input into chunks.
                        // So we delay processing, until next chunk arrives.
                        source = srcOrig;
                        return ConversionResult::SourceExhausted;
                    }

                    if (trailingBytesForUTF8[UTF8(*source)] >= 0)
                    {
                        // We've found start of the next character.
                        // It may be invalid as well, but it will be replaced with individual replacement
                        // character.
                        return ConversionResult::ConversionOK;
                    }
                }

            } else {
                return ConversionResult::SourceIllegal;
            }
        }

        It8 savedSrc = source;
        ch = UTF8(*source++);
        for(int i = 0; i < extraBytesToRead; ++i) {
            if (!(source < sourceEnd)) {
                return ConversionResult::SourceExhausted;
            }
            UTF8 c = *source++;
            if (trailingBytesForUTF8[c] != -1) {
                if (flags == ConversionFlags::LenientConversion) {
                    while (true) {
                        if (!(source < sourceEnd)) {
                            source = savedSrc;
                            return ConversionResult::SourceExhausted;
                        }
                        if (trailingBytesForUTF8[UTF8(*source)] >= 0) {
                            break;
                        }
                        ++source;
                    }
                    ch = UNI_REPLACEMENT_CHAR;
                    return ConversionResult::ConversionOK;
                } else {
                    source = savedSrc;
                    return ConversionResult::SourceIllegal;
                }
            }
            ch = (ch << 6) + c;
        }

        ch -= offsetsFromUTF8[extraBytesToRead];

        if (!IsValidCP(ch)) {
            if (flags == ConversionFlags::LenientConversion) {
                ch = UNI_REPLACEMENT_CHAR;
                return ConversionResult::ConversionOK;
            } else {
                source = savedSrc;
                return ConversionResult::SourceIllegal;
            }
        }

        return ConversionResult::ConversionOK;
    }

    template<class It8>
    static ConversionResult Write(It8& target, It8 targetEnd, UTF32 ch) {
        const UTF32 byteMask = 0xBF;
        const UTF32 byteMark = 0x80;

        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        unsigned short bytesToWrite = 0;
        if (ch < (UTF32)0x80) {
            bytesToWrite = 1;
        } else if (ch < (UTF32)0x800) {
            bytesToWrite = 2;
        } else if (ch < (UTF32)0x10000) {
            bytesToWrite = 3;
        } else {
            bytesToWrite = 4;
        }

        UTF8 buf[4];
        switch (bytesToWrite) { /* note: everything falls through. */
            case 4: buf[3] = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            case 3: buf[2] = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            case 2: buf[1] = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            case 1: buf[0] = (UTF8) (ch | firstByteMark[bytesToWrite]);
        }

        It8 origTarget = target;
        UTF8* ptr = buf;
        while (bytesToWrite > 0) {
            if (!(target < targetEnd)) {
                target = origTarget;
                return ConversionResult::TargetExhausted;
            }
            *target++ = *ptr++;
            --bytesToWrite;
        }
        return ConversionResult::ConversionOK;
    }
};

template<> class Traits<UTF16>
{
public:
    template<class It16>
    static ConversionResult Read(It16& source, It16 sourceEnd, UTF32& ch, ConversionFlags flags) {
        ch = UTF16(*source);
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            It16 srcOrig = source++;
            if (source < sourceEnd) {
                UTF32 ch2 = UTF16(*source);
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                    return ConversionResult::ConversionOK;
                } else {
                    /* it's an unpaired high surrogate */
                    if (flags == ConversionFlags::LenientConversion) {
                        ch = UNI_REPLACEMENT_CHAR;
                        return ConversionResult::ConversionOK;
                    } else {
                        source = srcOrig; /* return to the illegal value itself */
                        return ConversionResult::SourceIllegal;
                    }
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                source = srcOrig; /* return to the high surrogate */
                return ConversionResult::SourceExhausted;
            }
        } else if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
            if (flags == ConversionFlags::LenientConversion) {
                ch = UNI_REPLACEMENT_CHAR;
                ++source;
                return ConversionResult::ConversionOK;
            } else {
                /* UTF-16 surrogate values are illegal in UTF-32 */
                return ConversionResult::SourceIllegal;
            }
        } else {
            ++source;
            return ConversionResult::ConversionOK;
        }
    }

    template<class It16>
    static ConversionResult Write(It16& target, It16 targetEnd, UTF32 ch) {
        if (target < targetEnd) {
            if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
                *target++ = (UTF16)ch; /* normal case */
                return ConversionResult::ConversionOK;
            } else {
                /* target is a character in range 0xFFFF - 0x10FFFF. */
                ch -= halfBase;
                It16 origTarget = target;
                *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
                if (target < targetEnd) {
                    *target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
                    return ConversionResult::ConversionOK;
                } else {
                    target = origTarget;
                    return ConversionResult::TargetExhausted;
                }
            }
        } else {
            return ConversionResult::TargetExhausted;
        }
    }
};

template<> class Traits<UTF32>
{
public:
    template<class It32>
    static ConversionResult Read(It32& source, It32 sourceEnd, UTF32& ch, ConversionFlags flags) {
        ch = UTF32(*source);
        if ((ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) || ch > UNI_MAX_LEGAL_UTF32) {
            if (flags == ConversionFlags::LenientConversion) {
                ++source;
                ch = UNI_REPLACEMENT_CHAR;
                return ConversionResult::ConversionOK;
            } else {
                return ConversionResult::SourceIllegal;
            }
        } else {
            ++source;
            return ConversionResult::ConversionOK;
        }
    }

    template<class It32>
    static ConversionResult Write(It32& target, It32 targetEnd, UTF32 ch) {
        if (target < targetEnd) {
            *target++ = ch; /* normal case */
            return ConversionResult::ConversionOK;
        } else {
            return ConversionResult::TargetExhausted;
        }
    }
};

template<class SrcUTF, class DstUTF, class SrcIt, class DstIt>
ConversionResult Convert(SrcIt& sourceStart, SrcIt sourceEnd, DstIt& targetStart, DstIt targetEnd, ConversionFlags flags) {
    while (sourceStart < sourceEnd) {
        SrcIt savedSrc = sourceStart;
        UTF32 ch;
        ConversionResult r = Traits<SrcUTF>::Read(sourceStart, sourceEnd, ch, flags);
        if (r != ConversionResult::ConversionOK) {
            return r;
        }
        r = Traits<DstUTF>::Write(targetStart, targetEnd, ch);
        if (r != ConversionResult::ConversionOK) {
            sourceStart = savedSrc;
            return r;
        }
    }
    return ConversionResult::ConversionOK;
}

class LE {
public:
    template<class It8>
    static void read(It8 it, UTF8& out)
    {
        out = UTF8(*it);
    }

    template<class It8>
    static void write(It8 it, UTF8 in)
    {
        *it = in;
    }

    template<class It8>
    static void read(It8 it, UTF16& out)
    {
        UTF8 lo = *it;
        UTF8 hi = *++it;

        out = (UTF16(hi) << 8) | UTF16(lo);
    }

    template<class It8>
    static void write(It8 it, UTF16 in)
    {
        *it = UTF8(in);
        *++it = UTF8(in >> 8);
    }

    template<class It8>
    static void read(It8 it, UTF32& out)
    {
        UTF8 b0 = *it;
        UTF8 b1 = *++it;
        UTF8 b2 = *++it;
        UTF8 b3 = *++it;
        out = (UTF32(b3) << 24) | (UTF32(b2) << 16) | (UTF32(b1) << 8) | UTF32(b0);
    }

    template<class It8>
    static void write(It8 it, UTF32 in)
    {
        *it = UTF8(in);
        *++it = UTF8(in >> 8);
        *++it = UTF8(in >> 16);
        *++it = UTF8(in >> 24);
    }
};

class BE {
public:
    template<class It8>
    static void read(It8 it, UTF8& out)
    {
        out = UTF8(*it);
    }

    template<class It8>
    static void write(It8 it, UTF8 in)
    {
        *it = in;
    }
    
    template<class It8>
    static void read(It8 it, UTF16& out)
    {
        UTF8 hi = *it;
        UTF8 lo = *++it;

        out = (UTF16(hi) << 8) | UTF16(lo);
    }

    template<class It8>
    static void write(It8 it, UTF16 in)
    {
        *it = UTF8(in >> 8);
        *++it = UTF8(in);
    }

    template<class It8>
    static void read(It8 it, UTF32& out)
    {
        UTF8 b3 = *it;
        UTF8 b2 = *++it;
        UTF8 b1 = *++it;
        UTF8 b0 = *++it;
        out = (UTF32(b3) << 24) | (UTF32(b2) << 16) | (UTF32(b1) << 8) | UTF32(b0);
    }

    template<class It8>
    static void write(It8 it, UTF32 in)
    {
        *it = UTF8(in >> 24);
        *++it = UTF8(in >> 16);
        *++it = UTF8(in >> 8);
        *++it = UTF8(in);
    }
};

template<class It8, class CodeUnit, class ByteOrder>
class CodeUnitIterator
{
public:
    class Ref
    {
    public:
        explicit Ref(It8 it) : it_(it) {}

        operator CodeUnit() const
        {
            CodeUnit retVal;
            ByteOrder::read(it_, retVal);
            return retVal;
        }

        CodeUnit operator=(CodeUnit x) const
        {
            ByteOrder::write(it_, x);
            return x;
        }
   private:
        It8 it_;

    };

    explicit CodeUnitIterator(It8 it) : it_(it) {}

    bool operator<(const CodeUnitIterator& other) { return it_ < other.it_; }
    bool operator==(const CodeUnitIterator& other) { return it_ == other.it_; }

    CodeUnitIterator& operator++()
    {
        for (std::size_t i = 0; i < sizeof(CodeUnit); ++i) {
            ++it_;
        }
        return *this;
    }

    CodeUnitIterator operator++(int)
    {
        CodeUnitIterator retVal(it_);
        ++(*this);
        return retVal;
    }

    Ref operator*() const
    {
        return Ref(it_);
    }
private:
    It8 it_;
};

template<class CodeUnit> class DummyWriteIterator
{
public:
    class DummyRef {
    public:
        CodeUnit operator=(CodeUnit x) { return x; }
    };
    
    DummyWriteIterator(std::size_t pos = std::size_t(std::ptrdiff_t(-1))) : pos_(pos) {}

    std::size_t pos() const { return pos_; }

    bool operator<(DummyWriteIterator other) const { return pos_ < other.pos_; }
    bool operator==(DummyWriteIterator other) const { return pos_ == other.pos_; }

    DummyWriteIterator& operator++()
    {
        ++pos_;
        return *this;
    }

    DummyWriteIterator operator++(int)
    {
        return DummyWriteIterator(pos_++);
    }

    DummyRef operator*() const { return DummyRef(); }
private:
    std::size_t pos_;
};

} // namespace utf

#endif

/* --------------------------------------------------------------------- */
