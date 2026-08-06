/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <iosfwd>
#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <sstream>

/// I2C Access Logging API
/// These classes are used to log & track the accesses made by the application.
/// This is hierarchical, so that you may mark off different sections in the logs.
///
/// Usage:
///   To log an access, simply do:
///     AccessLogger::Get() << {
///       busID,
///       "Device name",
///       deviceAddress,
///       registerAddress,
///       value
///     };
///
///   To mark off different sections of logging (e.g. to annotate that you're,
///   say, changing resolutions), use the AccessScope class, by declaring an instance:
///
///     void MyFunction() {
///         AccessScope log{"Changing resolution now!"};
///         // ... do things here...
///         // ... call other functions which may start their own scopes...
///
///         // The AccessScope ends here, automatically letting the logger know the resolution change is done.
///     }
///
///   This will automatically manage the tree of accesses for future use.
///
///   A convenient way to log scopes is to just use the surrounding function
///   name. This can be done like so:
///
///     void MyFunction() {
///         AccessScope log{__func__};
///     }
///

namespace SwApi {

namespace AccessLogging {

struct IAccessEntry : std::enable_shared_from_this<IAccessEntry> {
    virtual ~IAccessEntry() {};
    virtual std::string ToString() const = 0;
    virtual std::string PrettyPrint() const = 0;
};

struct Access : virtual IAccessEntry {
    uint32_t busID;
    std::string deviceName;
    uint16_t deviceAddress;
    unsigned int registerAddress;
    unsigned int value;

    Access(uint32_t busID,
           std::string deviceName,
           uint16_t deviceAddress,
           unsigned int registerAddress,
           unsigned int value);
    std::string ToString() const override;
    std::string PrettyPrint() const override;
};

struct AccessComment : virtual IAccessEntry {
    AccessComment(std::string c);
    std::string comment = "";
    std::string ToString() const override;
    std::string PrettyPrint() const override;
};

struct AccessSection : virtual IAccessEntry {
    AccessSection(std::string c);
    std::string comment = "";
    std::string ToString() const override;
    std::string PrettyPrint() const override;
};

class AccessScope {
public:
    AccessScope(std::string name);
    ~AccessScope();
    AccessScope(const AccessScope&) = delete;
    AccessScope& operator=(const AccessScope&) = delete;

    std::shared_ptr<AccessSection> spSection;
};

template<typename T>
AccessScope& operator<<(AccessScope& sc, const T& s) {
    std::stringstream ss;
    ss << sc.spSection->comment << s;
    sc.spSection->comment = ss.str();
    return sc;
}

class AccessNode {
public:
    AccessNode(AccessNode* parent, const std::shared_ptr<IAccessEntry>& data);

    AccessNode* GetParent() const;
    std::shared_ptr<IAccessEntry> GetData() const;

    const std::vector<std::shared_ptr<AccessNode>>& GetChildren() const;
    std::vector<std::shared_ptr<AccessNode>>& GetChildren();
    std::shared_ptr<AccessNode> AddChild(const std::shared_ptr<IAccessEntry>& e);

    unsigned int CalcDepth();

    std::string ToString() const;
    std::string PrettyPrint();
private:
    AccessNode* _parent = nullptr;
    std::shared_ptr<IAccessEntry> _data;
    std::vector<std::shared_ptr<AccessNode>> _children;
};

class AccessLogger {
public:
    static constexpr char DefaultInitialMessage[] = "Log of I2C Accesses";
    AccessLogger(std::string initialMessage=DefaultInitialMessage);
    static AccessLogger& Get() noexcept;
    void Log(const std::shared_ptr<IAccessEntry>& e);
    std::shared_ptr<AccessSection> EnterSection(AccessSection s);
    void ExitSection();
    std::string PrettyPrint();

    std::shared_ptr<AccessNode> root;
    AccessNode* cursor;
};


AccessLogger& operator<<(AccessLogger& logger, const std::string& s);
AccessLogger& operator<<(AccessLogger& logger, const Access& a);
AccessLogger& operator<<(AccessLogger& logger, const AccessComment& ac);

std::ostream& operator<<(std::ostream& os, const IAccessEntry& a);
std::ostream& operator<<(std::ostream& os, const AccessNode& a);
std::ostream& operator<<(std::ostream& os, const Access& a);
std::ostream& operator<<(std::ostream& os, const AccessComment& ac);
std::ostream& operator<<(std::ostream& os, const AccessSection& s);
std::ostream& operator<<(std::ostream& os, const AccessLogger& l);

} // namespace AccessLogging

} // namespace SwApi
