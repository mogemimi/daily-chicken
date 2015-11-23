// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "ProjectTemplate.h"
#include "FileSystem.h"
#include "StringHelper.h"
#include "Any.h"
#include "../somerachan/src/optional.h"
#include <fstream>
#include <utility>
#include <random>
#include <ctime>
#include <sstream>
#include <vector>
#include <cassert>
#include <map>

namespace somera {
namespace {

//std::string generateXcodeID()
//{
//    std::random_device device;
//    static uint32_t hash1 = device();
//    hash1 += 2;
//
//    ::time_t timeRaw;
//    ::time(&timeRaw);
//    static uint32_t hash2 = static_cast<uint32_t>(timeRaw);
//    std::uniform_int_distribution<uint8_t> dist(0, 16);
//    hash2 += dist(device);
//
//    static const uint32_t hash3 = device();
//
//    std::string id = StringHelper::format("%08X%08X%08X", hash1, hash2, hash3);
//    return id;
//}

std::string generateXCWorkSpaceData(const std::string& xcodeprojName)
{
    std::stringstream stream;
    stream <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Workspace\n"
    "   version = \"1.0\">\n"
    "   <FileRef\n";
    stream <<
    "      location = \"self:" << xcodeprojName << "\">\n";
    stream <<
    "   </FileRef>\n"
    "</Workspace>\n";
    return stream.str();
}

struct XcodeProject;
struct XcodeObject;
struct XcodeBuildPhase;
struct PBXBuildFile;
struct PBXCopyFilesBuildPhase;
struct PBXFileReference;
struct PBXFrameworksBuildPhase;
struct PBXGroup;
struct PBXNativeTarget;
struct PBXProject;
struct PBXSourcesBuildPhase;
struct XCBuildConfiguration;
struct XCConfigurationList;

std::string encodeComment(const std::string& comment)
{
    return "/* " + comment + " */";
}

std::string encodeDoubleQuotes(const std::string& comment)
{
    return '\"' + comment + '\"';
}

struct Noncopyable {
    Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable & operator=(const Noncopyable&) = delete;
};

struct XcodeObject : Noncopyable {
    virtual ~XcodeObject() = default;
    virtual std::string getUuid() const noexcept = 0;
    virtual std::string isa() const noexcept = 0;
};

struct XcodeBuildPhase : public XcodeObject {
    virtual ~XcodeBuildPhase() = default;
    virtual std::string comments() const noexcept = 0;
};

struct XcodeTargetAttribute final {
    std::shared_ptr<PBXNativeTarget> target;
    Optional<std::string> CreatedOnToolsVersion;
    Optional<std::string> DevelopmentTeam;
    Optional<std::string> TestTargetID;
};

struct XcodeProject final : Noncopyable {
    std::string archiveVersion;
    std::string objectVersion;
    //std::map<std::string, Any> classes;
    //std::map<std::string, std::shared_ptr<XcodeObject>> objects;
    std::shared_ptr<PBXProject> rootObject;
    std::string name;

    std::vector<std::shared_ptr<XCBuildConfiguration>> buildConfigurations;
    std::vector<std::shared_ptr<XCConfigurationList>> configurationLists;
    std::vector<std::shared_ptr<PBXFileReference>> fileReferences;
    std::vector<std::shared_ptr<PBXSourcesBuildPhase>> sourcesBuildPhases;
    std::vector<std::shared_ptr<PBXGroup>> groups;
    std::vector<std::shared_ptr<PBXFrameworksBuildPhase>> frameworkBuildPhases;
    std::vector<std::shared_ptr<PBXCopyFilesBuildPhase>> copyFilesBuildPhases;
    std::vector<std::shared_ptr<PBXNativeTarget>> nativeTargets;
    std::vector<std::shared_ptr<PBXProject>> projects;
};

struct PBXBuildFile final : public XcodeObject {
    std::string isa() const noexcept override { return "PBXBuildFile"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::shared_ptr<PBXFileReference> fileRef;
};

struct PBXFileReference final : public XcodeObject {
    std::string isa() const noexcept override { return "PBXFileReference"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    Optional<std::string> explicitFileType;
    Optional<std::string> includeInIndex;
    Optional<std::string> lastKnownFileType;
    std::string path;
    std::string sourceTree;
};

struct PBXGroup final : public XcodeObject {
    std::string isa() const noexcept override { return "PBXGroup"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::vector<std::shared_ptr<XcodeObject>> children;

    Optional<std::string> name;
    Optional<std::string> path;
    Optional<std::string> sourceTree;

    std::string getSourceTree() const noexcept
    {
        return sourceTree ? *sourceTree : "\"<group>\"";
    }

    std::string getUuidWithComment()
    {
        if (name) {
            return uuid + " " + encodeComment(*name);
        }
        if (path) {
            return uuid + " " + encodeComment(*path);
        }
        return uuid;
    }

    std::vector<std::string> getChildrenString() const
    {
        std::vector<std::string> result;
        for (auto & child : children) {
            if (auto group = std::dynamic_pointer_cast<PBXGroup>(child)) {
                result.push_back(group->getUuidWithComment());
            }
            else if (auto file = std::dynamic_pointer_cast<PBXFileReference>(child)) {
                result.push_back(file->uuid + " " + encodeComment(file->path));
            }
        }
        return std::move(result);
    }
};

struct PBXNativeTarget final : public XcodeObject {
    std::string isa() const noexcept override { return "PBXNativeTarget"; }
    std::string getUuid() const noexcept override { return uuid; }

