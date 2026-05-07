#include <gmock/gmock.h>

#include <QSet>

#include "AgentInformation.hpp"


TEST(AgentInformationTest, exposesConstructorValues)
{
    const AgentInformation info("agent", QHostAddress("192.168.1.10"), 1630,
                                AgentInformation::DetectionSource::Hardcoded);

    EXPECT_EQ(info.name(), "agent");
    EXPECT_EQ(info.host(), QHostAddress("192.168.1.10"));
    EXPECT_EQ(info.port(), 1630);
    EXPECT_EQ(info.detectionSource(), AgentInformation::DetectionSource::Hardcoded);
}


TEST(AgentInformationTest, equalityAndHashUseEndpointIdentity)
{
    const AgentInformation zeroConf("agent", QHostAddress("192.168.1.10"), 1630,
                                    AgentInformation::DetectionSource::ZeroConf);
    const AgentInformation hardcoded("agent", QHostAddress("192.168.1.10"), 1630,
                                     AgentInformation::DetectionSource::Hardcoded);

    EXPECT_EQ(zeroConf, hardcoded);
    EXPECT_EQ(qHash(zeroConf, 0), qHash(hardcoded, 0));

    QSet<AgentInformation> uniqueAgents;
    uniqueAgents.insert(zeroConf);
    uniqueAgents.insert(hardcoded);

    EXPECT_EQ(uniqueAgents.size(), 1);
}


TEST(AgentInformationTest, endpointDifferencesMakeAgentsDifferent)
{
    const AgentInformation base("agent", QHostAddress("192.168.1.10"), 1630,
                                AgentInformation::DetectionSource::Hardcoded);

    EXPECT_FALSE(base == AgentInformation("other", QHostAddress("192.168.1.10"), 1630,
                                          AgentInformation::DetectionSource::Hardcoded));
    EXPECT_FALSE(base == AgentInformation("agent", QHostAddress("192.168.1.11"), 1630,
                                          AgentInformation::DetectionSource::Hardcoded));
    EXPECT_FALSE(base == AgentInformation("agent", QHostAddress("192.168.1.10"), 1631,
                                          AgentInformation::DetectionSource::Hardcoded));
}
