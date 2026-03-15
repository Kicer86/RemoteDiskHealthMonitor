
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QWindow>

#include "AgentsList.hpp"
#include "AgentsExplorer.hpp"
#include "AgentsStatusProvider.hpp"
#include "ManualAgentsValidator.hpp"
#include "HealthEnumQml.hpp"
#include "Configuration.hpp"
#include "TrayIcon.hpp"


namespace
{
    void storeHardcodedAgents(Configuration& config, const AgentsList& agentsList)
    {
        const QVector<AgentInformation>& allAgents = agentsList.agents();
        QVector<AgentInformation> hardcodedAgents;

        std::copy_if(allAgents.begin(), allAgents.end(), std::back_inserter(hardcodedAgents), [](const AgentInformation& info){
            return info.detectionSource() == AgentInformation::DetectionSource::Hardcoded;
        });

        config.storeAgents(hardcodedAgents);
    }

    void restoreHardcodedAgents(Configuration& config, AgentsList& agentsList)
    {
        QVector<AgentInformation> hardcodedAgents = config.readAgents();

        for(const auto& agent: hardcodedAgents)
            agentsList.addAgent(agent);
    }
}


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QQuickStyle::setStyle("Fusion");
    app.setQuitOnLastWindowClosed(false);

    Configuration config;

    AgentsStatusProvider statusProvider;
    AgentsList activeAgents(statusProvider);

    ManualAgentsValidator manualAgentsValidator;
    QObject::connect(&manualAgentsValidator, &ManualAgentsValidator::agentDiscovered,
                     &activeAgents, &AgentsList::addAgent);

    AgentsExplorer agentsEnumerator;
    QObject::connect(&agentsEnumerator, &AgentsExplorer::agentDiscovered, &activeAgents, &AgentsList::addAgent);
    QObject::connect(&agentsEnumerator, &AgentsExplorer::agentLost, &activeAgents, &AgentsList::removeAgent);

    agentsEnumerator.startListening();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("agentsModel", &activeAgents);
    engine.rootContext()->setContextProperty("agentsValidator", &manualAgentsValidator);

    engine.loadFromModule("RDHM.Monitor", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    TrayIcon trayIcon(activeAgents);
    auto* window = qobject_cast<QWindow*>(engine.rootObjects().first());
    trayIcon.setWindow(window);

    restoreHardcodedAgents(config, activeAgents);

    const int exitCode = app.exec();

    storeHardcodedAgents(config, activeAgents);

    return exitCode;
}
