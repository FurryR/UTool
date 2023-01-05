#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <iostream>
#include <string>

#include "parser.h"
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
    std::string Label;

    std::string Lyric;

    std::string PreUtterance;

    int Velocity;
    int Intensity;
    int Modulation;
    int StartPoint;
    int NoteNum;
    int Length;
    int Tempo;

    Note()
        : Velocity(100),
          Intensity(100),
          Modulation(0),
          StartPoint(0),
          NoteNum(0),
          Length(480),
          Tempo(120) {}
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
    INI::Object build() const {
        INI::Object ret;
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
            if (!note.Flags.empty()) ret[strid]["Flags"] = note.Flags;
            if (!note.PBType.empty()) ret[strid]["PBType"] = note.PBType;
            if (!note.PitchBend.empty())
                ret[strid]["PitchBend"] = note.PitchBend;
            if (note.StartPoint != 0)
                ret[strid]["StartPoint"] = std::to_string(note.StartPoint);
            if (!note.PBW.empty()) ret[strid]["PBW"] = note.PBW;
            if (!note.PBS.empty()) ret[strid]["PBS"] = note.PBS;
            if (!note.VBR.empty()) ret[strid]["VBR"] = note.VBR;
            if (!note.PBStart.empty()) ret[strid]["PBStart"] = note.PBStart;
            if (!note.Envelope.empty()) ret[strid]["Envelope"] = note.Envelope;
            if (!note.PBY.empty()) ret[strid]["PBY"] = note.PBY;
            if (!note.Label.empty()) ret[strid]["Label"] = note.Label;
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
        return "[#VERSION]\n" + version + "\n" + INI::stringify(build()) +
               "\n[#TRACKEND]\n";
    }
    /**
     * @brief 将 INI_Object 转换为 Project 实例。
     *
     * @param obj INI_Object INI 对象
     * @return Project Project 实例。
     * @throw 任意的 std::exception 当解析发生错误
     */
    static Project parse(const INI::Object &obj) {
        Project tmp{};
        for (auto &&j : obj.at("#VERSION")) {
            tmp.version = j.first;  // 因为 VERSION 是 key
        }
        tmp.tempo = std::stod(obj.at("#SETTING").at("Tempo"));
        tmp.project_name = obj.at("#SETTING").at("ProjectName");
        tmp.voice_dir = obj.at("#SETTING").at("VoiceDir");
        tmp.cache_dir = obj.at("#SETTING").at("CacheDir");
        tmp.out_file = obj.at("#SETTING").at("OutFile");
        tmp.tool1 = obj.at("#SETTING").at("Tool1");
        tmp.tool2 = obj.at("#SETTING").at("Tool2");
        tmp.mode2 = (obj.at("#SETTING").at("Mode2") == "True");
        tmp.global_flags =
            defaultval(obj.at("#SETTING"), std::string("Flags"));  // ??
        for (auto &&i : obj) {
            if (i.first.length() > 0 && i.first[0] == '#' &&
                i.first != "#TRACKEND" && i.first != "#VERSION" &&
                i.first != "#SETTING") {
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
                                         std::string("100")));  // 这里
                tmp.notes[sz].Modulation = std::stoi(defaultval(
                    i.second, std::string("Modulation"), std::string("0")));
                tmp.notes[sz].StartPoint = std::stoi(defaultval(
                    i.second, std::string("StartPoint"), std::string("0")));
                tmp.notes[sz].Tempo = std::stoi(defaultval(
                    i.second, std::string("Tempo"), std::to_string(tmp.tempo)));
                tmp.notes[sz].VoiceOverlap =
                    defaultval(i.second, std::string("VoiceOverlap"));
                tmp.notes[sz].Flags =
                    defaultval(i.second, std::string("Flags"));
                tmp.notes[sz].Envelope =
                    defaultval(i.second, std::string("Envelope"));

                tmp.notes[sz].PBType =
                    defaultval(i.second, std::string("PBType"));
                tmp.notes[sz].PitchBend =
                    defaultval(i.second, std::string("PitchBend"));
                tmp.notes[sz].PBStart =
                    defaultval(i.second, std::string("PBStart"));
                tmp.notes[sz].PBS = defaultval(i.second, std::string("PBS"));
                tmp.notes[sz].PBW = defaultval(i.second, std::string("PBW"));
                tmp.notes[sz].PBY = defaultval(i.second, std::string("PBY"));
                tmp.notes[sz].PBM = defaultval(i.second, std::string("PBM"));
                tmp.notes[sz].VBR = defaultval(i.second, std::string("VBR"));
                tmp.notes[sz].Label =
                    defaultval(i.second, std::string("Label"));
            }
        }
        return tmp;
    }
    Project() : project_name("New Project"), tempo(120.0f), mode2(true) {}

   private:
    /**
     * @brief 获得 id 对应的 Track
     *
     * @param id 正整数 id
     * @return std::string Track
     */
    static std::string get_id(size_t id) noexcept {
        std::string tmp = std::to_string(id);
        while (tmp.length() < 4) tmp.insert(0, "0");
        return "#" + tmp;
    }
} Project;
#endif