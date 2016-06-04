#ifndef UTF8DECODER_H
#define UTF8DECODER_H

#include <libCom/String.h>
#include <set>

class UTF8Char {
public:
    UTF8Char(uint32_t codepoint, uint8_t size) : mCodepoint(codepoint), mEncodedSize(size) {

    }

    inline uint32_t codepoint() const {
        return mCodepoint;
    }

    inline uint8_t encodedSize() const {
        return mEncodedSize;
    }

    inline bool isValid() const {
        return mEncodedSize != 0;
    }

private:
    uint32_t mCodepoint;
    uint8_t mEncodedSize;
};

/**
 * Decodes a byte stream as UTF-8, providing some actions on each decoded Unicode codepoint
 */
template <typename Replacer>
class UTF8Decoder {
public:
    /**
     * Constructs an UTF8Decoder.
     *
     * Replacer template interface implementation must provide the following method:
     *     String replace(const UTF8Char &, bool &, uint32_t &);
     *
     * @param source Source byte stream
     * @param Replacer template interface
     */
    UTF8Decoder(const String &source, Replacer &replacer) : mSource(source), mReplacer(replacer) {
        uint32_t index = 0;
        while (index < mSource.size()) {
            UTF8Char c = decodeCodepoint(mSource, index);
            // don't check for validness of UTF8Char, we need to know if there is an invalid codepoint. Otherwise
            // byte stream and codepoint array will be out-of-sync.
            mCharacters.push_back(c);
            index += c.isValid() ? c.encodedSize() : 1;     // if codepoint is invalid, skip one byte
        }
    }

    /**
     * Applies the replacer on the complete UTF-8 byte stream. Does not modify the source stream.
     * @return Byte stream with applied Replacer
     */
    String replace() {
        uint32_t index = 0, oldIndex;
        uint32_t streamPos = 0;
        String out;

        while (index < mCharacters.size()) {
            // got a multibyte char, extract codepoint
            UTF8Char character = mCharacters[index];

            if (!character.isValid()) {       // error, skip this byte
                out.append(mSource.const_data(streamPos), 1);        // add to output, since we don't want to drop it
                streamPos++;
                index++;
                continue;
            }

            oldIndex = index;
            index++;

            bool replaced = false;
            String replacement = mReplacer.replace(*this, character, replaced, index);

            if (replaced) {         // add replacement if specified
                out += replacement;
                // need to forward stream position UTF8 codepoints' bytes added by replacement
                for (uint32_t i = oldIndex; i < index; i++)
                    streamPos += mCharacters[i].encodedSize();
            } else {
                out.append(mSource.const_data(streamPos), character.encodedSize());     // or add original
                streamPos += character.encodedSize();
            }

        }

        return out;
    }

    /**
     * Decodes the next Unicode codepoint of the source at the given index. The index will be moved forward to next
     * codepoint.
     * @param index Current index. Will be increased so it points to next codepoint.
     * @return The decoded Unicode codepoint.
     */
    UTF8Char nextCodepoint(uint32_t &index) {
        UTF8Char c = mCharacters[index];
        index++;
        return c;
    }

private:
    static UTF8Char decodeCodepoint(const String &source, uint32_t index) {
        // at end of stream
        if (index >= source.size())
            return UTF8Char(0, 0);

        uint8_t achar = *static_cast<const uint8_t*>(source.const_data(index));
        uint32_t codepoint = 0;
        uint8_t additional = 0;

        // UNICODE 2-byte char matching 110xxxxx
        if ((achar & 0xE0) == 0xC0) {
            codepoint = achar & 0x1Fu;
            additional = 1;
        }
        // UNICODE 3-byte char matching 1110xxxxx
        else if ((achar & 0xF0) == 0xE0) {
            codepoint = achar & 0x0Fu;
            additional = 2;
        }
        // UNICODE 4-byte char matching 11110xxx
        else if ((achar & 0xF8) == 0xF0) {
            codepoint = achar & 0x07u;
            additional = 3;
        }
        // default / ordinary ASCII
        else
            return UTF8Char(achar, 1);


        for (int i = 0; i < additional; i++) {
            // at end of stream
            if ((index + i + 1) >= source.size()) {
                return UTF8Char(0, 0);
            }

            uint8_t nchar = *static_cast<const uint8_t*>(source.const_data(index + i + 1));

            // Not matching a continuation byte (10xxxxxx) although it must be.
            // As error handling we discard the previous sequence and treat the byte as default / ordinary ASCII
            if ((nchar & 0xC0) != 0x80) {
                return UTF8Char(achar, 1);
            }

            // Add the Xs of 10xxxxxx to the codepoint. Existing codepoint must be shifted 6 bits to the left to make
            // some room
            codepoint = (codepoint << 6) | (nchar & 0x3F);
        }

        return UTF8Char(codepoint, additional + static_cast<uint8_t>(1));
    }

    const String &mSource;
    Replacer &mReplacer;
    std::vector<UTF8Char> mCharacters;
};

#endif // UTF8DECODER_H