    std::string uuid;
    std::shared_ptr<XCConfigurationList> buildConfigurationList;
    std::vector<std::shared_ptr<XcodeBuildPhase>> buildPhases;
    std::vector<std::string> buildRules;
    std::vector<std::string> dependencies;
    std::string name;
    std::string productName;
    std::shared_ptr<PBXFileReference> productReference;
    std::string productType;

    std::vector<std::string> getBuildPhasesString() const
    {
        std::vector<std::string> result;
        for (auto & phase : buildPhases) {
            result.push_back(phase->getUuid() + " " + encodeComment(phase->comments()));
        }
        return std::move(result);
    }
};

struct PBXProject final : public XcodeObject {
    std::string isa() const noexcept override { return "PBXProject"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::map<std::string, Any> attributes;
    std::shared_ptr<XCConfigurationList> buildConfigurationList;
    std::string compatibilityVersion;
    std::string developmentRegion;
    std::string hasScannedForEncodings;
    std::vector<std::string> knownRegions;
    std::shared_ptr<PBXGroup> mainGroup;
    std::shared_ptr<PBXGroup> productRefGroup;
    std::string projectDirPath;
    std::string projectRoot;
    std::vector<std::shared_ptr<PBXNativeTarget>> targets;

    std::vector<std::string> getTargetsString() const
    {
        std::vector<std::string> result;
        for (auto & target : targets) {
            result.push_back(target->getUuid() + " " + encodeComment(target->name));
        }
        return std::move(result);
    }

    void addAttribute(const std::string& key, const std::string& value)
    {
        attributes.emplace(key, value);
    }

    void addAttribute(const std::string& key, std::vector<XcodeTargetAttribute> && value)
    {
        attributes.emplace(key, std::move(value));
    }
};

struct PBXCopyFilesBuildPhase final : public XcodeBuildPhase {
    std::string isa() const noexcept override { return "PBXCopyFilesBuildPhase"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::string buildActionMask;
    std::string dstPath;
    std::string dstSubfolderSpec;
    std::string runOnlyForDeploymentPostprocessing;
    std::vector<std::string> files;

    std::string comments() const noexcept override { return "CopyFiles"; }
};

struct PBXFrameworksBuildPhase final : public XcodeBuildPhase {
    std::string isa() const noexcept override { return "PBXFrameworksBuildPhase"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::string buildActionMask;
    std::vector<std::string> files;
    std::string runOnlyForDeploymentPostprocessing;

    std::string comments() const noexcept override { return "Frameworks"; }
};

struct PBXSourcesBuildPhase final : public XcodeBuildPhase {
    std::string isa() const noexcept override { return "PBXSourcesBuildPhase"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::string buildActionMask;
    std::vector<std::shared_ptr<PBXBuildFile>> files;
    std::string runOnlyForDeploymentPostprocessing;

    std::string comments() const noexcept override { return "Sources"; }

    std::vector<std::string> getFileListString() const
    {
        std::vector<std::string> result;
        for (auto & buildFile : files) {
            result.push_back(buildFile->uuid + " " + encodeComment(buildFile->fileRef->path + " in " + comments()));
        }
        return std::move(result);
    }
};

struct XCBuildConfiguration final : public XcodeObject {
    std::string isa() const noexcept override { return "XCBuildConfiguration"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::map<std::string, Any> buildSettings;
    std::string name;

    void addBuildSettings(const std::string& key, const std::string& value)
    {
        buildSettings.emplace(key, value);
    }

    void addBuildSettings(const std::string& key, const std::vector<std::string>& value)
    {
        buildSettings.emplace(key, value);
    }
};

struct XCConfigurationList final : public XcodeObject {
    std::string isa() const noexcept override { return "XCConfigurationList"; }
    std::string getUuid() const noexcept override { return uuid; }
    std::string uuid;
    std::vector<std::shared_ptr<XCBuildConfiguration>> buildConfigurations;
    std::string defaultConfigurationIsVisible;
    Optional<std::string> defaultConfigurationName;

    std::vector<std::string> getBuildConfigurationsString() const
    {
        std::vector<std::string> result;
        for (auto & buildConfig : buildConfigurations) {
            result.push_back(buildConfig->uuid + " " + encodeComment(buildConfig->name));
        }
        return std::move(result);
    }
};

struct XcodePrinterSettings {
    bool isSingleLine = false;
};

class XcodePrinter {
    std::stringstream & stream;
    int tabs;
    std::string section;
    std::vector<XcodePrinterSettings> settingsStack;

public:
    explicit XcodePrinter(std::stringstream & streamIn)
        : stream(streamIn)
        , tabs(0)
    {
        XcodePrinterSettings settings;
        settings.isSingleLine = false;
        settingsStack.push_back(std::move(settings));
    }

    std::string getIndent() const noexcept
    {
        std::string spaces;
        for (int i = 0; i < tabs; ++i) {
            spaces += "\t";
        }
        return std::move(spaces);
    }

