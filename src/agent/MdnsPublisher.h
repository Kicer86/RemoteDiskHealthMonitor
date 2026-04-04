#pragma once

#include <string>
#include <memory>


class MdnsPublisher
{
public:
    MdnsPublisher(const std::string& serviceName, const std::string& serviceType,
                  unsigned int port);
    ~MdnsPublisher();

    void start();
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
