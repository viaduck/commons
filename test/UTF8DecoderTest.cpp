#include "UTF8DecoderTest.h"

#include <commons/UTF8Decoder.h>

// generic Replacer interface implementation used by tests
class ReplacerImpl {
public:
    ReplacerImpl(std::function<bool(UTF8Decoder<ReplacerImpl> &decoder, const UTF8Char &, String &, uint32_t &)> replacerFn) : mReplacerFn(replacerFn) {

    }

    bool replace(UTF8Decoder<ReplacerImpl> &decoder, const UTF8Char &aChar, String &out, uint32_t &index) {
        return mReplacerFn(decoder, aChar, out, index);
    }

private:
    std::function<bool(UTF8Decoder<ReplacerImpl> &decoder, const UTF8Char &, String &, uint32_t &)> mReplacerFn;
};

#define PILE_OF_POO "\xf0\x9f\x92\xa9"  // U+1F4A9
#define BOMB "\xf0\x9f\x92\xa3"         // U+1F4A3
#define CAT "\xf0\x9f\x90\xb1"          // U+1F431
#define WOMAN_HEART_WOMAN "\xf0\x9f\x91\xa9\xe2\x80\x8d\xe2\x9d\xa4\xef\xb8\x8f\xe2\x80\x8d\xf0\x9f\x91\xa9"          // U+1F469 U+200D U+2764 U+FE0F U+200D U+1F469
#define BLACK_MAN "\xf0\x9f\x91\xa8\xf0\x9f\x8f\xbf"            // U+1F468 U+1F3FF
#define WHITE_MAN "\xf0\x9f\x91\xa8\xf0\x9f\x8f\xbb"            // U+1F468 U+1F3FB

ReplacerImpl noop([] (UTF8Decoder<ReplacerImpl> &, const UTF8Char &, String &, uint32_t &) -> bool {
    return false;
});

TEST(UTF8DecoderTest, NoReplacementASCII) {
    {
        const String source = "abcdefghijklmnopqrstuvwxyz\n\r\b\t0123456789";
        UTF8Decoder<ReplacerImpl> decoder(source, noop);
        // should not replace anything
        ASSERT_EQ(source, decoder.replace());
    }
    {
        const String source = "";
        UTF8Decoder<ReplacerImpl> decoder(source, noop);
        // should not replace anything
        ASSERT_EQ(source, decoder.replace());
    }
    {
        const String source("0123456789\0abcdefghijklmnopqrstuvwxyz", 37);
        UTF8Decoder<ReplacerImpl> decoder(source, noop);
        // should not replace anything
        ASSERT_EQ(source, decoder.replace());
    }
}

TEST(UTF8DecoderTest, NoReplacementUnicode) {
    {
        const String source("aa" BOMB "a" PILE_OF_POO "bb" BOMB "bb");
        UTF8Decoder<ReplacerImpl> decoder(source, noop);
        // should not replace anything
        ASSERT_EQ(source, decoder.replace());
    }
    {
        const String source = "";
        UTF8Decoder<ReplacerImpl> decoder(source, noop);
        // should not replace anything
        ASSERT_EQ(source, decoder.replace());
    }
    {
        const String source("aa" BOMB "a\n\0" PILE_OF_POO "\0b\tb" BOMB "bb", 21);
        UTF8Decoder<ReplacerImpl> decoder(source, noop);
        // should not replace anything
        ASSERT_EQ(source, decoder.replace());
    }
}

TEST(UTF8DecoderTest, ReplacementUnicode) {
    ReplacerImpl replaceUnicode([] (UTF8Decoder<ReplacerImpl> &, const UTF8Char &c, String &out, uint32_t &) -> bool {
        switch (c.codepoint()) {
            case 0x1F4A9:           // PILE OF POO
                out += "<CENSORED>";
                return true;
            case 0x1F4A3:           // BOMB
                out += "<CAUGHT!>";
                return true;
            default:
                return false;
        }
    });

    {
        const String source("aa" BOMB "a" PILE_OF_POO "bb" BOMB "bb");
        UTF8Decoder<ReplacerImpl> decoder(source, replaceUnicode);
        ASSERT_EQ(String("aa<CAUGHT!>a<CENSORED>bb<CAUGHT!>bb"), decoder.replace());
    }

    {
        const String source("aa" BOMB "a" PILE_OF_POO "basdfasdf" CAT "b" BOMB "bb");
        UTF8Decoder<ReplacerImpl> decoder(source, replaceUnicode);
        ASSERT_EQ(String("aa<CAUGHT!>a<CENSORED>basdfasdf" CAT "b<CAUGHT!>bb"), decoder.replace());
    }
}

