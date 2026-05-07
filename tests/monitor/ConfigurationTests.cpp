#include <gmock/gmock.h>

#include <QSettings>
#include <QTemporaryDir>

#include "Configuration.hpp"
#include "common/constants.hpp"


namespace
{
    void useTemporarySettingsPath(const QTemporaryDir& dir)
    {
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dir.path());
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                           QString::fromStdString(ApplicationShortName),
                           QString::fromStdString(ApplicationShortName));
        settings.clear();
        settings.sync();
    }
}


TEST(ConfigurationTest, persistsAgentsAcrossInstances)
{
    QTemporaryDir settingsDir;
    ASSERT_TRUE(settingsDir.isValid());
    useTemporarySettingsPath(settingsDir);

    const QVector<AgentInformation> agents = {
        AgentInformation("Agent, One; Encoded", QHostAddress("192.168.1.10"), 1630,
                         AgentInformation::DetectionSource::Hardcoded),
        AgentInformation("ZeroConf Agent", QHostAddress("fe80::1"), 1631,
                         AgentInformation::DetectionSource::ZeroConf),
    };

    {
        Configuration configuration;
        configuration.storeAgents(agents);
    }

    {
        Configuration configuration;
        const auto restored = configuration.readAgents();

        ASSERT_EQ(restored.size(), agents.size());
        EXPECT_EQ(restored[0], agents[0]);
        EXPECT_EQ(restored[0].detectionSource(), agents[0].detectionSource());
        EXPECT_EQ(restored[1], agents[1]);
        EXPECT_EQ(restored[1].detectionSource(), agents[1].detectionSource());
    }
}


TEST(ConfigurationTest, storesEmptyAgentList)
{
    QTemporaryDir settingsDir;
    ASSERT_TRUE(settingsDir.isValid());
    useTemporarySettingsPath(settingsDir);

    {
        Configuration configuration;
        configuration.storeAgents({});
    }

    {
        Configuration configuration;
        EXPECT_TRUE(configuration.readAgents().empty());
    }
}


TEST(ConfigurationTest, skipsMalformedAgentEntries)
{
    QTemporaryDir settingsDir;
    ASSERT_TRUE(settingsDir.isValid());
    useTemporarySettingsPath(settingsDir);

    const QString encodedName = QString::fromUtf8(QByteArray("valid").toBase64());
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QString::fromStdString(ApplicationShortName),
                       QString::fromStdString(ApplicationShortName));
    settings.setValue("agents",
                      QString("too,few,fields;%1,127.0.0.1,1630,1;too,many,fields,for,agent")
                          .arg(encodedName));
    settings.sync();

    Configuration configuration;
    const auto restored = configuration.readAgents();

    ASSERT_EQ(restored.size(), 1);
    EXPECT_EQ(restored[0].name(), "valid");
    EXPECT_EQ(restored[0].host(), QHostAddress("127.0.0.1"));
    EXPECT_EQ(restored[0].port(), 1630);
    EXPECT_EQ(restored[0].detectionSource(), AgentInformation::DetectionSource::Hardcoded);
}
