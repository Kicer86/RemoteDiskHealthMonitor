#include <gmock/gmock.h>

#include <optional>

#include "ManualAgentsValidator.hpp"


namespace
{
    struct ValidationResult
    {
        std::optional<AgentInformation> discoveredAgent;
        std::vector<QString> failures;
    };

    ValidationResult validate(const QString& name, const QString& ip, const QString& port)
    {
        ManualAgentsValidator validator;
        ValidationResult result;

        QObject::connect(&validator, &ManualAgentsValidator::agentDiscovered,
                         [&result](const AgentInformation& info) {
                             result.discoveredAgent = info;
                         });
        QObject::connect(&validator, &ManualAgentsValidator::validationFailed,
                         [&result](const QString& reason) {
                             result.failures.push_back(reason);
                         });

        validator.addNewAgent(name, ip, port);

        return result;
    }
}


TEST(ManualAgentsValidatorTest, rejectsEmptyAgentName)
{
    const auto result = validate("  \t ", "192.168.1.10", "1630");

    EXPECT_FALSE(result.discoveredAgent.has_value());
    ASSERT_EQ(result.failures.size(), 1);
    EXPECT_EQ(result.failures[0], "Agent name cannot be empty");
}


TEST(ManualAgentsValidatorTest, rejectsInvalidIpAddress)
{
    const std::vector<QString> invalidIps = {
        "",
        "192.168.1",
        "192.168.1.1.1",
        "192.168.1.256",
        "192.168.one.1",
    };

    for (const auto& ip : invalidIps)
    {
        const auto result = validate("agent", ip, "1630");

        EXPECT_FALSE(result.discoveredAgent.has_value()) << ip.toStdString();
        ASSERT_EQ(result.failures.size(), 1) << ip.toStdString();
        EXPECT_EQ(result.failures[0], "Invalid IP address") << ip.toStdString();
    }
}


TEST(ManualAgentsValidatorTest, rejectsInvalidPort)
{
    const std::vector<QString> invalidPorts = {
        "",
        "0",
        "65536",
        "abc",
    };

    for (const auto& port : invalidPorts)
    {
        const auto result = validate("agent", "192.168.1.10", port);

        EXPECT_FALSE(result.discoveredAgent.has_value()) << port.toStdString();
        ASSERT_EQ(result.failures.size(), 1) << port.toStdString();
        EXPECT_EQ(result.failures[0], "Port must be between 1 and 65535") << port.toStdString();
    }
}


TEST(ManualAgentsValidatorTest, emitsTrimmedHardcodedAgentForValidInput)
{
    const auto result = validate("  agent  ", "10.0.0.5", "65535");

    ASSERT_TRUE(result.discoveredAgent.has_value());
    EXPECT_TRUE(result.failures.empty());
    EXPECT_EQ(result.discoveredAgent->name(), "agent");
    EXPECT_EQ(result.discoveredAgent->host(), QHostAddress("10.0.0.5"));
    EXPECT_EQ(result.discoveredAgent->port(), 65535);
    EXPECT_EQ(result.discoveredAgent->detectionSource(), AgentInformation::DetectionSource::Hardcoded);
}
