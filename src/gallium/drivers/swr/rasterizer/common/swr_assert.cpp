/****************************************************************************
* Copyright (C) 2014-2015 Intel Corporation.   All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the next
* paragraph) shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
****************************************************************************/

#include "common/os.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#if defined(SWR_ENABLE_ASSERTS)

#if defined(_WIN32)
#pragma comment(lib, "user32.lib")
#endif // _WIN32

bool SwrAssert(
    bool&       enabled,
    const char* pExpression,
    const char* pFileName,
    uint32_t    lineNum,
    const char* pFmtString /* = nullptr */,
    ...)
{
    if (!enabled) return false;

#if defined(_WIN32)
    static const int MAX_MESSAGE_LEN = 2048;
    char msgBuf[MAX_MESSAGE_LEN];

    sprintf_s(msgBuf, "%s(%d): assert: %s\n", pFileName, lineNum, pExpression);
    msgBuf[MAX_MESSAGE_LEN - 2] = '\n';
    msgBuf[MAX_MESSAGE_LEN - 1] = 0;
    OutputDebugStringA(msgBuf);

    int offset = 0;

    if (pFmtString)
    {
        va_list args;
        va_start(args, pFmtString);
        offset = _vsnprintf_s(
            msgBuf,
            sizeof(msgBuf),
            sizeof(msgBuf),
            pFmtString,
            args);
        va_end(args);

        if (offset < 0) { return true; }

        OutputDebugStringA("\t");
        OutputDebugStringA(msgBuf);
        OutputDebugStringA("\n");
    }

    if (KNOB_ENABLE_ASSERT_DIALOGS)
    {
        int retval = sprintf_s(
            &msgBuf[offset],
            MAX_MESSAGE_LEN - offset,
            "\n\n"
            "File: %s\n"
            "Line: %d\n"
            "\n"
            "Expression: %s\n\n"
            "Cancel: Disable this assert for the remainder of the process\n"
            "Try Again: Break into the debugger\n"
            "Continue: Continue execution (but leave assert enabled)",
            pFileName,
            lineNum,
            pExpression);

        if (retval < 0) { return true; }

        offset += retval;

        if (!IsDebuggerPresent())
        {
            sprintf_s(
                &msgBuf[offset],
                MAX_MESSAGE_LEN - offset,
                "\n\n*** NO DEBUGGER DETECTED ***\n\nPressing \"Try Again\" will cause a program crash!");
        }

        retval = MessageBoxA(nullptr, msgBuf, "Assert Failed", MB_CANCELTRYCONTINUE | MB_ICONEXCLAMATION);

        switch (retval)
        {
        case IDCANCEL:
            enabled = false;
            return false;

        case IDTRYAGAIN:
            return true;

        case IDCONTINUE:
            return false;
        }
    }
    else
    {
        return 0 != IsDebuggerPresent();
    }

#else // !_WIN32
    fprintf(stderr, "%s(%d): assert: %s\n", pFileName, lineNum, pExpression);
    if (pFmtString)
    {
        va_list args;
        va_start(args, pFmtString);
        vfprintf(stderr, pFmtString, args);
        va_end(args);
    }
    fflush(stderr);

    /// @todo - Implement message box on non-Windows platforms

#endif
    return true;
}

#endif // SWR_ENABLE_ASSERTS