    void beginKeyValue(const std::string& key)
    {
        if (!isSingleLine()) {
            stream << getIndent();
        }
        stream << key << " = ";
    }

    void endKeyValue()
    {
        stream << ";";
        if (!isSingleLine()) {
            stream << "\n";
        } else {
            stream << " ";
        }
    }

    void printKeyValue(const std::string& key, const std::string& value)
    {
        beginKeyValue(key);
        stream << value;
        endKeyValue();
    }

    void printKeyValue(const std::string& key, const std::function<void()>& valuePrinter)
    {
        beginKeyValue(key);
        valuePrinter();
        endKeyValue();
    }

    void printKeyValue(const std::string& key, const std::vector<std::string>& array)
    {
        beginKeyValue(key);
        stream << "(";
        if (!isSingleLine()) {
            stream << "\n";
        }
        ++tabs;
        for (auto & value : array) {
            stream << getIndent() << value << ",";
            if (!isSingleLine()) {
                stream << "\n";
            } else {
                stream << " ";
            }
        }
        --tabs;
        stream << getIndent() << ")";
        endKeyValue();
    }

    void beginObject(bool singleLine = false)
    {
        settingsStack.push_back(XcodePrinterSettings{singleLine});
        stream << "{";
        if (!isSingleLine()) {
            stream << "\n";
            ++tabs;
        }
    }

    void endObject()
    {
        if (!isSingleLine()) {
            --tabs;
            stream << getIndent();
        }
        stream << "}";
        settingsStack.pop_back();
    }

    void setTabs(int tabsIn)
    {
        this->tabs = tabsIn;
    }

    bool isSingleLine() const
    {
        assert(!settingsStack.empty());
        return settingsStack.back().isSingleLine;
    }

    void beginSection(const std::string& sectionIn)
    {
        settingsStack.push_back(XcodePrinterSettings{false});
        this->section = sectionIn;
        stream << "\n";
        stream << StringHelper::format("/* Begin %s section */\n",
            section.c_str());
    }

