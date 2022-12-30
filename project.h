#ifndef _PROJECT_H_
#define _PROJECT_H_

#include "parser.h"
#include <iostream>
#include <string>

typedef struct Note {
    std::string VoiceOverlap;
    std::string Flags;
    std::string Envelope;

    std::string PBType;
    std::string PitchBend;
    std::string PBStart;
    std::string PBS;
    std::string PBW;
    std::string PBY;
    std::string PBM;
    std::string VBR;

    std::string Lyric;

    std::string PreUtterance;

    int Velocity;
    int Intensity;
    int Modulation;
    int StartPoint;
    int NoteNum;
    int Length;
    int Tempo;

    bool matched;

    Note()
        : Velocity(100), Intensity(100), Modulation(0), StartPoint(0),
          NoteNum(0), Length(480), Tempo(120), matched(false) {}
} Note;

typedef struct Project {
    std::vector<Note> notes;
    std::string version;
    std::string project_name;
    std::string voice_dir;
    std::string out_file;
    std::string cache_dir;
    std::string tool1;
    std::string tool2;
    std::string global_flags;
    double tempo;
    bool mode2;
    /**
     * @brief 将 Project 转换为 INI_Object。
     *
     * @return INI_Object 不含 Version 的 INI_Object
     */
    INI_Object build() const {
        INI_Object ret;
        size_t id = 0;
        ret["#SETTING"]["Tempo"] = std::to_string(tempo);
        ret["#SETTING"]["Tracks"] = "1";
        ret["#SETTING"]["ProjectName"] = project_name;
        ret["#SETTING"]["VoiceDir"] = voice_dir;
        ret["#SETTING"]["OutFile"] = out_file;
        ret["#SETTING"]["CacheDir"] = cache_dir;
        ret["#SETTING"]["Tool1"] = tool1;
        ret["#SETTING"]["Tool2"] = tool2;
        ret["#SETTING"]["Mode2"] = mode2 ? "True" : "False";
        ret["#SETTING"]["Flags"] = global_flags;

        for (const Note &note : notes) {
            std::string strid = get_id(id);
            ret[strid]["Length"] = std::to_string(note.Length);
            ret[strid]["Lyric"] = note.Lyric;
            ret[strid]["NoteNum"] = std::to_string(note.NoteNum);
            ret[strid]["PreUtterance"] = note.PreUtterance;
            ret[strid]["Velocity"] = std::to_string(note.Velocity);
            ret[strid]["Intensity"] = std::to_string(note.Intensity);
            ret[strid]["Modulation"] = std::to_string(note.Modulation);
            if (!note.PBType.empty())
                ret[strid]["PBType"] = note.PBType;
            if (!note.PitchBend.empty())
                ret[strid]["PitchBend"] = note.PitchBend;
            if (note.StartPoint != 0)
                ret[strid]["StartPoint"] = std::to_string(note.StartPoint);
            if (!note.PBW.empty())
                ret[strid]["PBW"] = note.PBW;
            if (!note.PBS.empty())
                ret[strid]["PBS"] = note.PBS;
            if (!note.VBR.empty())
                ret[strid]["VBR"] = note.VBR;
            if (!note.PBStart.empty())
                ret[strid]["PBStart"] = note.PBStart;
            if (!note.Envelope.empty())
                ret[strid]["Envelope"] = note.Envelope;
            if (!note.PBY.empty())
                ret[strid]["PBY"] = note.PBY;
            if (note.Tempo != tempo)
                ret[strid]["Tempo"] = std::to_string(note.Tempo);
            id++;
        }
        return ret;
    }
    /**
     * @brief 将 Project 转换为 std::string
     *
     * @return std::string INI 格式的字符串
     */
    std::string to_string() const {
        std::string raw = ini_encode(build());
        return "[#VERSION]\n" + version + "\n" + raw + "\n[#TRACKEND]\n";
    }
    Project() : tempo(120.0f), mode2(true) {}

  private:
    /**
     * @brief 获得 id 对应的 Track
     *
     * @param id 正整数 id
     * @return std::string Track
     */
    static std::string get_id(size_t id) noexcept {
        std::string tmp = std::to_string(id);
        while (tmp.length() < 4)
            tmp.insert(0, "0");
        return "#" + tmp;
    }
} Project;
/**
 * @brief 实用工具。用于 std::map 默认值。
 *
 * @tparam Key 键的类型。
 * @tparam Value 值的类型。
 * @param m std::map 对象。
 * @param key 键。
 * @param def 不存在时的替代值。默认为默认构造参数。
 * @return Value 存在时返回值，否则返回替代值。
 */
