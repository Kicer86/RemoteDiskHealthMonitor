
#include <cstdio>
#include <array>

#include "../SystemUtilitiesFactory.h"
#include "LinuxDiskCollector.h"
#include "LinGeneralAnalyzer.h"
#include "LinSmartAnalyzer.h"
#include "LsblkOutputParser.h"


namespace
{
    class LinuxDiskCollectorWrapper: public IDiskCollector
    {
        public:
            LinuxDiskCollectorWrapper(std::shared_ptr<IDiskCollector> collector)
                : m_collector(collector)
            {

            }

            std::vector<Disk> GetDisksList() override
            {
                return m_collector->GetDisksList();
            }

        private:
            std::shared_ptr<IDiskCollector> m_collector;
    };
}


struct SystemUtilitiesFactory::State
{
    State()
    {
        std::string output;
        std::array<char, 4096> buffer;

        FILE* pipe = popen("lsblk -rMb", "r");
        if (pipe)
        {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
                output += buffer.data();
            pclose(pipe);
        }

        const auto diskData = LsblkOutputParser::parse(output);

        m_diskCollector = std::make_unique<LinuxDiskCollector>(diskData);
    }

    std::shared_ptr<LinuxDiskCollector> m_diskCollector;
};


SystemUtilitiesFactory::SystemUtilitiesFactory()
    : m_state(std::make_unique<State>())
{

}


SystemUtilitiesFactory::~SystemUtilitiesFactory()
{

}


std::unique_ptr<IDiskCollector> SystemUtilitiesFactory::diskCollector()
{
    return std::make_unique<LinuxDiskCollectorWrapper>(m_state->m_diskCollector);
}


std::vector<std::unique_ptr<IProbe>> SystemUtilitiesFactory::getProbes()
{
    std::vector<std::unique_ptr<IProbe>> probes;
    probes.emplace_back(std::make_unique<LinGeneralAnalyzer>(m_state->m_diskCollector));
    probes.emplace_back(std::make_unique<LinSmartAnalyzer>());

    return probes;
}
