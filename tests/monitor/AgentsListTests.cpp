
#include <gmock/gmock.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>

#include "AgentsList.hpp"
#include "IAgentsStatusProviderMock.hpp"
#include "common/JSonUtils.hpp"


using testing::_;
using testing::Contains;
using testing::InvokeArgument;
using testing::IsSupersetOf;
using testing::NiceMock;
using testing::UnorderedElementsAre;


TEST(AgentsListTest, isConstructible)
{
    EXPECT_TRUE( (std::is_constructible_v<AgentsList, IAgentsStatusProvider&, QObject *>) );
}


TEST(AgentsListTest, savesNewAgentInfo)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    aal.addAgent(info1);
    aal.addAgent(info2);

    ASSERT_EQ(aal.rowCount({}), 2);
}


TEST(AgentsListTest, noDuplicatesAllowed)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);

    aal.addAgent(info1);
    aal.addAgent(info2);

    ASSERT_EQ(aal.rowCount({}), 1);
}


TEST(AgentsListTest, doubleAgentRemoveShouldBeSafe)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    aal.addAgent(info1);
    aal.addAgent(info2);

    EXPECT_NO_THROW({
        aal.removeAgent(info1);
        aal.removeAgent(info1);
        aal.removeAgent(info2);
        aal.removeAgent(info2);
    });
}


TEST(AgentsListTest, agentRemoval)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    aal.addAgent(info1);
    aal.addAgent(info2);

    aal.removeAgent(info1);
    ASSERT_EQ(aal.rowCount({}), 1);

    const auto& agents = aal.agents();
    EXPECT_THAT(agents, Contains(info2));
}


TEST(AgentsListTest, listofAvailableRoles)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    QStringList listOfRoles;

    const auto roles = aal.roleNames();
    for(auto it = roles.begin(); it != roles.end(); ++it)
        listOfRoles.append(it.value());

    EXPECT_THAT(listOfRoles, IsSupersetOf( {"agentName", "agentHealth", "agentDetectionType"} ));
}


TEST(AgentsListTest, exposesAgentsToView)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    aal.addAgent(info1);
    aal.addAgent(info2);

    const auto idx1 = aal.index(0, 0);
    const auto idx2 = aal.index(1, 0);

    QStringList names;
    names.append(idx1.data(AgentsList::AgentNameRole).toString());
    names.append(idx2.data(AgentsList::AgentNameRole).toString());

    EXPECT_THAT(names, UnorderedElementsAre("Krzysiu", "Zbysiu"));
}


TEST(AgentsListTest, emitsSignalsWhenAgentsAdded)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    QSignalSpy newRowsSpy(&aal, &AgentsList::rowsInserted);

    aal.addAgent(info1);
    aal.addAgent(info2);

    ASSERT_EQ(newRowsSpy.count(), 2);

    // verify first emission
    EXPECT_EQ(newRowsSpy.at(0).at(0), QModelIndex());   // no parent
    EXPECT_EQ(newRowsSpy.at(0).at(1), 0);               // first row
    EXPECT_EQ(newRowsSpy.at(0).at(2), 0);               // 1 item

    // verify second emission
    EXPECT_EQ(newRowsSpy.at(1).at(0), QModelIndex());   // no parent
    EXPECT_EQ(newRowsSpy.at(1).at(1), 1);               // second row
    EXPECT_EQ(newRowsSpy.at(1).at(2), 1);               // 1 item
}


TEST(AgentsListTest, emitsSignalsWhenAgentsRemoved)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    QSignalSpy removedRowsSpy(&aal, &AgentsList::rowsRemoved);

    aal.addAgent(info1);
    aal.addAgent(info2);

    aal.removeAgent(info2);
    aal.removeAgent(info1);

    ASSERT_EQ(removedRowsSpy.count(), 2);

    // verify first emission
    EXPECT_EQ(removedRowsSpy.at(0).at(0), QModelIndex());   // no parent
    EXPECT_EQ(removedRowsSpy.at(0).at(1), 1);               // second row
    EXPECT_EQ(removedRowsSpy.at(0).at(2), 1);               // 1 item

    // verify second emission
    EXPECT_EQ(removedRowsSpy.at(1).at(0), QModelIndex());   // no parent
    EXPECT_EQ(removedRowsSpy.at(1).at(1), 0);               // first row
    EXPECT_EQ(removedRowsSpy.at(1).at(2), 0);               // 1 item
}


TEST(AgentsListTest, itemsShouldBeChildless)
{
    NiceMock<IAgentsStatusProviderMock> statusProvider;
    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    aal.addAgent(info1);
    aal.addAgent(info2);

    const QModelIndex idx1 = aal.index(0, 0, {});
    const QModelIndex idx2 = aal.index(1, 0, {});

    EXPECT_EQ(aal.rowCount(idx1), 0);
    EXPECT_EQ(aal.rowCount(idx2), 0);
}


