#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QMessageBox>

#include "AppConfig.h"
#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Advanced Tic-Tac-Toe");
    QCoreApplication::setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Advanced Tic-Tac-Toe (engine + Qt6 GUI)");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption(QCommandLineOption({"c", "config"}, "Path to INI config file.", "file"));

    parser.addOption(QCommandLineOption("topology", "Board topology: finite|infinite.", "finite|infinite"));
    parser.addOption(QCommandLineOption("width", "Board width for finite topology.", "int"));
    parser.addOption(QCommandLineOption("height", "Board height for finite topology.", "int"));
    parser.addOption(QCommandLineOption("N", "Line length N.", "int"));

    parser.addOption(QCommandLineOption("classic", "Enable classic first-to-N mode."));
    parser.addOption(QCommandLineOption("no-classic", "Disable classic mode."));

    parser.addOption(QCommandLineOption("maximize-lines", "Enable 'maximize number of lines' objective."));
    parser.addOption(QCommandLineOption("no-maximize-lines", "Disable maximize-lines objective."));

    parser.addOption(QCommandLineOption("weights", "Enable weights/scoring."));
    parser.addOption(QCommandLineOption("no-weights", "Disable weights/scoring."));

    parser.addOption(QCommandLineOption("move-costs", "Enable move costs."));
    parser.addOption(QCommandLineOption("no-move-costs", "Disable move costs."));

    parser.addOption(QCommandLineOption("count-subsegments", "Count subsegments of length N inside longer runs (only for AtLeastN)."));
    parser.addOption(QCommandLineOption("no-count-subsegments", "Do not count subsegments."));

    parser.addOption(QCommandLineOption("line-mode", "How to treat run length: exact|atleast.", "exact|atleast"));
    parser.addOption(QCommandLineOption("target-score", "Target score to instantly win (requires weights).", "int"));
    parser.addOption(QCommandLineOption("max-moves", "Move limit (0=unlimited).", "int"));

    parser.addOption(QCommandLineOption("cost-mode", "Move cost mode: score|budget.", "score|budget"));
    parser.addOption(QCommandLineOption("budget", "Initial budget per player (cost-mode=budget). Use -1 for unlimited.", "int"));

    parser.addOption(QCommandLineOption("ai", "AI mode: none|X|O (which side is controlled by AI).", "none|X|O"));
    parser.addOption(QCommandLineOption("ai-radius", "AI search radius around existing moves (default 2).", "int"));

    parser.addOption(QCommandLineOption("cell-size", "Cell size in pixels (default 40).", "int"));

    parser.process(app);

    AppConfig cfg = AppConfig::fromParser(parser);

    MainWindow w(cfg);
    w.resize(1200, 800);
    w.show();

    if (!cfg.warnings.isEmpty()) {
        QMessageBox::information(&w, "Config warnings", cfg.warnings.join("\n"));
    }

    return app.exec();
}
