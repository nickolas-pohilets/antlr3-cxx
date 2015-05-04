#include <gtest/gtest.h>
#include <antlr3/ConvertUTF.hpp>

using namespace utf;

TEST(UTFTest, TestByteOrder)
{
    {
        UTF8 bytes[5] = { 0xAB, 0x12, 0xCD, 0x42, 0xFF };
        UTF16 out;
        LE::read(bytes, out);
        ASSERT_EQ(out, 0x12AB);
        BE::read(bytes, out);
        ASSERT_EQ(out, 0xAB12);
        out = 0x5678;
        LE::write(bytes, out);
        ASSERT_EQ(bytes[0], 0x78);
        ASSERT_EQ(bytes[1], 0x56);
        ASSERT_EQ(bytes[2], 0xCD);
        ASSERT_EQ(bytes[3], 0x42);
        ASSERT_EQ(bytes[4], 0xFF);
        BE::write(bytes, out);
        ASSERT_EQ(bytes[0], 0x56);
        ASSERT_EQ(bytes[1], 0x78);
        ASSERT_EQ(bytes[2], 0xCD);
        ASSERT_EQ(bytes[3], 0x42);
        ASSERT_EQ(bytes[4], 0xFF);
    }
    {
        UTF8 bytes[5] = { 0xAB, 0x12, 0xCD, 0x42, 0xFF };
        UTF32 out;
        LE::read(bytes, out);
        ASSERT_EQ(out, 0x42CD12AB);
        BE::read(bytes, out);
        ASSERT_EQ(out, 0xAB12CD42);
        out = 0x567890E3;
        LE::write(bytes, out);
        ASSERT_EQ(bytes[0], 0xE3);
        ASSERT_EQ(bytes[1], 0x90);
        ASSERT_EQ(bytes[2], 0x78);
        ASSERT_EQ(bytes[3], 0x56);
        ASSERT_EQ(bytes[4], 0xFF);
        BE::write(bytes, out);
        ASSERT_EQ(bytes[0], 0x56);
        ASSERT_EQ(bytes[1], 0x78);
        ASSERT_EQ(bytes[2], 0x90);
        ASSERT_EQ(bytes[3], 0xE3);
        ASSERT_EQ(bytes[4], 0xFF);
    }
}

static void read8(char const * str, UTF32 ch, ConversionFlags flags, ConversionResult r, size_t n)
{
    char const* s = str;
    UTF32 ch1;
    ConversionResult r1 = Traits<UTF8>::Read(s, str + strlen(str), ch1, flags);
    ASSERT_EQ(r, r1);
    ASSERT_EQ(n, s - str);
    if (r1 == ConversionResult::ConversionOK) {
        ASSERT_EQ(ch, ch1);
    }
}

static void read8ok(char const * str, UTF32 ch, size_t n)
{
    read8(str, ch, ConversionFlags::StrictConversion, ConversionResult::ConversionOK, n);
    read8(str, ch, ConversionFlags::LenientConversion, ConversionResult::ConversionOK, n);
}

static void read8rec(char const* str, size_t n)
{
    read8(str, 0, ConversionFlags::StrictConversion, ConversionResult::SourceIllegal, 0);
    read8(str, UNI_REPLACEMENT_CHAR, ConversionFlags::LenientConversion, ConversionResult::ConversionOK, n);
}

