#include <Link.hpp>
#include <iostream>
#include <vector>

Link::Target::Target(std::string host, std::string target) {
    this->hosts.push_back(host);
    this->target = target;
}

Link::Target* Link::Target::AddHost(std::string host) {
    this->hosts.push_back(host);
    return this;
}

std::vector<std::string> Link::Target::GetHosts() {
    return this->hosts;
}

std::string Link::Target::GetTarget() {
    return this->target;
}