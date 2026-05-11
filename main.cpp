#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

// Создаём простую иконку программно (круг),
// чтобы не зависеть от внешнего файла
QIcon createIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#5865F2")); // синий цвет
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(4, 4, 56, 56);
    painter.end();

    return QIcon(pixmap);
}

// Запуск приложения по команде
void launchApp(const QString &command) {
    QProcess::startDetached(command, {});
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Без этого приложение закроется когда закроешь последнее окно
    app.setQuitOnLastWindowClosed(false);

    // Создаём иконку в трее
    QSystemTrayIcon trayIcon(createIcon());
    trayIcon.setToolTip("My Launcher");

    // ─── Меню ───────────────────────────────────────────────
    QMenu menu;

    // Секция: Браузеры
    QAction *titleBrowsers = menu.addAction("── Браузеры ──");
    titleBrowsers->setEnabled(false); // заголовок, не кликабельный

    QAction *firefox   = menu.addAction("🦊  Firefox");
    QAction *chromium  = menu.addAction("🌐  Chromium");

    menu.addSeparator();

    // Секция: Терминал и редакторы
    QAction *titleDev = menu.addAction("── Разработка ──");
    titleDev->setEnabled(false);

    QAction *terminal  = menu.addAction("🖥️  Терминал");
    QAction *vscode    = menu.addAction("💻  VS Code");
    QAction *nvim      = menu.addAction("📝  Neovim");

    menu.addSeparator();

    // Секция: Система
    QAction *titleSys = menu.addAction("── Система ──");
    titleSys->setEnabled(false);

    QAction *files     = menu.addAction("📁  Файловый менеджер");
    QAction *settings  = menu.addAction("⚙️  Настройки системы");

    menu.addSeparator();

    QAction *quitAction = menu.addAction("❌  Выйти");

    // ─── Подключаем команды ─────────────────────────────────
    // Измени команды под свою систему!
    QObject::connect(firefox,   &QAction::triggered, [](){ launchApp("firefox"); });
    QObject::connect(chromium,  &QAction::triggered, [](){ launchApp("chromium-browser"); });
    QObject::connect(terminal,  &QAction::triggered, [](){ launchApp("bash -c 'x-terminal-emulator'"); });
    QObject::connect(vscode,    &QAction::triggered, [](){ launchApp("code"); });
    QObject::connect(nvim,      &QAction::triggered, [](){ launchApp("bash -c 'x-terminal-emulator -e nvim'"); });
    QObject::connect(files,     &QAction::triggered, [](){ launchApp("xdg-open ~"); });
    QObject::connect(settings,  &QAction::triggered, [](){ launchApp("gnome-control-center"); });

    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

    // ─── Показываем меню по клику на иконку ─────────────────
    trayIcon.setContextMenu(&menu);

    // Левый клик тоже открывает меню
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated,
        [&](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
                trayIcon.contextMenu()->popup(QCursor::pos());
            }
        });

    trayIcon.show();

    return app.exec();
}
