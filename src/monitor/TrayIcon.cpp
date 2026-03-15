#include "TrayIcon.hpp"

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QWindow>

#include "AgentsList.hpp"


TrayIcon::TrayIcon(AgentsList& agents, QObject* parent)
    : QObject(parent)
    , m_agents(agents)
{
    m_menu = std::make_unique<QMenu>();
    auto* showAction = m_menu->addAction(tr("Show / Hide"));
    m_menu->addSeparator();
    auto* quitAction = m_menu->addAction(tr("Quit"));

    connect(showAction, &QAction::triggered, this, [this]() {
        if (!m_window)
            return;

        if (m_window->isVisible())
        {
            m_window->hide();
        }
        else
        {
            m_window->show();
            m_window->raise();
            m_window->requestActivate();
        }
    });

    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon.setContextMenu(m_menu.get());
    m_trayIcon.setToolTip(tr("Remote Disc Health Monitor"));

    connect(&m_trayIcon, &QSystemTrayIcon::activated,
            this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::DoubleClick && reason != QSystemTrayIcon::Trigger)
            return;
        if (!m_window)
            return;

        if (m_window->isVisible())
        {
            m_window->hide();
        }
        else
        {
            m_window->show();
            m_window->raise();
            m_window->requestActivate();
        }
    });

    m_blinkTimer.setInterval(800);
    connect(&m_blinkTimer, &QTimer::timeout, this, &TrayIcon::onBlinkTick);

    connect(&m_agents, &QAbstractItemModel::dataChanged,  this, &TrayIcon::updateIcon);
    connect(&m_agents, &QAbstractItemModel::rowsInserted, this, &TrayIcon::updateIcon);
    connect(&m_agents, &QAbstractItemModel::rowsRemoved,  this, &TrayIcon::updateIcon);
    connect(&m_agents, &QAbstractItemModel::modelReset,   this, &TrayIcon::updateIcon);

    updateIcon();
    m_trayIcon.show();
}


TrayIcon::~TrayIcon() = default;


void TrayIcon::setWindow(QWindow* window)
{
    m_window = window;
}


GeneralHealth::Health TrayIcon::worstHealth() const
{
    GeneralHealth::Health worst = GeneralHealth::UNKNOWN;
    const int count = m_agents.rowCount({});

    for (int i = 0; i < count; ++i)
    {
        const auto idx = m_agents.index(i);
        const auto health = static_cast<GeneralHealth::Health>(
            m_agents.data(idx, AgentsList::AgentHealthRole).toInt());

        if (health > worst)
            worst = health;
    }

    return worst;
}


void TrayIcon::updateIcon()
{
    const auto health = worstHealth();
    m_currentHealth = health;

    const bool shouldBlink = (health == GeneralHealth::CHECK_STATUS
                           || health == GeneralHealth::BAD);

    if (shouldBlink && !m_blinkTimer.isActive())
    {
        m_blinkVisible = true;
        m_blinkTimer.start();
    }
    else if (!shouldBlink && m_blinkTimer.isActive())
    {
        m_blinkTimer.stop();
        m_blinkVisible = true;
    }

    m_trayIcon.setIcon(createHealthIcon(m_currentHealth, !m_blinkVisible));

    static const QString tooltipFmt = tr("RDHM — %1");
    const QString healthStr = QString::fromStdString(healthToString(health));
    m_trayIcon.setToolTip(tooltipFmt.arg(healthStr));
}


void TrayIcon::onBlinkTick()
{
    m_blinkVisible = !m_blinkVisible;
    m_trayIcon.setIcon(createHealthIcon(m_currentHealth, !m_blinkVisible));
}


QIcon TrayIcon::createHealthIcon(GeneralHealth::Health health, bool dim)
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor body;
    QColor led;

    switch (health)
    {
        case GeneralHealth::GOOD:
            body = QColor("#4CAF50");
            led  = QColor("#81C784");
            break;
        case GeneralHealth::CHECK_STATUS:
            body = QColor("#FF9800");
            led  = QColor("#FFB74D");
            break;
        case GeneralHealth::BAD:
            body = QColor("#F44336");
            led  = QColor("#E57373");
            break;
        default:
            body = QColor("#9E9E9E");
            led  = QColor("#BDBDBD");
            break;
    }

    if (dim)
    {
        body.setAlpha(60);
        led.setAlpha(60);
    }

    // HDD body
    painter.setBrush(body);
    painter.setPen(QPen(body.darker(140), 2));
    painter.drawRoundedRect(4, 10, 56, 44, 6, 6);

    // Platter circle
    painter.setPen(QPen(body.lighter(140), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(10, 16, 28, 28);

    // Activity LED
    painter.setPen(Qt::NoPen);
    painter.setBrush(led);
    painter.drawEllipse(44, 38, 12, 12);

    return QIcon(pixmap);
}
