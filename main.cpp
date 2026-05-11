#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QWidget>
#include <QMouseEvent>
#include <QScreen>

// ─── Создаём иконку (синий круг) ────────────────────────────
QIcon createIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#5865F2"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(4, 4, 56, 56);
    painter.end();
    return QIcon(pixmap);
}

QPixmap createPixmap(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#5865F2"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(2, 2, size - 4, size - 4);
    painter.end();
    return pixmap;
}

void launchApp(const QString &command) {
    QProcess::startDetached("bash", {"-c", command});
}

// ─── Плавающая иконка на экране ─────────────────────────────
class FloatingIcon : public QWidget {
public:
    FloatingIcon(QMenu *menu, QWidget *parent = nullptr)
        : QWidget(parent), m_menu(menu)
    {
        // Без рамки, поверх всех окон, не в таскбаре
        setWindowFlags(
            Qt::FramelessWindowHint |
            Qt::WindowStaysOnTopHint |
            Qt::Tool
        );
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(64, 64);

        // Стартовая позиция — правый нижний угол экрана
        QScreen *screen = QApplication::primaryScreen();
        QRect geo = screen->availableGeometry();
        move(geo.right() - 80, geo.bottom() - 80);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Тень
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 60));
        painter.drawEllipse(6, 6, 56, 56);

        // Сама иконка
        painter.drawPixmap(0, 0, createPixmap(64));
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
            m_dragging = false;
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton) {
            move(event->globalPosition().toPoint() - m_dragPos);
            m_dragging = true;
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && !m_dragging) {
            QPoint pos = mapToGlobal(QPoint(0, 0));
            m_menu->popup(pos);
        }
        m_dragging = false;
    }

private:
    QMenu  *m_menu;
    QPoint  m_dragPos;
    bool    m_dragging = false;
};

// ────────────────────────────────────────────────────────────
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // ─── Меню ───────────────────────────────────────────────
    QMenu menu;

    QAction *titleBrowsers = menu.addAction("── Браузеры ──");
    titleBrowsers->setEnabled(false);
    QAction *firefox  = menu.addAction("🦊  Firefox");
    QAction *chromium = menu.addAction("🌐  Chromium");

    menu.addSeparator();

    QAction *titleDev = menu.addAction("── Разработка ──");
    titleDev->setEnabled(false);
    QAction *terminal = menu.addAction("🖥️  Терминал");
    QAction *vscode   = menu.addAction("💻  VS Code");
    QAction *nvim     = menu.addAction("📝  Neovim");

    menu.addSeparator();

    QAction *titleSys = menu.addAction("── Система ──");
    titleSys->setEnabled(false);
    QAction *files    = menu.addAction("📁  Файловый менеджер");
    QAction *settings = menu.addAction("⚙️  Настройки системы");

    menu.addSeparator();
    QAction *quitAction = menu.addAction("❌  Выйти");

    // ─── Команды ────────────────────────────────────────────
    QObject::connect(firefox,   &QAction::triggered, [](){ launchApp("firefox"); });
    QObject::connect(chromium,  &QAction::triggered, [](){ launchApp("chromium-browser"); });
    QObject::connect(terminal,  &QAction::triggered, [](){ launchApp("x-terminal-emulator"); });
    QObject::connect(vscode,    &QAction::triggered, [](){ launchApp("code"); });
    QObject::connect(nvim,      &QAction::triggered, [](){ launchApp("x-terminal-emulator -e nvim"); });
    QObject::connect(files,     &QAction::triggered, [](){ launchApp("xdg-open $HOME"); });
    QObject::connect(settings,  &QAction::triggered, [](){ launchApp("gnome-control-center"); });
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

    // ─── Иконка в трее ──────────────────────────────────────
    QSystemTrayIcon trayIcon(createIcon());
    trayIcon.setToolTip("My Launcher");
    trayIcon.setContextMenu(&menu);
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated,
        [&](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger)
                trayIcon.contextMenu()->popup(QCursor::pos());
        });
    trayIcon.show();

    // ─── Плавающая иконка ───────────────────────────────────
    FloatingIcon floatingIcon(&menu);
    floatingIcon.show();

    return app.exec();
}