TEST(UTF8DecoderTest, ReplacementASCII) {
    ReplacerImpl replaceASCII([] (UTF8Decoder<ReplacerImpl> &, const UTF8Char &c, String &out, uint32_t &) -> bool {
        switch (c.codepoint()) {
            case 'A':
                out += "a";
                return true;
            case 'Z':
                out += "z";
                return true;
            default:
                return false;
        }
    });
    {
        const String source("abcdefABCDEFGZzZzZzyyx");
        UTF8Decoder<ReplacerImpl> decoder(source, replaceASCII);
        ASSERT_EQ(String("abcdefaBCDEFGzzzzzzyyx"), decoder.replace());
    }
}


TEST(UTF8DecoderTest, ReplacementMultipleUnicode) {
    ReplacerImpl replaceUnicode([] (UTF8Decoder<ReplacerImpl> &decoder, const UTF8Char &c, String &out, uint32_t &index) -> bool {
        uint32_t oldIndex = index;
        switch (c.codepoint()) {
            case 0x1F4A9:           // PILE OF POO
                out += "<CENSORED>";
                return true;
            case 0x1F468:           // MAN
                if (decoder.nextCodepoint(index).codepoint() == 0x1F3FF) {        // BLACK
                    out += "*";
                    return true;
                }
                index = oldIndex;       // reset index since it was modified by nextCodepoint(..)
                if (decoder.nextCodepoint(index).codepoint() == 0x1F3FB) {        // WHITE
                    out += "+";
                    return true;
                }
                break;
            case 0x1F469:           // WOMAN HEART WOMAN
                if (decoder.nextCodepoint(index).codepoint() == 0x200D && decoder.nextCodepoint(index).codepoint() == 0x2764 &&
                        decoder.nextCodepoint(index).codepoint() == 0xFE0f && decoder.nextCodepoint(index).codepoint() == 0x200D &&
                        decoder.nextCodepoint(index).codepoint() == 0x1F469) {
                    out += "<3";
                    return true;
                }
                break;
            default:
                break;
        }
        return false;
    });

    {
        const String source("aa" WOMAN_HEART_WOMAN "a" PILE_OF_POO "bb" BLACK_MAN "b" WHITE_MAN "b");
        UTF8Decoder<ReplacerImpl> decoder(source, replaceUnicode);
        ASSERT_EQ(String("aa" "<3" "a" "<CENSORED>" "bb" "*" "b" "+" "b"), decoder.replace());
    }

    {
        const String source("aa" WOMAN_HEART_WOMAN "a" PILE_OF_POO "b" "x___x" WOMAN_HEART_WOMAN "b" BLACK_MAN "bb");
        UTF8Decoder<ReplacerImpl> decoder(source, replaceUnicode);
        ASSERT_EQ(String("aa" "<3" "a" "<CENSORED>" "b" "x___x" "<3" "b" "*" "bb"), decoder.replace());
    }
}

TEST(UTF8DecoderTest, InvalidUTF8) {
    ReplacerImpl replaceASCII([] (UTF8Decoder<ReplacerImpl> &, const UTF8Char &c, String &out, uint32_t &) -> bool {
        switch (c.codepoint()) {
            case 0xf09f:
                ADD_FAILURE() << "Must NOT replace invalid codepoint, e.g. codepoint must NOT be recognized!";
                out += "_";
                return true;
            default:
                return false;
        }
    });
    {
        const String source("abcdef\xf0\x9fzZzZzyyx");
        UTF8Decoder<ReplacerImpl> decoder(source, replaceASCII);
        ASSERT_EQ(String("abcdef\xf0\x9fzZzZzyyx"), decoder.replace());
    }
    {
        const String source("abcdef\xf0\x9f", 8);
        // test if incomplete UTF-8 sequence does not SIGSEGV, nor endless loop
        UTF8Decoder<ReplacerImpl> decoder(source, replaceASCII);
        ASSERT_EQ(String("abcdef\xf0\x9f", 8), decoder.replace());
    }
}


TEST(UTF8DecoderTest, AccessOutOfRange) {
    ReplacerImpl replaceASCII([] (UTF8Decoder<ReplacerImpl> &replacer, const UTF8Char &c, String &, uint32_t &index) -> bool {
        uint32_t oldIndex = index;
        switch (c.codepoint()) {
            case 'x': {       // 'x' is last character in string
                // acessing next should return empty codepoint object and not crash
                UTF8Char c2 = replacer.nextCodepoint(oldIndex);
                EXPECT_EQ(0u, c2.codepoint());
                EXPECT_EQ(0u, c2.encodedSize());
                EXPECT_FALSE(c2.isValid());
                return false;
            }
            default:
                return false;
        }
    });
    {
        const String source("abcdef\xf0\x9fzZzZzyyx");
        UTF8Decoder<ReplacerImpl> decoder(source, replaceASCII);
        ASSERT_EQ(String("abcdef\xf0\x9fzZzZzyyx"), decoder.replace());     // no replace
    }
}