TEST(UTFTest, TestReadUTF8)
{
    read8ok("\x7F", 0x7F, 1);
    read8ok("\xC0\x80", 0, 2);
    read8ok("\xC2\x80", 0x80, 2);
    read8ok("\xDF\xBF", 0x7FF, 2);
    read8ok("\xE0\x80\x80", 0, 3);
    read8ok("\xE0\xA0\x80", 0x800, 3);
    read8ok("\xEF\xBF\xBF", 0xFFFF, 3);
    read8ok("\xF0\x80\x80\x80", 0, 4);
    read8ok("\xF0\x90\x80\x80", 0x10000, 4);
    read8ok("\xF4\x8F\xBF\xBF", 0x10FFFF, 4);
    read8ok("\xF8\x80\x90\x80\x80", 0x10000, 5);
    read8ok("\xF8\x84\x8F\xBF\xBF", 0x10FFFF, 5);
    read8ok("\xFC\x80\x80\x90\x80\x80", 0x10000, 6);
    read8ok("\xFC\x80\x84\x8F\xBF\xBF", 0x10FFFF, 6);

    read8("\x88", 0, ConversionFlags::StrictConversion, ConversionResult::SourceIllegal, 0);
    read8("\xFF", 0, ConversionFlags::StrictConversion, ConversionResult::SourceIllegal, 0);
    read8("\xFF", 0, ConversionFlags::LenientConversion, ConversionResult::SourceExhausted, 0);
    read8rec("\xFF\xC3", 1);
    read8("\xE0\xFE\x80", 0, ConversionFlags::StrictConversion, ConversionResult::SourceIllegal, 0);
    read8("\xE0\xFE\x80", UNI_REPLACEMENT_CHAR, ConversionFlags::LenientConversion, ConversionResult::SourceExhausted, 0);
    read8rec("\xE0\xFE\x80\xC3", 3);

    read8rec("\xF4\x90\x80\x80", 4);
    read8rec("\xF8\x84\x90\x80\x80", 5);
    read8rec("\xFC\x80\x84\x90\x80\x80", 6);

    read8ok("\xED\x9F\xBF", 0xD7FF, 3);
    read8rec("\xED\xA0\x80", 3);
    read8rec("\xED\xBF\xBF", 3);
    read8ok("\xEE\x80\x80", 0xE000, 3);
}

static void write8(UTF32 ch, char const * str)
{
    char buffer[128];
    char* s = buffer;
    size_t n = strlen(str);
    n = n < 1 ? 1 : n;
    ConversionResult r = Traits<UTF8>::Write(s, buffer + n, ch);
    *s = '\0';
    ASSERT_EQ(r, ConversionResult::ConversionOK);
    ASSERT_STREQ(buffer, str);
    ASSERT_EQ(s, buffer + n);
    for (unsigned i = 0; i < n; ++i) {
        s = buffer;
        r = Traits<UTF8>::Write(s, buffer + i, ch);
        ASSERT_EQ(r, ConversionResult::TargetExhausted);
        ASSERT_EQ(s, buffer);
    }
}

TEST(UTFTest, TestWriteUTF8)
{
    write8(0x000000, "\x00");
    write8(0x000001, "\x01");
    write8(0x00007F, "\x7F");
    write8(0x000080, "\xC2\x80");
    write8(0x0007FF, "\xDF\xBF");
    write8(0x000800, "\xE0\xA0\x80");
    write8(0x00FFFF, "\xEF\xBF\xBF");
    write8(0x010000, "\xF0\x90\x80\x80");
    write8(0x10FFFF, "\xF4\x8F\xBF\xBF");
}