template <typename Key, typename Value>
Value defaultval(const std::map<Key, Value> &m, const Key &key,
                 const Value &def = Value()) {
    try {
        return m.at(key);
    } catch (...) {
        return def;
    }
}
/**
 * @brief 将 INI_Object 转换为 Project 实例。
 *
 * @param obj INI_Object INI 对象
 * @return Project Project 实例。
 * @throw 任意的 std::exception 当解析发生错误
 */
Project parse(const INI_Object &obj) {
    Project tmp{};
    for (auto &&i : obj) {
        if (i.first == "#VERSION") {
            for (auto &&j : i.second) {
                tmp.version = j.first; // 因为 VERSION 是 key
            }
        } else if (i.first == "#SETTING") {
            tmp.tempo = std::stod(i.second.at("Tempo"));
            tmp.project_name = i.second.at("ProjectName");
            tmp.voice_dir = i.second.at("VoiceDir");
            tmp.cache_dir = i.second.at("CacheDir");
            tmp.out_file = i.second.at("OutFile");
            tmp.tool1 = i.second.at("Tool1");
            tmp.tool2 = i.second.at("Tool2");
            tmp.mode2 = (i.second.at("Mode2") == "True");
            tmp.global_flags = defaultval(i.second, std::string("Flags")); // ??
        } else if (i.first.length() > 0 && i.first[0] == '#') {
            size_t sz = std::stoi(i.first.substr(1));
            tmp.notes.resize(sz + 1);
            tmp.notes[sz].Length = std::stoi(i.second.at("Length"));
            tmp.notes[sz].Lyric = i.second.at("Lyric");
            tmp.notes[sz].NoteNum = std::stoi(i.second.at("NoteNum"));
            tmp.notes[sz].PreUtterance =
                defaultval(i.second, std::string("PreUtterance"));

            tmp.notes[sz].Velocity = std::stoi(defaultval(
                i.second, std::string("Velocity"), std::string("100")));
            tmp.notes[sz].Intensity =
                std::stoi(defaultval(i.second, std::string("Intensity"),
                                     std::string("100"))); // 这里
            tmp.notes[sz].Modulation = std::stoi(defaultval(
                i.second, std::string("Modulation"), std::string("0")));
            tmp.notes[sz].StartPoint = std::stoi(defaultval(
                i.second, std::string("StartPoint"), std::string("0")));
            tmp.notes[sz].Tempo = std::stoi(defaultval(
                i.second, std::string("Tempo"), std::to_string(tmp.tempo)));
            tmp.notes[sz].VoiceOverlap =
                defaultval(i.second, std::string("VoiceOverlap"));
            tmp.notes[sz].Flags = defaultval(i.second, std::string("Flags"));
            tmp.notes[sz].Envelope =
                defaultval(i.second, std::string("Envelope"));

            tmp.notes[sz].PBType = defaultval(i.second, std::string("PBType"));
            tmp.notes[sz].PitchBend =
                defaultval(i.second, std::string("PitchBend"));
            tmp.notes[sz].PBStart =
                defaultval(i.second, std::string("PBStart"));
            tmp.notes[sz].PBS = defaultval(i.second, std::string("PBS"));
            tmp.notes[sz].PBW = defaultval(i.second, std::string("PBW"));
            tmp.notes[sz].PBY = defaultval(i.second, std::string("PBY"));
            tmp.notes[sz].PBM = defaultval(i.second, std::string("PBM"));
            tmp.notes[sz].VBR = defaultval(i.second, std::string("VBR"));
        }
    }

    return tmp;
}

#endif