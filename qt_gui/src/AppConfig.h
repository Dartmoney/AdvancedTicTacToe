//
// Created by imako on 08.01.2026.
//

#ifndef TIKTAKTOE_APPCONFIG_H
#define TIKTAKTOE_APPCONFIG_H
#pragma once

#include <QString>
#include <QStringList>
#include <QCommandLineParser>

#include <engine/Player.h>
#include <engine/RuleSet.h>

struct AppConfig {
    engine::RuleSet rules{};

    bool aiEnabled = false;
    engine::Player aiPlayer = engine::Player::O;
    int aiCandidateRadius = 2;

    int cellSizePx = 40;
    QString configFile;

    QStringList warnings;

    // Reads config from optional --config INI file and then applies CLI overrides.
    static AppConfig fromParser(const QCommandLineParser& parser);
};

#endif //TIKTAKTOE_APPCONFIG_H