short hexval(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if(c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if(c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return -1;
    }
}

static size_t sto16(char const* str8, short* buffer)
{
    size_t k = strlen(str8) / 4;
    for (unsigned i = 0; i < k; ++i) {
        short c3 = hexval(str8[4 * i + 0]);
        short c2 = hexval(str8[4 * i + 1]);
        short c1 = hexval(str8[4 * i + 2]);
        short c0 = hexval(str8[4 * i + 3]);
        buffer[i] = (c3 << 12) | (c2 << 8) | (c1 << 4) | c0;
    }
    return k;
}

static void read16(char const * str8, UTF32 ch, ConversionFlags flags, ConversionResult r, size_t n)
{
    short str[16];
    size_t k = sto16(str8, str);
    short const* s = str;
    UTF32 ch1;
    ConversionResult r1 = Traits<UTF16>::Read(s, s + k, ch1, flags);
    ASSERT_EQ(r, r1);
    ASSERT_EQ(n, s - str);
    if (r1 == ConversionResult::ConversionOK) {
        ASSERT_EQ(ch, ch1);
    }
}

static void read16ok(char const * str, UTF32 ch, size_t n)
{
    read16(str, ch, ConversionFlags::StrictConversion, ConversionResult::ConversionOK, n);
    read16(str, ch, ConversionFlags::LenientConversion, ConversionResult::ConversionOK, n);
}

static void read16rec(char const* str, size_t n)
{
    read16(str, 0, ConversionFlags::StrictConversion, ConversionResult::SourceIllegal, 0);
    read16(str, UNI_REPLACEMENT_CHAR, ConversionFlags::LenientConversion, ConversionResult::ConversionOK, n);
}

TEST(UTFTest, TestReadUTF16)
{
    read16ok("0000", 0, 1);
    read16ok("D7FF", 0xD7FF, 1);
    read16rec("D8000026", 1);
    read16rec("DFFF0026", 1);
    read16rec("DC00", 1);
    read16ok("E000", 0xE000, 1);
    read16ok("FFFF", 0xFFFF, 1);
    read16ok("D800DC00", 0x10000, 2);
    read16ok("DBFFDFFF", 0x10FFFF, 2);
    read16("D800", 0, ConversionFlags::StrictConversion, ConversionResult::SourceExhausted, 0);
    read16("D800", 0, ConversionFlags::LenientConversion, ConversionResult::SourceExhausted, 0);
}

static void write16(UTF32 ch, char const * str8)
{
    short buffer[128];
    short str[16];
    size_t n = sto16(str8, str);
    short* s = buffer;
    ConversionResult r = Traits<UTF16>::Write(s, buffer + n, ch);
    ASSERT_EQ(r, ConversionResult::ConversionOK);
    for (size_t i = 0; i < n; ++i) {
        ASSERT_EQ(buffer[i], str[i]);
    }
    ASSERT_EQ(s, buffer + n);
    for (unsigned i = 0; i < n; ++i) {
        s = buffer;
        r = Traits<UTF16>::Write(s, buffer + i, ch);
        ASSERT_EQ(r, ConversionResult::TargetExhausted);
        ASSERT_EQ(s, buffer);
    }
}

TEST(UTFTest, TestWriteUTF16)
{
    write16(0, "0000");
    write16(1, "0001");
    write16(0xD7FF, "D7FF");
    write16(0xE000, "E000");
    write16(0xFFFF, "FFFF");
    write16(0x10000, "D800DC00");
    write16(0x10FFFF, "DBFFDFFF");
}

static void read32(long val, UTF32 ch, ConversionFlags flags, ConversionResult r) {
    long const* ptr = &val;
    UTF32 ch1;
    ConversionResult r1 = Traits<UTF32>::Read(ptr, ptr + 1, ch1, flags);
    ASSERT_EQ(r, r1);
    if (r1 == ConversionResult::ConversionOK) {
        ASSERT_EQ(ptr, &val + 1);
        ASSERT_EQ(ch, ch1);
    } else {
        ASSERT_EQ(ptr, &val);
    }
}

static void read32ok(long val) {
    read32(val, UTF32(val), ConversionFlags::StrictConversion, ConversionResult::ConversionOK);
    read32(val, UTF32(val), ConversionFlags::LenientConversion, ConversionResult::ConversionOK);
}

static void read32rec(long val) {
    read32(val, 0, ConversionFlags::StrictConversion, ConversionResult::SourceIllegal);
    read32(val, UNI_REPLACEMENT_CHAR, ConversionFlags::LenientConversion, ConversionResult::ConversionOK);
}

TEST(UTFTest, TestReadUTF32)
{
    read32ok(0);
    read32ok(1);
    read32ok(0xD7FF);
    read32rec(0xD800);
    read32rec(0xDFFF);
    read32ok(0xE000);
    read32ok(0xFFFF);
    read32ok(0x10000);
    read32ok(0x10FFFF);
    read32rec(0x110000);
    read32rec(-1);
}

TEST(UTFTest, TestWriteUTF32)
{
    long buffer[2];
    long* ptr = buffer;
    ASSERT_EQ(ConversionResult::ConversionOK, Traits<UTF32>::Write(ptr, ptr + 1, 0x10FFFF));
    ASSERT_EQ(0x10FFFF, buffer[0]);
    ASSERT_EQ(buffer + 1, ptr);
    ptr = buffer;
    ASSERT_EQ(ConversionResult::TargetExhausted, Traits<UTF32>::Write(ptr, ptr, 0x10FFFF));
    ASSERT_EQ(buffer, ptr);
}
