/*
  AndroidLogger: Android Logger plugin for Notepad++
  Copyright (C) 2015 Simbaba at Najing <zhaoxi.du@gmail.com>

  ******************************************************
  Thanks for GedcomLexer & NppFtp plugin source code.
  ******************************************************
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/
#ifndef LOG_LEXER_H
#define LOG_LEXER_H

#include "Scintilla.h"
#include "ILexer.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "LexerModule.h"
#include "StyleContext.h"
#include "CharacterSet.h"

namespace LoggerLexer {

#undef EXT_LEXER_DECL
#define EXT_LEXER_DECL __declspec( dllexport ) __stdcall

class LogLexer : public ILexer
{
public:
    LogLexer();
    virtual ~LogLexer();

    static ILexer* LexerFactory();

    // ILexer
    void SCI_METHOD Release();
    int SCI_METHOD Version() const;
    const char* SCI_METHOD PropertyNames();
    int SCI_METHOD PropertyType(const char* name);
    const char* SCI_METHOD DescribeProperty(const char* name);
    int SCI_METHOD PropertySet(const char* key, const char* val);
    const char* SCI_METHOD DescribeWordListSets();
    int SCI_METHOD WordListSet(int n, const char* wl);
    void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument* pAccess);
    //void SCI_METHOD Lex2(unsigned int startPos, int length, int initStyle, IDocument* pAccess);
    void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument* pAccess);
    void* SCI_METHOD PrivateCall(int operation, void* pointer);

private:
    void getLine(Accessor styler, TCHAR* buffer, int start, int end);

private:
    enum LogState
    {
        LOG_V,
        LOG_D,
        LOG_I,
        LOG_W,
        LOG_E,
        LOG_USER_LEVEL,
        LOG_USER_TAG1,
        LOG_USER_TAG2,
        LOG_USER_TAG3, // crash, abt etc.
    };

    enum LogColor
    {
        COLOR_VERBOSE,
        COLOR_DEBUG,
        COLOR_INFO,
        COLOR_WARNING,
        COLOR_ERROR,
        COLOR_USER_LEVEL,
        COLOR_USER_TAG1,
        COLOR_USER_TAG2,
        COLOR_USER_TAG3,
    };

    WordList keywords_v;
    WordList keywords_d;
    WordList keywords_i;
    WordList keywords_w;
    WordList keywords_e;
    WordList keywords_user_level;
    WordList keywords_user_tag1;
    WordList keywords_user_tag2;
    WordList keywords_user_tag3;
};

}

#endif // LOG_LEXER_H
