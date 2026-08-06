/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "AccessLogging.h"

#include <iostream>
#include <sstream>

namespace SwApi {

namespace AccessLogging {

//region AccessNode

AccessNode::AccessNode(AccessNode* parent,
                       const std::shared_ptr<IAccessEntry>& data)
    : _parent(parent)
    , _data(data) {
    this->_children = std::vector<std::shared_ptr<AccessNode>>();
}

AccessNode* AccessNode::GetParent() const {
    return this->_parent;
}

std::shared_ptr<IAccessEntry> AccessNode::GetData() const {
    return this->_data;
}

const std::vector<std::shared_ptr<AccessNode>>& AccessNode::GetChildren() const {
    return this->_children;
}

std::vector<std::shared_ptr<AccessNode>>& AccessNode::GetChildren() {
    return this->_children;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& v) {
    if (v == nullptr) {
        return os << "(null)";
    } else {
        return os << *v;
    }
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << '[';
    for (const auto& x : v) {
        os << x << ',';
    }
    return os << ']';
}

std::ostream& operator<<(std::ostream& os, const AccessNode& an) {
    return os << "AccessNode(parent:" << an.GetParent()
       << ", data:" << an.GetData()
       << ", children:" << an.GetChildren()
       << ")";
}

std::string AccessNode::ToString() const {
    std::stringstream ss; ss << *this;
    return ss.str();
}

std::string AccessNode::PrettyPrint() {
    std::stringstream ss;
    unsigned int myDepth = this->CalcDepth();
    auto tabToDepth = [myDepth](std::ostream& os) -> std::ostream& {
        for (unsigned int tabsLeft = myDepth+1; tabsLeft > 0; tabsLeft--) {
            os << '\t';
        }
        return os;
    };

    if (_data == nullptr) {
        ss << '-';
    } else {
        ss << _data->PrettyPrint();
    }

    ss << '\n';

    if (this->_children.size() > 0) {
        for (auto spNode : this->_children) {
            tabToDepth(ss) << spNode->PrettyPrint();
        }
        ss << '\n';
    }

    return ss.str();
}

unsigned int AccessNode::CalcDepth() {
    if (_parent == nullptr) {
        return 0;
    } else {
        return 1 + _parent->CalcDepth();
    }
}

std::shared_ptr<AccessNode> AccessNode::AddChild(const std::shared_ptr<IAccessEntry>& e) {
    auto ret = std::make_shared<AccessNode>(this, e);
    this->_children.push_back(ret);
    return ret;
}

//endregion

//region Access

Access::Access(uint32_t busID,
               std::string deviceName,
               uint16_t deviceAddress,
               unsigned int registerAddress,
               unsigned int value)
    : busID(busID)
    , deviceName(std::move(deviceName))
    , deviceAddress(deviceAddress)
    , registerAddress(registerAddress)
    , value(value)
{

}

std::ostream& operator<<(std::ostream& os, const Access& access) {
    return os << "Access(device:" << access.deviceName
        << ", busID:" << access.busID
        << ", deviceAddress:"   << std::dec << access.deviceAddress
                        << " (0x" << std::hex << access.deviceAddress << ')'
        << ", registerAddress:" << std::dec << access.registerAddress
                        << " (0x" << std::hex << access.registerAddress << ')'
        << ", value:"           << std::dec << access.value
                        << " (0x" << std::hex << access.value << ')'
        << std::dec
        << ")";
}

std::string Access::ToString() const {
    std::stringstream ss; ss << *this;
    return ss.str();
}

std::string Access::PrettyPrint() const {
    return ToString();
}


//endregion

//region AccessComment

AccessComment::AccessComment(std::string c)
     : comment(std::move(c))
{

}

std::ostream& operator<<(std::ostream& os, const AccessComment& ac) {
    return os << "AccessComment(" << ac.comment << ")";
}

std::string AccessComment::ToString() const {
    std::stringstream ss; ss << *this;
    return ss.str();
}

std::string AccessComment::PrettyPrint() const {
    return comment;
}

//endregion

//region AccessSection

AccessSection::AccessSection(std::string c)
     : comment(std::move(c))
{

}

std::ostream& operator<<(std::ostream& os, const AccessSection& section) {
    return os << " | " << section.comment;
}

std::string AccessSection::ToString() const {
    std::stringstream ss; ss << *this;
    return ss.str();
}

std::string AccessSection::PrettyPrint() const {
    return ToString();
}

//endregion


//region AccessLogger

AccessLogger accessLogger;

AccessLogger::AccessLogger(std::string initialMessage) {
    this->root = std::make_shared<AccessNode>(nullptr,
                                              std::make_shared<AccessComment>(initialMessage));
    this->cursor = root.get();
}

AccessLogger& AccessLogger::Get() noexcept {
    return accessLogger;
}

void AccessLogger::Log(const std::shared_ptr<IAccessEntry>& e) {
    cursor->AddChild(e);
}

std::shared_ptr<AccessSection> AccessLogger::EnterSection(AccessSection section) {
    auto copy = std::make_shared<AccessSection>(section);
    auto spAccessNode = cursor->AddChild(copy);
    cursor = spAccessNode.get();
    return copy;
}

void AccessLogger::ExitSection() {
    if (cursor->GetParent() == nullptr) {
        // Do nothing
    } else {
        cursor = cursor->GetParent();
    }
}

std::string AccessLogger::PrettyPrint() {
    return root->PrettyPrint();
}

std::ostream& operator<<(std::ostream& os, const AccessLogger& a) {
    return os << "AccessLog:\n\t"
       << *a.root;
}

AccessLogger& operator<<(AccessLogger& logger, const std::string& s) {
    AccessComment accessComment{s};
    return logger << accessComment;
}

AccessLogger& operator<<(AccessLogger& logger, const AccessComment& ac) {
    auto copy = std::make_shared<AccessComment>(ac);
    logger.Log(copy);
    return logger;
}

AccessLogger& operator<<(AccessLogger& logger, const Access& access) {
    auto copy = std::make_shared<Access>(access);
    logger.Log(copy);
    return logger;
}

//endregion

//region AccessScope

AccessScope::AccessScope(std::string name) {
    this->spSection = AccessLogger::Get().EnterSection(AccessSection(std::move(name)));
}

AccessScope::~AccessScope() {
    AccessLogger::Get().ExitSection();
}

//endregion

std::ostream& operator<<(std::ostream& os, const IAccessEntry& a) {
    return os << a.ToString();
}

} // namespace AccessLogging

} // namespace SwApi

