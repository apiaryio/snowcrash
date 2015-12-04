//
//  ByteBuffer.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/22/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "ByteBuffer.h"

using namespace mdp;

/* Byte lenght of an UTF8 character (based on first byte) */
#define UTF8_CHAR_LEN( byte ) (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1

/* Number of UTF8 characters in byte buffer */
static size_t strnlen_utf8(const char* s, size_t len)
{
    if (!s || !len)
        return 0;
    
    size_t i = 0, j = 0;
    while (s[i] && i < len) {
        i += UTF8_CHAR_LEN(s[i]);
        j++;
    }
    return j;
}

/* Convert range of bytes to a range of characters */
static CharactersRange BytesRangeToCharactersRange(const BytesRange& bytesRange, const ByteBuffer& byteBuffer)
{
    if (byteBuffer.empty()) {
        return CharactersRange();
    }
    
    BytesRange workRange = bytesRange;
    if (bytesRange.location + bytesRange.length > byteBuffer.length()) {
        // Accomodate maximum possible length
        workRange.length -= bytesRange.location + bytesRange.length - byteBuffer.length();
    }

    size_t charLocation = 0;
    if (bytesRange.location > 0)
        charLocation = strnlen_utf8(byteBuffer.c_str(), bytesRange.location);
    
    size_t charLength = 0;
    if (bytesRange.length > 0)
        charLength = strnlen_utf8(byteBuffer.c_str() + bytesRange.location, bytesRange.length);
    
    CharactersRange characterRange = CharactersRange(charLocation, charLength);
    return characterRange;
}

static CharactersRange BytesRangeToCharactersRange(const BytesRange& bytesRange, const ByteBufferCharacterIndex& index)
{
    if (index.empty()) {
        return CharactersRange();
    }

    BytesRange workRange = bytesRange;
    if (bytesRange.location + bytesRange.length > index.size()) {
        // Accomodate maximum possible length
        workRange.length -= bytesRange.location + bytesRange.length - index.size();
    }

    size_t charLocation = 0;
    if (workRange.location > 0)
        charLocation = index[workRange.location];
    
    size_t charLength = 0;
    if (workRange.length > 0) {
        size_t pos = workRange.location + workRange.length;
        if (pos >= index.size()) {
            // this code branch is there to be compatioble with strlen_utf8()
            charLength = index[index.size()-1];
            charLength -= charLocation - 1;
        }
        else {
            charLength = index[pos];
            charLength -= charLocation;
        }
    }
    
    CharactersRange characterRange = CharactersRange(charLocation, charLength);
    return characterRange;
}

void mdp::BuildCharacterIndex(ByteBufferCharacterIndex& index, const ByteBuffer& byteBuffer) {

    const char* source = byteBuffer.c_str();
    size_t len = byteBuffer.length();
    size_t pos = 0;
    size_t charPos = 0;

    index.resize(byteBuffer.length());

    while (source[pos] && pos < len) {
        int charLen = UTF8_CHAR_LEN(source[pos]);
        pos += charLen;

        while (charLen) {
            index[pos-charLen] = charPos;
            charLen--;
        }

        charPos++;
    }
}

CharactersRangeSet mdp::BytesRangeSetToCharactersRangeSet(const BytesRangeSet& rangeSet, const ByteBuffer& byteBuffer)
{
    CharactersRangeSet characterMap;
    
    for (BytesRangeSet::const_iterator it = rangeSet.begin(); it != rangeSet.end(); ++it) {
        CharactersRange characterRange = BytesRangeToCharactersRange(*it, byteBuffer);
        characterMap.push_back(characterRange);
    }
    
    return characterMap;
}

CharactersRangeSet mdp::BytesRangeSetToCharactersRangeSet(const BytesRangeSet& rangeSet, const ByteBufferCharacterIndex& index)
{
    CharactersRangeSet characterMap;
    
    for (BytesRangeSet::const_iterator it = rangeSet.begin(); it != rangeSet.end(); ++it) {
        CharactersRange characterRange = BytesRangeToCharactersRange(*it, index);
        characterMap.push_back(characterRange);
    }
    
    return characterMap;
}

ByteBuffer mdp::MapBytesRangeSet(const BytesRangeSet& rangeSet, const ByteBuffer& byteBuffer)
{
    if (byteBuffer.empty())
        return ByteBuffer();
    
    size_t length = byteBuffer.length();
    ByteBufferStream s;
    for (BytesRangeSet::const_iterator it = rangeSet.begin(); it != rangeSet.end(); ++it) {
        
        if (it->location + it->length > length) {
            // Sundown adds an extra newline on the source input if needed.
            if (it->location + it->length - length == 1) {
                s << byteBuffer.substr(it->location, length - it->location);
                return s.str();
            }
            else {
                // Wrong map
                return ByteBuffer();
            }
        }
        
        s << byteBuffer.substr(it->location, it->length);
    }
    
    return s.str();
}
