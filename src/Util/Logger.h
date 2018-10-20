//
// Created by Amrik Sadhra on 20/10/2018.
//

#pragma once

// Windows Specific console handles for colour
#ifdef _WIN32

#include <windows.h>

#endif

#include <iostream>
#include <cmath>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include "../Config.h"

static std::string FormatLog(const g3::LogMessage &msg) {
    const int levelFileLineWidth = 40;

    std::string timestamp(msg.timestamp().substr(11, 8)); // Trim microseconds and date from timestamp
    std::string variableWidthMessage(msg.level() + " [" + msg.file() + "->" + msg.function() + ":" + msg.line() + "]: ");
    variableWidthMessage.append(levelFileLineWidth > variableWidthMessage.length() ? (levelFileLineWidth - variableWidthMessage.length()) : 0, ' '); // Pad variable width element to fixed size

    return timestamp + " " + variableWidthMessage;
}

struct AppLog {
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets;        // Index to lines offset
    bool                ScrollToBottom;

    void    Clear()     { Buf.clear(); LineOffsets.clear(); }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size);
        ScrollToBottom = true;
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        ImGui::SetNextWindowSize(ImVec2(500,400), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (copy) ImGui::LogToClipboard();

        if (Filter.IsActive())
        {
            const char* buf_begin = Buf.begin();
            const char* line = buf_begin;
            for (int line_no = 0; line != NULL; line_no++)
            {
                const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
                if (Filter.PassFilter(line, line_end))
                    ImGui::TextUnformatted(line, line_end);
                line = line_end && line_end[1] ? line_end + 1 : NULL;
            }
        }
        else
        {
            ImGui::TextUnformatted(Buf.begin());
        }

        if (ScrollToBottom)
            ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;
        ImGui::EndChild();
        ImGui::End();
    }
};

struct OnScreenLogSink {
    AppLog *log;

    explicit OnScreenLogSink(AppLog *targetLog) {
        log = targetLog;
    };

    void ReceiveLogMessage(g3::LogMessageMover logEntry) {
        auto level = logEntry.get()._level;
        log->AddLog(logEntry.get().toString(&FormatLog).c_str(), nullptr);
    }
};

struct ColorCoutSink {
#ifdef _WIN32
    enum FG_Color {
        BLACK = 0, BLUE = 1, GREEN = 2, RED = 4, YELLOW = 6, WHITE = 7
    };
#else
    enum FG_Color {
        YELLOW = 33, RED = 31, GREEN = 32, WHITE = 97
    };
#endif

    FG_Color GetColor(const LEVELS level) const {
        if (level.value == WARNING.value) { return YELLOW; }
        if (level.value == DEBUG.value) { return GREEN; }
        if (g3::internal::wasFatal(level)) { return RED; }

        return WHITE;
    }

    void ReceiveLogMessage(g3::LogMessageMover logEntry) {
        auto level = logEntry.get()._level;
        auto color = GetColor(level);
#ifdef _WIN32
        HANDLE consoleHandle_;
        consoleHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(consoleHandle_, color);
        std::cout << logEntry.get().toString(&FormatLog);
        SetConsoleTextAttribute(consoleHandle_, WHITE);
#else
        std::cout << "\033[" << color << "m" << logEntry.get().toString(&FormatLog) << "\033[m";
#endif
    }
};

class Logger {
public:
    AppLog onScreenLog;

    explicit Logger();
};


