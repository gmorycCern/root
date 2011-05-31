//===--- TerminalDisplayWin.h - Output To Windows Console -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the interface for writing to a Windows console
//  i.e. cmd.exe.
//
//  Axel Naumann <axel@cern.ch>, 2011-05-12
//===----------------------------------------------------------------------===//

#ifdef WIN32
#include "textinput/TerminalDisplayWin.h"

#include "textinput/Color.h"

namespace textinput {
  TerminalDisplayWin::TerminalDisplayWin():
    TerminalDisplay(false), fStartLine(0), fIsAttached(false) {
    HandleResizeEvent();
  }

  TerminalDisplayWin::~TerminalDisplayWin() {}

  void
  TerminalDisplayWin::UpdateHandle() {
    fOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    SetIsTTY(::GetConsoleMode(fOut, &fOldMode) != 0);
    if (!IsTTY()) {
      fOut = CreateFile("CONOUT$", (GENERIC_READ | GENERIC_WRITE),
        (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
      if (fOut == INVALID_HANDLE_VALUE) {
        ShowError("opening CONOUT$");
        fOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
      } else {
        SetIsTTY(::GetConsoleMode(fOut, &fOldMode) != 0);
      }
    }
    fMyMode = fOldMode | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
  }

  void
  TerminalDisplayWin::HandleResizeEvent() {
    UpdateHandle();
    if (IsTTY()) {
      CONSOLE_SCREEN_BUFFER_INFO Info;
      if (!::GetConsoleScreenBufferInfo(fOut, &Info)) {
        ShowError("resize / getting console info");
        return;
      }
      SetWidth(Info.dwSize.X);
    }
  }

  void
  TerminalDisplayWin::SetColor(char CIdx, const Color& C) {
    WORD Attribs = 0;
    // There is no underline since DOS has died.
    if (C.fModifiers & Color::kModUnderline) Attribs |= BACKGROUND_INTENSITY;
    if (C.fModifiers & Color::kModBold) Attribs |= FOREGROUND_INTENSITY;
    if (C.fR > 64) Attribs |= FOREGROUND_RED;
    if (C.fG > 64) Attribs |= FOREGROUND_GREEN;
    if (C.fB > 64) Attribs |= FOREGROUND_BLUE;
    ::SetConsoleTextAttribute(fOut, Attribs);
  }

  void
  TerminalDisplayWin::Move(Pos P) {
    MoveInternal(P);
    fWritePos = P;
  }

  void
  TerminalDisplayWin::MoveInternal(Pos P) {
    COORD C = {P.fCol, P.fLine + fStartLine};
    ::SetConsoleCursorPosition(fOut, C);
  }

  void
  TerminalDisplayWin::MoveFront() {
    Pos P(fWritePos);
    P.fCol = 0;
    MoveInternal(P);
  }

  void
  TerminalDisplayWin::MoveUp(size_t nLines /* = 1 */) {
    Pos P(fWritePos);
    --P.fLine;
    MoveInternal(P);
  }

  void
  TerminalDisplayWin::MoveDown(size_t nLines /* = 1 */) {
    Pos P(fWritePos);
    ++P.fLine;
    MoveInternal(P);
  }

  void
  TerminalDisplayWin::MoveRight(size_t nCols /* = 1 */) {
    Pos P(fWritePos);
    ++P.fCol;
    MoveInternal(P);
  }

  void
  TerminalDisplayWin::MoveLeft(size_t nCols /* = 1 */) {
    Pos P(fWritePos);
    --P.fCol;
    MoveInternal(P);
  }

  void
  TerminalDisplayWin::EraseToRight() {
    DWORD NumWritten;
    COORD C = {fWritePos.fCol, fWritePos.fLine + fStartLine};
    ::FillConsoleOutputCharacter(fOut, ' ', GetWidth() - C.X, C,
      &NumWritten);
    // It wraps, so move up and reset WritePos:
    //MoveUp();
    //++WritePos.Line;
  }

  void
  TerminalDisplayWin::WriteRawString(const char *text, size_t len) {
    DWORD NumWritten = 0;
    if (IsTTY()) {
      WriteConsole(fOut, text, (DWORD) len, &NumWritten, NULL);
    } else {
      WriteFile(fOut, text, (DWORD) len, &NumWritten, NULL);
    }
    if (NumWritten != len) {
      ShowError("writing to output");
    }
  }

  void
  TerminalDisplayWin::Attach() {
    // set to noecho
    if (fIsAttached) return;
    UpdateHandle();
    if (IsTTY() && !::SetConsoleMode(fOut, fMyMode)) {
      ShowError("attaching to console output");
    }
    CONSOLE_SCREEN_BUFFER_INFO Info;
    if (IsTTY()) {
      if (!::GetConsoleScreenBufferInfo(fOut, &Info)) {
        ShowError("attaching / getting console info");
      } else {
        fStartLine = Info.dwCursorPosition.Y;
        if (Info.dwCursorPosition.X) {
          // Whooa - where are we?! Newline and cross fingers:
          WriteRawString("\n", 1);
          ++fStartLine;
        }
      }
    }
    fIsAttached = true;
    NotifyTextChange(Range::AllWithPrompt());
  }

  void
  TerminalDisplayWin::Detach() {
    if (!fIsAttached) return;
    if (IsTTY() && !SetConsoleMode(fOut, fOldMode)) {
      ShowError("detaching to console output");
    }
    TerminalDisplay::Detach();
    fIsAttached = false;
  }

  void
  TerminalDisplayWin::ShowError(const char* Where) const {
    DWORD Err = GetLastError();
    LPVOID MsgBuf = 0;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS, NULL, Err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &MsgBuf, 0, NULL);

    printf("Error %d in textinput::TerminalDisplayWin %s: %s\n", Err, Where, MsgBuf);
    LocalFree(MsgBuf);
  }

}

#endif // WIN32