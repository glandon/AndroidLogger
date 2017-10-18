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
#include "Plugin.h"
#include "LogLexer.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "DebugTrace.h"

namespace LoggerLexer {
//
// ILexer
//

ILexer* LogLexer::LexerFactory() {
    return new LogLexer();
}


LogLexer::LogLexer() {
    //ctor
}

LogLexer::~LogLexer() {
    //dtor
}

// ILexer
void SCI_METHOD LogLexer::Release() {
    delete this;
}

int SCI_METHOD LogLexer::Version() const {
    return lvOriginal;
}

const char* SCI_METHOD LogLexer::PropertyNames() {
    return "";
}

int SCI_METHOD LogLexer::PropertyType(const char* name) {
    return SC_TYPE_BOOLEAN;
}

const char* SCI_METHOD LogLexer::DescribeProperty(const char* name) {
    return "";
}

int SCI_METHOD LogLexer::PropertySet(const char* key, const char* val) {
    return -1;
}

const char* SCI_METHOD LogLexer::DescribeWordListSets() {
    return "";
}

int SCI_METHOD LogLexer::WordListSet(int n, const char* wl) {
    WordList *wordListN = nullptr;

    switch (n) {
    case LOG_V:
        wordListN = &keywords_v;
        break;
    case LOG_D:
        wordListN = &keywords_d;
        break;
    case LOG_I:
        wordListN = &keywords_i;
        break;
    case LOG_W:
        wordListN = &keywords_w;
        break;
    case LOG_E:
        wordListN = &keywords_e;
        break;
    case LOG_USER_LEVEL:
        wordListN = &keywords_user_level;
        break;
    case LOG_USER_TAG1:
        wordListN = &keywords_user_tag1;
        break;
    case LOG_USER_TAG2:
        wordListN = &keywords_user_tag2;
        break;
    case LOG_USER_TAG3:
        wordListN = &keywords_user_tag3;
        break;
    default:
        break;
    }

    int firstModification = -1;

    if ( wordListN ) {
        WordList wlNew;
        wlNew.Set(wl);
        if (*wordListN != wlNew) {
            wordListN->Set(wl);
            firstModification = 0;
        }
    }
    return firstModification;
}

void SCI_METHOD LogLexer::Lex(unsigned int startPos, int length, int initStyle, IDocument* pAccess)
{
    //DebugTrace(TEXT("LogLexer: Lex() - startPos=%d\r\n"), startPos);
    LexAccessor styler(pAccess);
    styler.StartAt(startPos);
    styler.StartSegment(startPos);
    
    int line = styler.GetLine(startPos);
    int lineStart = styler.LineStart(line);
    int lineEnd = styler.LineEnd(line);
    
    int state = LOG_I;
    int curPos = startPos;
    length += startPos;
    
    while( curPos < length )
    {
        state = COLOR_VERBOSE;

        for ( int curLinePos = lineStart; curLinePos < lineEnd; curLinePos++ ) {
            char ch[2] = {0};
            ch[0]= styler.SafeGetCharAt(curLinePos, '\0');

            if ( ch[0] == '(') {
                state = -1;
                break;
            }

            if ( keywords_v.InList(ch) ) {
                state = LOG_V;
            } else if ( keywords_d.InList(ch) ) {
                state = LOG_D;
            } else if ( keywords_i.InList(ch) ) {
                state = LOG_I;
            } else if ( keywords_w.InList(ch) ) {
                state = LOG_W;
            } else if ( keywords_e.InList(ch) ) {
                state = LOG_E;
            } else if ( keywords_user_level.InList(ch) ) {
                state = LOG_USER_LEVEL;
            } else {
                state = -1;
            }

            if ( state != -1 ) 
            {
                char words[64] = {0};

                for ( int i = 0, j=0; i < 63; j++ ) {
                    char c = styler.SafeGetCharAt(curLinePos + 2 + j, '\0');

                    if (!isspace(c) && c != '(' && c != ':' ) {
                        words[i++] = c;
                    } else if ( i == 0 ) {
                        continue;
                    } else {
                        words[i] = '\0';
                        break;
                    }   
                }

                if ( keywords_user_tag1.InList(words) ) {
                    state = LOG_USER_TAG1;
                } else if ( keywords_user_tag2.InList(words) ) {
                    state = LOG_USER_TAG2;
                } else if ( keywords_user_tag3.InList(words) ) {
                    state = LOG_USER_TAG3;
                }
                break;
            }
        }

        if ( state == -1 ) {
            state = LOG_V;
        }

        for(int j = lineStart; j < lineEnd; j++) {
            if ( state == LOG_E ) {
                styler.ColourTo(j, COLOR_ERROR);
            } else if ( state == LOG_W ) {
                styler.ColourTo(j, COLOR_WARNING);
            } else if ( state == LOG_I ) {
                styler.ColourTo(j, COLOR_INFO);
            } else if ( state == LOG_D )  {
                styler.ColourTo(j, COLOR_DEBUG);
            }  else if ( state == LOG_V )  {
                styler.ColourTo(j, COLOR_VERBOSE);
            } else if ( state == LOG_USER_LEVEL ) {
                styler.ColourTo(j, COLOR_USER_LEVEL);
            } else if ( state == LOG_USER_TAG1 ) {
                styler.ColourTo(j, COLOR_USER_TAG1);
            } else if ( state == LOG_USER_TAG2 ) {
                styler.ColourTo(j, COLOR_USER_TAG2);
            } else if ( state == LOG_USER_TAG3 ) {
                styler.ColourTo(j, COLOR_USER_TAG3);
            } else {
                styler.ColourTo(j, COLOR_VERBOSE);
            }
        }

        // The last line of log have none \r\n
        if ( lineEnd == styler.Length() ) {
            break;
        }

        // Also update lienEnd
        char ch = styler.SafeGetCharAt(lineEnd, '\0');
        char chNext = styler.SafeGetCharAt(lineEnd+1, '\0');

        if (ch == '\r' || ch == '\n') {
            styler.ColourTo(lineEnd, COLOR_VERBOSE);
            state = LOG_V;
        }

        if (chNext == '\r' || chNext == '\n') {
            styler.ColourTo(lineEnd+1, COLOR_VERBOSE);
            lineEnd++;
            state = LOG_V;
        }

        lineStart = lineEnd+1;
        lineEnd = styler.LineEnd(++line);
        curPos = lineStart;
    }

    styler.Flush();
}

void SCI_METHOD LogLexer::Fold(unsigned int startPos, int length, int initStyle, IDocument* pAccess) {
    return;
}

void* SCI_METHOD LogLexer::PrivateCall(int operation, void* pointer) {
    return nullptr;
}


void LogLexer::getLine(Accessor styler, TCHAR* buffer, int start, int end) {
    TCHAR *pBuffer = buffer;
    int len = end - start;
    while ( start <= end ) {
        *pBuffer++ = styler.SafeGetCharAt(start);
        start++;
    }
    buffer[len] = 0;
}

//
// Scintilla exports
//

int EXT_LEXER_DECL GetLexerCount() {
    return 1;
}

void EXT_LEXER_DECL GetLexerName(unsigned int index, char* name, int buflength) {
    strncpy_s(name, buflength, PLUGIN_NAME, strlen(PLUGIN_NAME));
    name[buflength - 1] = '\0';
}

void SCI_METHOD GetLexerStatusText(unsigned int index, WCHAR* desc, int buflength)
{
    wcsncpy_s(desc, buflength, PLUGIN_NAME_UNICODE, wcslen(PLUGIN_NAME_UNICODE));
    desc[buflength - 1] = L'\0';
}

LexerFactoryFunction EXT_LEXER_DECL GetLexerFactory(unsigned int index)
{
    return (index == 0) ? LogLexer::LexerFactory : nullptr;
}

} // namespace LogLexer