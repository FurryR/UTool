#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <windows.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "awacorn/awacorn.h"
#include "awacorn/promise.h"

namespace Audio {

HMIDIOUT outHandle;
bool channel0 = true;

bool init() {
    return !midiOutOpen(&outHandle, (UINT)-1, 0, 0, CALLBACK_WINDOW);
}

void close() { midiOutClose(outHandle); }

void panic() { midiOutShortMsg(outHandle, 0x000000FC); }

template <typename Rep, typename Period>
Awacorn::AsyncFn<std::pair<const Awacorn::Event *, Promise::Promise<void>>>
play_note(const std::string &lyric, size_t note_num,
          const std::chrono::duration<Rep, Period> &tm) {
    return [lyric, note_num, tm](Awacorn::EventLoop *ev) {
        Promise::Promise<void> pm;
        const Awacorn::Event *task;
        if (lyric == "R" || lyric == " " || lyric == "") {
            task = ev->create(
                [pm](Awacorn::EventLoop *, const Awacorn::Event *) -> void {
                    pm.resolve();
                },
                tm);
        } else {
            std::stringstream _begin;
            unsigned int begin_int, end_int;
            _begin << std::hex << note_num;  // int decimal_value
            begin_int =
                std::stoul("0x0070" + _begin.str() + (channel0 ? "90" : "91"),
                           nullptr, 16);
            end_int =
                std::stoul("0x0000" + _begin.str() + (channel0 ? "90" : "91"),
                           nullptr, 16);
            channel0 = !channel0;
            midiOutShortMsg(outHandle, begin_int);
            task = ev->create(
                [pm, end_int](Awacorn::EventLoop *,
                              const Awacorn::Event *) -> void {
                    midiOutShortMsg(outHandle, end_int);
                    pm.resolve();
                },
                tm);
        }
        return std::make_pair(task, pm);
    };
}
}  // namespace Audio

#endif