TEST(AgentsListTest, fetchHealthOfNewAgents)
{
    IAgentsStatusProviderMock statusProvider;

    AgentsList aal(statusProvider);

    AgentInformation info1("Krzysiu", QHostAddress("192.168.1.12"), 2300, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Zbysiu", QHostAddress("192.168.1.45"), 2301, AgentInformation::DetectionSource::Hardcoded);

    EXPECT_CALL(statusProvider, observe(info1)).Times(1);
    EXPECT_CALL(statusProvider, observe(info2)).Times(1);

    aal.addAgent(info1);
    aal.addAgent(info2);
    aal.addAgent(info2);

    ASSERT_EQ(aal.rowCount({}), 2);
}


TEST(AgentsListTest, healthUpdatesAfterFetch)
{
    IAgentsStatusProviderMock statusProvider;

    AgentsList aal(statusProvider);

    AgentInformation info1("John Connor", QHostAddress("192.168.1.15"), 1998, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("T-1000", QHostAddress("192.168.1.16"), 1998, AgentInformation::DetectionSource::Hardcoded);

    EXPECT_CALL(statusProvider, observe(info1)).Times(1);
    EXPECT_CALL(statusProvider, observe(info2)).Times(1);

    aal.addAgent(info1);
    aal.addAgent(info2);

    emit statusProvider.statusChanged(info1, GeneralHealth::GOOD);
    emit statusProvider.statusChanged(info2, GeneralHealth::BAD);

    const QModelIndex idx1 = aal.index(0, 0);
    const QModelIndex idx2 = aal.index(1, 0);

    EXPECT_EQ(idx1.data(AgentsList::AgentHealthRole), GeneralHealth::GOOD);
    EXPECT_EQ(idx2.data(AgentsList::AgentHealthRole), GeneralHealth::BAD);
}


TEST(AgentsListTest, agentDetectionTypeRoleFetching)
{
    IAgentsStatusProviderMock statusProvider;

    AgentsList aal(statusProvider);

    AgentInformation info1("John Connor", QHostAddress("192.168.1.15"), 1998, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("T-1000", QHostAddress("192.168.1.16"), 1998, AgentInformation::DetectionSource::ZeroConf);

    aal.addAgent(info1);
    aal.addAgent(info2);

    const QModelIndex idx1 = aal.index(0, 0);
    const QModelIndex idx2 = aal.index(1, 0);

    EXPECT_EQ(idx1.data(AgentsList::AgentDetectionTypeRole), static_cast<int>(AgentInformation::DetectionSource::Hardcoded));
    EXPECT_EQ(idx2.data(AgentsList::AgentDetectionTypeRole), static_cast<int>(AgentInformation::DetectionSource::ZeroConf));
}

TEST(AgentsListTest, GetAgentDiskInfoNamesAfterFetch)
{
    IAgentsStatusProviderMock statusProvider;

    AgentsList aal(statusProvider);

    AgentInformation info1("Scorpion", QHostAddress("192.168.1.99"), 1998, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Sub-Zero", QHostAddress("192.168.1.101"), 1998, AgentInformation::DetectionSource::Hardcoded);

    EXPECT_CALL(statusProvider, observe(info1)).Times(1);
    EXPECT_CALL(statusProvider, observe(info2)).Times(1);

    aal.addAgent(info1);
    aal.addAgent(info2);

    QStringList answer1, answer2;

    const QModelIndex idx1 = aal.index(0, 0);
    const QModelIndex idx2 = aal.index(1, 0);

    EXPECT_EQ(idx1.data(AgentsList::AgentDiskInfoNamesRole), answer1);
    EXPECT_EQ(idx2.data(AgentsList::AgentDiskInfoNamesRole), answer2);
}

TEST(AgentsListTest, DiskInfoCollectionUpdateAfterFetch)
{
    IAgentsStatusProviderMock statusProvider;

    AgentsList aal(statusProvider);

    AgentInformation info1("Scorpion", QHostAddress("192.168.1.99"), 1998, AgentInformation::DetectionSource::Hardcoded);
    AgentInformation info2("Sub-Zero", QHostAddress("192.168.1.101"), 1998, AgentInformation::DetectionSource::Hardcoded);

    EXPECT_CALL(statusProvider, observe(info1)).Times(1);
    EXPECT_CALL(statusProvider, observe(info2)).Times(1);

    aal.addAgent(info1);
    aal.addAgent(info2);

    GeneralHealth::Health good = GeneralHealth::GOOD;
    GeneralHealth::Health checkStatus = GeneralHealth::CHECK_STATUS;
    GeneralHealth::Health bad = GeneralHealth::BAD;

    DiskInfo di1a("Disk Numero Uno", good, {});
    DiskInfo di1b("Disk Numero Dos", checkStatus, {});
    DiskInfo di2a("Disk Cif", bad, {});
    DiskInfo di2b("Disk Domestos", checkStatus, {});

    std::vector<DiskInfo> v1, v2;
    v1.push_back(di1a);
    v1.push_back(di1b);
    v2.push_back(di2a);
    v2.push_back(di2b);

    QJsonObject jsonObj1;
    jsonObj1["disks"] = JSonUtils::DiskInfoToJSon(v1);
    const QJsonDocument doc1(jsonObj1);

    QJsonObject jsonObj2;
    jsonObj2["disks"] = JSonUtils::DiskInfoToJSon(v2);
    const QJsonDocument doc2(jsonObj2);

    emit statusProvider.diskCollectionChanged(info1, doc1);
    emit statusProvider.diskCollectionChanged(info2, doc2);

    const QModelIndex idx1 = aal.index(0, 0);
    const QModelIndex idx2 = aal.index(1, 0);

    QStringList answer1, answer2;
    answer1.append("Disk Numero Uno");
    answer1.append("Disk Numero Dos");
    answer2.append("Disk Cif");
    answer2.append("Disk Domestos");

    EXPECT_EQ(idx1.data(AgentsList::AgentDiskInfoNamesRole), answer1);
    EXPECT_EQ(idx2.data(AgentsList::AgentDiskInfoNamesRole), answer2);
}
