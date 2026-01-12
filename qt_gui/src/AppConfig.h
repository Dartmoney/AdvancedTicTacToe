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

    static AppConfig fromParser(const QCommandLineParser& parser);
};

#endif