#pragma once

#include <memory>

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QIcon>

#include "common/GeneralHealth.h"

class AgentsList;
class QMenu;
class QWindow;


class TrayIcon : public QObject
{
    Q_OBJECT

public:
    TrayIcon(AgentsList& agents, QObject* parent = nullptr);
    ~TrayIcon() override;

    void setWindow(QWindow* window);

private:
    void updateIcon();
    void onBlinkTick();
    GeneralHealth::Health worstHealth() const;
    static QIcon createHealthIcon(GeneralHealth::Health health, bool dim);

    QSystemTrayIcon m_trayIcon;
    std::unique_ptr<QMenu> m_menu;
    QTimer m_blinkTimer;
    AgentsList& m_agents;
    QWindow* m_window = nullptr;
    GeneralHealth::Health m_currentHealth = GeneralHealth::UNKNOWN;
    bool m_blinkVisible = true;
};
