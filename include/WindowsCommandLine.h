// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <string>

// Quote an individual Windows argv element, including trailing backslashes and
// literal quotes, according to the C/C++ runtime command-line parsing rules.
inline std::wstring QuoteWindowsArgument(const std::wstring& argument) {
    std::wstring result = L"\"";
    size_t backslashes = 0;
    for (wchar_t character : argument) {
        if (character == L'\\') {
            ++backslashes;
            continue;
        }
        result.append(backslashes * (character == L'"' ? 2 : 1), L'\\');
        backslashes = 0;
        if (character == L'"') result += L'\\';
        result += character;
    }
    result.append(backslashes * 2, L'\\');
    result += L'"';
    return result;
}