    void endSection()
    {
        stream << StringHelper::format("/* End %s section */\n",
            section.c_str());
        settingsStack.pop_back();
    }
};

template <typename Container, typename Func>
auto findIf(Container & c, Func f)
{
    auto iter = std::find_if(std::begin(c), std::end(c), f);
    assert(iter != std::end(c));
    return *iter;
}

template <typename Container>
auto findByPath(Container & c, const std::string& path)
{
    return findIf(c, [&](const typename Container::value_type& v) {
        return v->path == path;
    });
}

template <typename Container>
auto findByUuid(Container & c, const std::string& uuid)
{
    return findIf(c, [&](const typename Container::value_type& v) {
        return v->uuid == uuid;
    });
}

void printObject(XcodePrinter & printer, const std::map<std::string, Any>& object)
{
    ///@todo sorting by key [A-Z]
    printer.beginObject();
    for (auto & pair : object) {
        auto & key = pair.first;
        auto & value = pair.second;
        if (value.is<std::string>()) {
            printer.printKeyValue(key, value.as<std::string>());
        }
        else if (pair.second.is<std::vector<std::string>>()) {
            printer.printKeyValue(key, value.as<std::vector<std::string>>());
        }
    }
    printer.endObject();
}

std::string findLastKnownFileType(const std::string& path) noexcept
{
    auto ext = std::get<1>(FileSystem::splitExtension(path));
    if (ext == "cpp" || ext == "cxx" || ext == "cc") {
        return "sourcecode.cpp.cpp";
    }
    if (ext == "ii") {
        return "sourcecode.cpp.cpp.preprocessed";
    }
    if (ext == "hpp" || ext == "hxx" || ext == "hh") {
        return "sourcecode.cpp.h";
    }
    if (ext == "mm") {
        return "sourcecode.cpp.objcpp";
    }
    if (ext == "m") {
        return "sourcecode.c.objc";
    }
    if (ext == "m") {
        return "sourcecode.c.objc";
    }
    if (ext == "c") {
        return "sourcecode.c";
    }
    if (ext == "i") {
        return "sourcecode.c.c.preprocessed";
    }
    if (ext == "h") {
        return "sourcecode.c.h";
    }
    if (ext == "sourcecode.swift") {
        return "sourcecode.c.h";
    }
    return "sourcecode";
}

std::shared_ptr<XcodeProject> createXcodeProject(const Xcode::CompileOptions& options)
{
    auto xcodeProject = std::make_shared<XcodeProject>();

    // PBXFileReference
    {
        auto f = std::make_shared<PBXFileReference>();
        f->uuid = "A932DE881BFCD3CC0006E050";
        f->explicitFileType = "\"compiled.mach-o.executable\"";
        f->includeInIndex = "0";
        f->path = options.outputFileName;
        f->sourceTree = "BUILT_PRODUCTS_DIR";
        xcodeProject->fileReferences.push_back(std::move(f));
    }
    {
        auto f = std::make_shared<PBXFileReference>();
        f->uuid = "A932DE8B1BFCD3CC0006E050";
        f->lastKnownFileType = findLastKnownFileType("main.cpp");
        f->path = "main.cpp";
        f->sourceTree = "\"<group>\"";
        xcodeProject->fileReferences.push_back(std::move(f));
    }

    // PBXSourcesBuildPhase
    {
        auto phase = std::make_shared<PBXSourcesBuildPhase>();
        phase->uuid = "A932DE841BFCD3CC0006E050";
        phase->buildActionMask = "2147483647";
        phase->runOnlyForDeploymentPostprocessing = "0";
        {
            auto file = std::make_shared<PBXBuildFile>();
            file->uuid = "A932DE8C1BFCD3CC0006E050";
            file->fileRef = findByPath(xcodeProject->fileReferences, "main.cpp");
            phase->files.push_back(std::move(file));
        }
        xcodeProject->sourcesBuildPhases.push_back(std::move(phase));
    }

    // XCBuildConfiguration
    {
        auto buildConfiguration = std::make_shared<XCBuildConfiguration>();
        auto & config = *buildConfiguration;
        config.uuid = "A932DE8D1BFCD3CC0006E050";
        config.name = "Debug";
        config.addBuildSettings("ALWAYS_SEARCH_USER_PATHS", "NO");
        config.addBuildSettings("CLANG_CXX_LANGUAGE_STANDARD", "\"gnu++0x\"");
        config.addBuildSettings("CLANG_CXX_LIBRARY", "\"libc++\"");
        config.addBuildSettings("CLANG_ENABLE_MODULES", "YES");
        config.addBuildSettings("CLANG_ENABLE_OBJC_ARC", "YES");
        config.addBuildSettings("CLANG_WARN_BOOL_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_CONSTANT_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_DIRECT_OBJC_ISA_USAGE", "YES_ERROR");
        config.addBuildSettings("CLANG_WARN_EMPTY_BODY", "YES");
        config.addBuildSettings("CLANG_WARN_ENUM_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_INT_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_OBJC_ROOT_CLASS", "YES_ERROR");
        config.addBuildSettings("CLANG_WARN_UNREACHABLE_CODE", "YES");
        config.addBuildSettings("CLANG_WARN__DUPLICATE_METHOD_MATCH", "YES");
        config.addBuildSettings("CODE_SIGN_IDENTITY", "\"-\"");
        config.addBuildSettings("COPY_PHASE_STRIP", "NO");
        config.addBuildSettings("DEBUG_INFORMATION_FORMAT", "dwarf");
        config.addBuildSettings("ENABLE_STRICT_OBJC_MSGSEND", "YES");
        config.addBuildSettings("ENABLE_TESTABILITY", "YES");
        config.addBuildSettings("GCC_C_LANGUAGE_STANDARD", "gnu99");
        config.addBuildSettings("GCC_DYNAMIC_NO_PIC", "NO");
        config.addBuildSettings("GCC_NO_COMMON_BLOCKS", "YES");
        config.addBuildSettings("GCC_OPTIMIZATION_LEVEL", "0");
        config.addBuildSettings("GCC_PREPROCESSOR_DEFINITIONS", std::vector<std::string>{
            "\"DEBUG=1\"",
            "\"$(inherited)\"",
        });
        config.addBuildSettings("GCC_WARN_64_TO_32_BIT_CONVERSION", "YES");
        config.addBuildSettings("GCC_WARN_ABOUT_RETURN_TYPE", "YES_ERROR");
        config.addBuildSettings("GCC_WARN_UNDECLARED_SELECTOR", "YES");
        config.addBuildSettings("GCC_WARN_UNINITIALIZED_AUTOS", "YES_AGGRESSIVE");
        config.addBuildSettings("GCC_WARN_UNUSED_FUNCTION", "YES");
        config.addBuildSettings("GCC_WARN_UNUSED_VARIABLE", "YES");
        config.addBuildSettings("MACOSX_DEPLOYMENT_TARGET", "10.11");
        config.addBuildSettings("MTL_ENABLE_DEBUG_INFO", "YES");
        config.addBuildSettings("ONLY_ACTIVE_ARCH", "YES");
        config.addBuildSettings("SDKROOT", "macosx");
        xcodeProject->buildConfigurations.push_back(std::move(buildConfiguration));
    }
    {
        auto buildConfiguration = std::make_shared<XCBuildConfiguration>();
        auto & config = *buildConfiguration;
        config.uuid = "A932DE8E1BFCD3CC0006E050";
        config.name = "Release";
        config.addBuildSettings("ALWAYS_SEARCH_USER_PATHS", "NO");
        config.addBuildSettings("CLANG_CXX_LANGUAGE_STANDARD", "\"gnu++0x\"");
        config.addBuildSettings("CLANG_CXX_LIBRARY", "\"libc++\"");
        config.addBuildSettings("CLANG_ENABLE_MODULES", "YES");
        config.addBuildSettings("CLANG_ENABLE_OBJC_ARC", "YES");
        config.addBuildSettings("CLANG_WARN_BOOL_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_CONSTANT_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_DIRECT_OBJC_ISA_USAGE", "YES_ERROR");
        config.addBuildSettings("CLANG_WARN_EMPTY_BODY", "YES");
        config.addBuildSettings("CLANG_WARN_ENUM_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_INT_CONVERSION", "YES");
        config.addBuildSettings("CLANG_WARN_OBJC_ROOT_CLASS", "YES_ERROR");
        config.addBuildSettings("CLANG_WARN_UNREACHABLE_CODE", "YES");
        config.addBuildSettings("CLANG_WARN__DUPLICATE_METHOD_MATCH", "YES");
        config.addBuildSettings("CODE_SIGN_IDENTITY", "\"-\"");
        config.addBuildSettings("COPY_PHASE_STRIP", "NO");
        config.addBuildSettings("DEBUG_INFORMATION_FORMAT", "\"dwarf-with-dsym\"");
        config.addBuildSettings("ENABLE_NS_ASSERTIONS", "NO");
        config.addBuildSettings("ENABLE_STRICT_OBJC_MSGSEND", "YES");
        config.addBuildSettings("GCC_C_LANGUAGE_STANDARD", "gnu99");
        config.addBuildSettings("GCC_NO_COMMON_BLOCKS", "YES");
        config.addBuildSettings("GCC_WARN_64_TO_32_BIT_CONVERSION", "YES");
        config.addBuildSettings("GCC_WARN_ABOUT_RETURN_TYPE", "YES_ERROR");
        config.addBuildSettings("GCC_WARN_UNDECLARED_SELECTOR", "YES");
        config.addBuildSettings("GCC_WARN_UNINITIALIZED_AUTOS", "YES_AGGRESSIVE");
        config.addBuildSettings("GCC_WARN_UNUSED_FUNCTION", "YES");
        config.addBuildSettings("GCC_WARN_UNUSED_VARIABLE", "YES");
        config.addBuildSettings("MACOSX_DEPLOYMENT_TARGET", "10.11");
        config.addBuildSettings("MTL_ENABLE_DEBUG_INFO", "NO");
        config.addBuildSettings("SDKROOT", "macosx");
        xcodeProject->buildConfigurations.push_back(std::move(buildConfiguration));
    }
    {
        auto buildConfiguration = std::make_shared<XCBuildConfiguration>();
        auto & config = *buildConfiguration;
        config.uuid = "A932DE901BFCD3CC0006E050";
        config.name = "Debug";
        config.addBuildSettings("PRODUCT_NAME", "\"$(TARGET_NAME)\"");
        xcodeProject->buildConfigurations.push_back(std::move(buildConfiguration));
    }
    {
        auto buildConfiguration = std::make_shared<XCBuildConfiguration>();
        auto & config = *buildConfiguration;
        config.uuid = "A932DE911BFCD3CC0006E050";
        config.name = "Release";
        config.addBuildSettings("PRODUCT_NAME", "\"$(TARGET_NAME)\"");
        xcodeProject->buildConfigurations.push_back(std::move(buildConfiguration));
    }

    // XCConfigurationList
    {
        auto configurationList = std::make_shared<XCConfigurationList>();
        configurationList->uuid = "A932DE831BFCD3CC0006E050";
        configurationList->buildConfigurations.push_back(findByUuid(xcodeProject->buildConfigurations, "A932DE8D1BFCD3CC0006E050"));
        configurationList->buildConfigurations.push_back(findByUuid(xcodeProject->buildConfigurations, "A932DE8E1BFCD3CC0006E050"));
        configurationList->defaultConfigurationIsVisible = "0";
        configurationList->defaultConfigurationName = "Release";
        xcodeProject->configurationLists.push_back(std::move(configurationList));
    }
    {
        auto configurationList = std::make_shared<XCConfigurationList>();
        configurationList->uuid = "A932DE8F1BFCD3CC0006E050";
        configurationList->buildConfigurations.push_back(findByUuid(xcodeProject->buildConfigurations, "A932DE901BFCD3CC0006E050"));
        configurationList->buildConfigurations.push_back(findByUuid(xcodeProject->buildConfigurations, "A932DE911BFCD3CC0006E050"));
        configurationList->defaultConfigurationIsVisible = "0";
        configurationList->defaultConfigurationName = "Release";
        xcodeProject->configurationLists.push_back(std::move(configurationList));
    }

    // PBXGroup
    {
        auto group = std::make_shared<PBXGroup>();
        group->uuid = "A932DE891BFCD3CC0006E050";
        group->children.push_back(findByUuid(xcodeProject->fileReferences, "A932DE881BFCD3CC0006E050"));
        group->name = "Products";
        xcodeProject->groups.push_back(std::move(group));
    }
    {
        auto group = std::make_shared<PBXGroup>();
        group->uuid = "A932DE8A1BFCD3CC0006E050";
        group->children.push_back(findByUuid(xcodeProject->fileReferences, "A932DE8B1BFCD3CC0006E050"));
        group->name = "Source";
        xcodeProject->groups.push_back(std::move(group));
    }
    {
        auto group = std::make_shared<PBXGroup>();
        group->uuid = "A932DE7F1BFCD3CC0006E050";
        group->children.push_back(findByUuid(xcodeProject->groups, "A932DE8A1BFCD3CC0006E050"));
        group->children.push_back(findByUuid(xcodeProject->groups, "A932DE891BFCD3CC0006E050"));
        xcodeProject->groups.push_back(std::move(group));
    }

    // PBXFrameworksBuildPhase
    {
        auto phase = std::make_shared<PBXFrameworksBuildPhase>();
        phase->uuid = "A932DE851BFCD3CC0006E050";
        phase->buildActionMask = "2147483647";
        phase->runOnlyForDeploymentPostprocessing = "0";
        xcodeProject->frameworkBuildPhases.push_back(std::move(phase));
    }

    // PBXCopyFilesBuildPhase
    {
        auto phase = std::make_shared<PBXCopyFilesBuildPhase>();
        phase->uuid = "A932DE861BFCD3CC0006E050";
        phase->buildActionMask = "2147483647";
        phase->dstPath = "/usr/share/man/man1/";
        phase->dstSubfolderSpec = "0";
        phase->runOnlyForDeploymentPostprocessing = "1";
        xcodeProject->copyFilesBuildPhases.push_back(std::move(phase));
    }

    // PBXNativeTarget
    {
        auto target = std::make_shared<PBXNativeTarget>();
        target->uuid = "A932DE871BFCD3CC0006E050";
        target->buildConfigurationList = findByUuid(xcodeProject->configurationLists, "A932DE8F1BFCD3CC0006E050");
        target->buildPhases.push_back(findByUuid(xcodeProject->sourcesBuildPhases, "A932DE841BFCD3CC0006E050"));
        target->buildPhases.push_back(findByUuid(xcodeProject->frameworkBuildPhases, "A932DE851BFCD3CC0006E050"));
        target->buildPhases.push_back(findByUuid(xcodeProject->copyFilesBuildPhases, "A932DE861BFCD3CC0006E050"));
        target->name = options.outputFileName;
        target->productName = options.outputFileName;
        target->productReference = findByUuid(xcodeProject->fileReferences, "A932DE881BFCD3CC0006E050");
        target->productType = "\"com.apple.product-type.tool\"";
        xcodeProject->nativeTargets.push_back(std::move(target));
    }

    // PBXProject
    {
        auto project = std::make_shared<PBXProject>();
        project->uuid = "A932DE801BFCD3CC0006E050";
        project->buildConfigurationList = findByUuid(xcodeProject->configurationLists, "A932DE831BFCD3CC0006E050");
        project->compatibilityVersion = "\"Xcode 3.2\"";
        project->developmentRegion = "English";
        project->hasScannedForEncodings = "0";
        project->knownRegions = {"en"};
        project->mainGroup = findByUuid(xcodeProject->groups, "A932DE7F1BFCD3CC0006E050");
        project->productRefGroup = findByUuid(xcodeProject->groups, "A932DE891BFCD3CC0006E050");
        project->projectDirPath = "\"\"";
        project->projectRoot = "\"\"";
        project->targets.push_back(findByUuid(xcodeProject->nativeTargets, "A932DE871BFCD3CC0006E050"));

        project->addAttribute("LastUpgradeCheck", "0710");
        project->addAttribute("ORGANIZATIONNAME", "mogemimi");

        std::vector<XcodeTargetAttribute> targetAttributes;
        {
            XcodeTargetAttribute attribute;
            attribute.target = findByUuid(xcodeProject->nativeTargets, "A932DE871BFCD3CC0006E050");
            attribute.CreatedOnToolsVersion = "7.1.1";
            targetAttributes.push_back(std::move(attribute));
        }
        project->addAttribute("TargetAttributes", std::move(targetAttributes));

        xcodeProject->projects.push_back(std::move(project));
    }

    xcodeProject->name = options.outputFileName;
    xcodeProject->archiveVersion = "1";
    xcodeProject->objectVersion = "46";
    xcodeProject->rootObject = findByUuid(xcodeProject->projects, "A932DE801BFCD3CC0006E050");

    std::sort(std::begin(xcodeProject->groups), std::end(xcodeProject->groups),
        [](const std::shared_ptr<PBXGroup>& a, const std::shared_ptr<PBXGroup>& b) {
            auto cost = [](const PBXGroup& group) {
                int c = 0;
                if (!group.name && !group.path) {
                    c += 42;
                }
                for (auto & child : group.children) {
                    if (child->isa() == "PBXGroup") {
                        c += 1;
                    }
                }
                return c;
            };
            return cost(*a) >= cost(*b);
        });

    return std::move(xcodeProject);
}

void printObjects(XcodePrinter & printer, const XcodeProject& xcodeProject)
{
    constexpr bool isSingleLine = true;

    printer.beginSection("PBXBuildFile");
    for (auto & phase : xcodeProject.sourcesBuildPhases) {
        for (auto & f : phase->files) {
            auto & buildFile = *f;
            printer.beginKeyValue(
                buildFile.uuid
                + " "
                + encodeComment(buildFile.fileRef->path + " in " + phase->comments()));
                printer.beginObject(isSingleLine);
                printer.printKeyValue("isa", buildFile.isa());
                printer.printKeyValue("fileRef",
                    buildFile.fileRef->uuid + " " + encodeComment(buildFile.fileRef->path));
                printer.endObject();
            printer.endKeyValue();
        }
    }
    printer.endSection();

    printer.beginSection("PBXCopyFilesBuildPhase");
    for (auto & phase : xcodeProject.copyFilesBuildPhases) {
        printer.beginKeyValue(phase->uuid + " " + encodeComment(phase->comments()));
            printer.beginObject();
            printer.printKeyValue("isa", phase->isa());
            printer.printKeyValue("buildActionMask", phase->buildActionMask);
            printer.printKeyValue("dstPath", phase->dstPath);
            printer.printKeyValue("dstSubfolderSpec", phase->dstSubfolderSpec);
            printer.printKeyValue("files", phase->files);
            printer.printKeyValue("runOnlyForDeploymentPostprocessing",
                phase->runOnlyForDeploymentPostprocessing);
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("PBXFileReference");
    for (auto & f : xcodeProject.fileReferences) {
        auto & fileRef = *f;
        printer.beginKeyValue(fileRef.uuid + " " + encodeComment(fileRef.path));
            printer.beginObject(isSingleLine);
                printer.printKeyValue("isa", fileRef.isa());
                if (fileRef.explicitFileType) {
                    printer.printKeyValue("explicitFileType", *fileRef.explicitFileType);
                }
                if (fileRef.includeInIndex) {
                    printer.printKeyValue("includeInIndex", *fileRef.includeInIndex);
                }
                if (fileRef.lastKnownFileType) {
                    printer.printKeyValue("lastKnownFileType", *fileRef.lastKnownFileType);
                }
                printer.printKeyValue("path", fileRef.path);
                printer.printKeyValue("sourceTree", fileRef.sourceTree);
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("PBXFrameworksBuildPhase");
    for (auto & phase : xcodeProject.frameworkBuildPhases) {
        printer.beginKeyValue(phase->getUuid() + " " + encodeComment(phase->comments()));
            printer.beginObject();
                printer.printKeyValue("isa", phase->isa());
                printer.printKeyValue("buildActionMask", phase->buildActionMask);
                printer.printKeyValue("files", phase->files);
                printer.printKeyValue("runOnlyForDeploymentPostprocessing",
                    phase->runOnlyForDeploymentPostprocessing);
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("PBXGroup");
    for (auto & group : xcodeProject.groups) {
        printer.beginKeyValue(group->getUuidWithComment());
            printer.beginObject();
                printer.printKeyValue("isa", group->isa());
                printer.printKeyValue("children", group->getChildrenString());
                if (group->name) {
                    printer.printKeyValue("name", *group->name);
                }
                if (group->path) {
                    printer.printKeyValue("path", *group->path);
                }
                printer.printKeyValue("sourceTree", group->getSourceTree());
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("PBXNativeTarget");
    for (auto & target : xcodeProject.nativeTargets) {
        printer.beginKeyValue(target->uuid + " " + encodeComment(target->name));
            printer.beginObject();
                printer.printKeyValue("isa", target->isa());
                printer.printKeyValue("buildConfigurationList",
                    target->buildConfigurationList->uuid
                    + " "
                    + encodeComment("Build configuration list for " + target->isa() + " \""+ target->name +"\""));
                printer.printKeyValue("buildPhases", target->getBuildPhasesString());
                printer.printKeyValue("buildRules", target->buildRules);
                printer.printKeyValue("dependencies", target->dependencies);
                printer.printKeyValue("name", target->name);
                printer.printKeyValue("productName", target->productName);
                printer.printKeyValue("productReference",
                    target->productReference->uuid + " " + encodeComment(target->productReference->path));
                printer.printKeyValue("productType", target->productType);
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("PBXProject");
    for (auto & project : xcodeProject.projects) {
        printer.beginKeyValue(project->uuid + " " + encodeComment("Project object"));
            printer.beginObject();
                printer.printKeyValue("isa", project->isa());
                printer.printKeyValue("attributes", [&] {
                    printer.beginObject();
                    for (auto & pair : project->attributes) {
                        if (pair.second.is<std::string>()) {
                            printer.printKeyValue(pair.first, pair.second.as<std::string>());
                        }
                        else if (pair.second.is<std::vector<XcodeTargetAttribute>>()) {
                            auto & targetAttributes = pair.second.as<std::vector<XcodeTargetAttribute>>();
                            printer.printKeyValue(pair.first, [&] {
                                printer.beginObject();
                                for (auto & targetAttribute : targetAttributes) {
                                    printer.beginKeyValue(targetAttribute.target->uuid);
                                        printer.beginObject();
                                            if (targetAttribute.CreatedOnToolsVersion) {
                                                printer.printKeyValue("CreatedOnToolsVersion",
                                                    *targetAttribute.CreatedOnToolsVersion);
                                            }
                                            if (targetAttribute.DevelopmentTeam) {
                                                printer.printKeyValue("DevelopmentTeam",
                                                    *targetAttribute.DevelopmentTeam);
                                            }
                                            if (targetAttribute.TestTargetID) {
                                                printer.printKeyValue("TestTargetID",
                                                    *targetAttribute.TestTargetID);
                                            }
                                        printer.endObject();
                                    printer.endKeyValue();
                                }
                                printer.endObject();
                            });
                        }
                    }
                    printer.endObject();
                });
                printer.printKeyValue("buildConfigurationList",
                    project->buildConfigurationList->uuid
                    + " "
                    + encodeComment("Build configuration list for PBXProject "
                    + encodeDoubleQuotes(xcodeProject.name)));
                printer.printKeyValue("compatibilityVersion", project->compatibilityVersion);
                printer.printKeyValue("developmentRegion", project->developmentRegion);
                printer.printKeyValue("hasScannedForEncodings", project->hasScannedForEncodings);
                printer.printKeyValue("knownRegions", project->knownRegions);
                printer.printKeyValue("mainGroup", project->mainGroup->uuid);
                printer.printKeyValue("productRefGroup",
                    project->productRefGroup->uuid + " " + encodeComment("Products"));
                printer.printKeyValue("projectDirPath", project->projectDirPath);
                printer.printKeyValue("projectRoot", project->projectRoot);
                printer.printKeyValue("targets", project->getTargetsString());
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("PBXSourcesBuildPhase");
    for (auto & phase : xcodeProject.sourcesBuildPhases) {
        printer.beginKeyValue(phase->uuid + " " + encodeComment(phase->comments()));
            printer.beginObject();
                printer.printKeyValue("isa", phase->isa());
                printer.printKeyValue("buildActionMask", phase->buildActionMask);
                printer.printKeyValue("files", phase->getFileListString());
                printer.printKeyValue("runOnlyForDeploymentPostprocessing",
                    phase->runOnlyForDeploymentPostprocessing);
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("XCBuildConfiguration");
    for (auto & config : xcodeProject.buildConfigurations) {
        printer.printKeyValue(config->uuid + " " + encodeComment(config->name), [&] {
            printer.beginObject();
                printer.printKeyValue("isa", config->isa());
                printer.printKeyValue("buildSettings", [&] {
                    printObject(printer, config->buildSettings);
                });
                printer.printKeyValue("name", config->name);
            printer.endObject();
        });
    }
    printer.endSection();

    printer.beginSection("XCConfigurationList");
    for (auto & configurationList : xcodeProject.configurationLists) {
        printer.beginKeyValue(
            configurationList->uuid
            + " "
            + encodeComment("Build configuration list for PBXProject "
            + encodeDoubleQuotes(xcodeProject.name)));
            printer.beginObject();
                printer.printKeyValue("isa", configurationList->isa());
                printer.printKeyValue("buildConfigurations",
                    configurationList->getBuildConfigurationsString());
                printer.printKeyValue("defaultConfigurationIsVisible",
                    configurationList->defaultConfigurationIsVisible);
                if (configurationList->defaultConfigurationName) {
                    printer.printKeyValue("defaultConfigurationName",
                        *configurationList->defaultConfigurationName);
                }
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();
}

std::string generatePbxproj(const XcodeProject& xcodeProject)
{
    using StringHelper::format;

    std::stringstream stream;
    stream << "// !$*UTF8*$!\n";

    XcodePrinter printer(stream);
    printer.beginObject();
        printer.printKeyValue("archiveVersion", xcodeProject.archiveVersion);
        printer.beginKeyValue("classes");
            printer.beginObject();
            printer.endObject();
        printer.endKeyValue();
        printer.printKeyValue("objectVersion", xcodeProject.objectVersion);
        printer.beginKeyValue("objects");
            printer.beginObject();
            printObjects(printer, xcodeProject);
            printer.endObject();
        printer.endKeyValue();
        printer.printKeyValue("rootObject",
            xcodeProject.rootObject->uuid + " " + encodeComment("Project object"));
    printer.endObject();

    stream << "\n";
    return stream.str();
}

} // unnamed namespace

GeneratorError Xcode::generateXcodeProject(const CompileOptions& options)
{
    namespace FileSystem = somera::FileSystem;

    const auto xcodeprojPath = FileSystem::join(
        options.generatorOutputDirectory, options.outputFileName + ".xcodeproj");

    if (FileSystem::exists(xcodeprojPath)) {
        if (!FileSystem::isDirectory(xcodeprojPath)) {
            return GeneratorError("Error: A file with the name '" + xcodeprojPath + "' already exists.");
        }
    }
    else {
        FileSystem::createDirectories(xcodeprojPath);
    }

    const auto xcworkspacePath = FileSystem::join(xcodeprojPath, "project.xcworkspace");
    if (!FileSystem::exists(xcworkspacePath)) {
        FileSystem::createDirectories(xcworkspacePath);
    }

    auto xcodeProject = createXcodeProject(options);
    {
        const auto pbxprojPath = FileSystem::join(xcodeprojPath, "project.pbxproj");
        std::ofstream stream(pbxprojPath);
        if (!stream) {
            return GeneratorError("Error: Cannot open.");
        }
        stream << generatePbxproj(*xcodeProject);
    }
    {
        const auto pbxprojPath = FileSystem::join(xcworkspacePath, "contents.xcworkspacedata");
        std::ofstream stream(pbxprojPath);
        if (!stream) {
            return GeneratorError("Error: Cannot open.");
        }
        auto xcodeprojName = std::get<1>(FileSystem::split(xcodeprojPath));
        stream << generateXCWorkSpaceData(xcodeprojName);
    }
    return {};
}

} // namespace somera
