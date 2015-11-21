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

struct PBXBuildFile final {
    std::string uuid;
    std::shared_ptr<PBXFileReference> fileRef;

    std::string isa() const noexcept { return "PBXBuildFile"; }
};

struct PBXCopyFilesBuildPhase final {
    std::string isa() const noexcept { return "PBXCopyFilesBuildPhase"; }
};

struct PBXFileReference final {
    std::string uuid;
    Optional<std::string> explicitFileType;
    Optional<std::string> includeInIndex;
    Optional<std::string> lastKnownFileType;
    std::string path;
    std::string sourceTree;

    std::string isa() const noexcept { return "PBXFileReference"; }
};

struct PBXFrameworksBuildPhase final {
    std::string isa() const noexcept { return "PBXFrameworksBuildPhase"; }
};

struct PBXGroup final {
    std::string uuid;

    // TODO: replace with std::vector<any<std::shared_ptr<PBXFileReference>, std::shared_ptr<PBXGroup>>>
    std::vector<std::shared_ptr<PBXFileReference>> children;

    Optional<std::string> name;
    Optional<std::string> path;
    std::string sourceTree;

    std::string isa() const noexcept { return "PBXGroup"; }
};

struct PBXNativeTarget final {
    std::string isa() const noexcept { return "PBXNativeTarget"; }
};

struct PBXProject final {
    std::string isa() const noexcept { return "PBXProject"; }
};

struct PBXSourcesBuildPhase final {
    std::string uuid;
    std::string isa() const noexcept { return "PBXSourcesBuildPhase"; }
    std::string buildActionMask;
    std::string comments;
    std::vector<std::shared_ptr<PBXBuildFile>> files;
    std::string runOnlyForDeploymentPostprocessing;

    std::vector<std::string> getFileListString() const
    {
        std::vector<std::string> result;
        for (auto & buildFile : files) {
            result.push_back(buildFile->uuid + " " + encodeComment(buildFile->fileRef->path + " in " + comments));
        }
        return std::move(result);
    }
};

struct XCBuildConfiguration final {
    std::string uuid;
    std::string isa() const noexcept { return "XCBuildConfiguration"; }
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

struct XCConfigurationList final {
    std::string isa() const noexcept { return "XCConfigurationList"; }
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

void printObject(XcodePrinter & printer, const std::map<std::string, Any>& object)
{
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

void printObjects(XcodePrinter & printer)
{
    constexpr bool isSingleLine = true;

#if 1 // ayafuya rocket~~~~~~~
    std::vector<std::shared_ptr<PBXFileReference>> pbxFileReferenceList;
    {
        auto f = std::make_shared<PBXFileReference>();
        f->uuid = "A932DE881BFCD3CC0006E050";
        f->explicitFileType = "\"compiled.mach-o.executable\"";
        f->includeInIndex = "0";
        f->path = "MyHidamari";
        f->sourceTree = "BUILT_PRODUCTS_DIR";
        pbxFileReferenceList.push_back(std::move(f));
    }
    {
        auto f = std::make_shared<PBXFileReference>();
        f->uuid = "A932DE8B1BFCD3CC0006E050";
        f->lastKnownFileType = "sourcecode.cpp.cpp";
        f->path = "main.cpp";
        f->sourceTree = "\"<group>\"";
        pbxFileReferenceList.push_back(std::move(f));
    }

    std::vector<std::shared_ptr<PBXSourcesBuildPhase>> pbxSourcesBuildPhaseList;
    {
        auto phase = std::make_shared<PBXSourcesBuildPhase>();
        phase->uuid = "A932DE841BFCD3CC0006E050";
        phase->buildActionMask = "2147483647";
        phase->runOnlyForDeploymentPostprocessing = "0";
        phase->comments = "Sources";
        {
            auto file = std::make_shared<PBXBuildFile>();
            file->uuid = "A932DE8C1BFCD3CC0006E050";
            file->fileRef = findByPath(pbxFileReferenceList, "main.cpp");
            phase->files.push_back(std::move(file));
        }
        pbxSourcesBuildPhaseList.push_back(std::move(phase));
    }

    std::vector<XCBuildConfiguration> buildConfigurations;
    {
        XCBuildConfiguration config;
        config.uuid = "A932DE8D1BFCD3CC0006E050";
        config.name = "Debug";
        config.addBuildSettings("PRODUCT_NAME", "\"$(TARGET_NAME)\"");
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
        buildConfigurations.push_back(std::move(config));
    }
    {
        XCBuildConfiguration config;
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
        buildConfigurations.push_back(std::move(config));
    }
    {
        XCBuildConfiguration config;
        config.uuid = "A932DE901BFCD3CC0006E050";
        config.name = "Debug";
        config.addBuildSettings("PRODUCT_NAME", "\"$(TARGET_NAME)\"");
        buildConfigurations.push_back(std::move(config));
    }
    {
        XCBuildConfiguration config;
        config.uuid = "A932DE911BFCD3CC0006E050";
        config.name = "Release";
        config.addBuildSettings("PRODUCT_NAME", "\"$(TARGET_NAME)\"");
        buildConfigurations.push_back(std::move(config));
    }

#endif // ayafuya rocket~~~~~~~

    printer.beginSection("PBXBuildFile");
    for (auto & buildPhase : pbxSourcesBuildPhaseList) {
        for (auto & f : buildPhase->files) {
            auto & buildFile = *f;
            printer.beginKeyValue(buildFile.uuid + " " + encodeComment(buildFile.fileRef->path + " in " + buildPhase->comments));
                printer.beginObject(isSingleLine);
                printer.printKeyValue("isa", buildFile.isa());
                printer.printKeyValue("fileRef", buildFile.fileRef->uuid + " " + encodeComment(buildFile.fileRef->path));
                printer.endObject();
            printer.endKeyValue();
        }
    }
    printer.endSection();

    printer.beginSection("PBXCopyFilesBuildPhase");
        printer.beginKeyValue("A932DE861BFCD3CC0006E050 /* CopyFiles */");
        printer.beginObject();
        printer.printKeyValue("isa", "PBXCopyFilesBuildPhase");
        printer.printKeyValue("buildActionMask", "2147483647");
        printer.printKeyValue("dstPath", "/usr/share/man/man1/");
        printer.printKeyValue("dstSubfolderSpec", "0");
        printer.printKeyValue("files", std::vector<std::string>{});
        printer.printKeyValue("runOnlyForDeploymentPostprocessing", "1");
        printer.endObject();
        printer.endKeyValue();
    printer.endSection();

    printer.beginSection("PBXFileReference");
    for (auto & f : pbxFileReferenceList) {
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
        printer.beginKeyValue("A932DE851BFCD3CC0006E050 /* Frameworks */");
            printer.beginObject();
                printer.printKeyValue("isa", "PBXFrameworksBuildPhase");
                printer.printKeyValue("buildActionMask", "2147483647");
                printer.printKeyValue("files", std::vector<std::string>{});
                printer.printKeyValue("runOnlyForDeploymentPostprocessing", "0");
            printer.endObject();
        printer.endKeyValue();
    printer.endSection();

    printer.beginSection("PBXGroup");
        printer.beginKeyValue("A932DE7F1BFCD3CC0006E050");
            printer.beginObject();
                printer.printKeyValue("isa", "PBXGroup");
                printer.printKeyValue("children", std::vector<std::string>{
                    "A932DE8A1BFCD3CC0006E050 /* MyHidamari */",
                    "A932DE891BFCD3CC0006E050 /* Products */",
                });
                printer.printKeyValue("sourceTree", "\"<group>\"");
            printer.endObject();
        printer.endKeyValue();

        printer.beginKeyValue("A932DE891BFCD3CC0006E050 /* Products */");
            printer.beginObject();
                printer.printKeyValue("isa", "PBXGroup");
                printer.printKeyValue("children", std::vector<std::string>{
                    "A932DE881BFCD3CC0006E050 /* MyHidamari */",
                });
                printer.printKeyValue("name", "Products");
                printer.printKeyValue("sourceTree", "\"<group>\"");
            printer.endObject();
        printer.endKeyValue();

        printer.beginKeyValue("A932DE8A1BFCD3CC0006E050 /* MyHidamari */");
            printer.beginObject();
                printer.printKeyValue("isa", "PBXGroup");
                printer.printKeyValue("children", std::vector<std::string>{
                    "A932DE8B1BFCD3CC0006E050 /* main.cpp */",
                });
                printer.printKeyValue("path", "MyHidamari");
                printer.printKeyValue("sourceTree", "\"<group>\"");
            printer.endObject();
        printer.endKeyValue();
    printer.endSection();

    printer.beginSection("PBXNativeTarget");
        printer.beginKeyValue("A932DE871BFCD3CC0006E050 /* MyHidamari */");
            printer.beginObject();
                printer.printKeyValue("isa", "PBXNativeTarget");
                printer.printKeyValue("buildConfigurationList", "A932DE8F1BFCD3CC0006E050 /* Build configuration list for PBXNativeTarget \"MyHidamari\" */");
                printer.printKeyValue("buildPhases", std::vector<std::string>{
                    "A932DE841BFCD3CC0006E050 /* Sources */",
                    "A932DE851BFCD3CC0006E050 /* Frameworks */",
                    "A932DE861BFCD3CC0006E050 /* CopyFiles */",
                });
                printer.printKeyValue("buildRules", std::vector<std::string>{});
                printer.printKeyValue("dependencies", std::vector<std::string>{});
                printer.printKeyValue("name", "MyHidamari");
                printer.printKeyValue("productName", "MyHidamari");
                printer.printKeyValue("productReference", "A932DE881BFCD3CC0006E050 /* MyHidamari */");
                printer.printKeyValue("productType", "\"com.apple.product-type.tool\"");
            printer.endObject();
        printer.endKeyValue();
    printer.endSection();

    printer.beginSection("PBXProject");
        printer.beginKeyValue("A932DE801BFCD3CC0006E050 /* Project object */");
            printer.beginObject();
                printer.printKeyValue("isa", "PBXProject");
                printer.beginKeyValue("attributes");
                    printer.beginObject();
                        printer.printKeyValue("LastUpgradeCheck", "0710");
                        printer.printKeyValue("ORGANIZATIONNAME", "mogemimi");
                        printer.beginKeyValue("TargetAttributes");
                            printer.beginObject();
                                printer.beginKeyValue("A932DE871BFCD3CC0006E050");
                                    printer.beginObject();
                                        printer.printKeyValue("CreatedOnToolsVersion", "7.1.1");
                                    printer.endObject();
                                printer.endKeyValue();
                            printer.endObject();
                        printer.endKeyValue();
                    printer.endObject();
                printer.endKeyValue();
                printer.printKeyValue("buildConfigurationList", "A932DE831BFCD3CC0006E050 /* Build configuration list for PBXProject \"MyHidamari\" */");
                printer.printKeyValue("compatibilityVersion", "\"Xcode 3.2\"");
                printer.printKeyValue("developmentRegion", "English");
                printer.printKeyValue("hasScannedForEncodings", "0");
                printer.printKeyValue("knownRegions", std::vector<std::string>{"en"});
                printer.printKeyValue("mainGroup", "A932DE7F1BFCD3CC0006E050");
                printer.printKeyValue("productRefGroup", "A932DE891BFCD3CC0006E050 /* Products */");
                printer.printKeyValue("projectDirPath", "\"\"");
                printer.printKeyValue("projectRoot", "\"\"");
                printer.printKeyValue("targets", std::vector<std::string>{
                    "A932DE871BFCD3CC0006E050 /* MyHidamari */",
                });
            printer.endObject();
        printer.endKeyValue();
    printer.endSection();

    printer.beginSection("PBXSourcesBuildPhase");
    for (auto & buildPhase : pbxSourcesBuildPhaseList) {
        printer.beginKeyValue(buildPhase->uuid + " " + encodeComment(buildPhase->comments));
            printer.beginObject();
                printer.printKeyValue("isa", buildPhase->isa());
                printer.printKeyValue("buildActionMask", buildPhase->buildActionMask);
                printer.printKeyValue("files", buildPhase->getFileListString());
                printer.printKeyValue("runOnlyForDeploymentPostprocessing", buildPhase->runOnlyForDeploymentPostprocessing);
            printer.endObject();
        printer.endKeyValue();
    }
    printer.endSection();

    printer.beginSection("XCBuildConfiguration");
    for (auto & config : buildConfigurations) {
        printer.printKeyValue(config.uuid + " " + encodeComment(config.name), [&] {
            printer.beginObject();
                printer.printKeyValue("isa", config.isa());
                printer.printKeyValue("buildSettings", [&] {
                    printObject(printer, config.buildSettings);
                });
                printer.printKeyValue("name", config.name);
            printer.endObject();
        });
    }
    printer.endSection();

    printer.beginSection("XCConfigurationList");
        printer.beginKeyValue("A932DE831BFCD3CC0006E050 /* Build configuration list for PBXProject \"MyHidamari\" */");
            printer.beginObject();
                printer.printKeyValue("isa", "XCConfigurationList");
                printer.printKeyValue("buildConfigurations", std::vector<std::string>{
                    "A932DE8D1BFCD3CC0006E050 /* Debug */",
                    "A932DE8E1BFCD3CC0006E050 /* Release */",
                });
                printer.printKeyValue("defaultConfigurationIsVisible", "0");
                printer.printKeyValue("defaultConfigurationName", "Release");
            printer.endObject();
        printer.endKeyValue();

        printer.beginKeyValue("A932DE8F1BFCD3CC0006E050 /* Build configuration list for PBXNativeTarget \"MyHidamari\" */");
            printer.beginObject();
                printer.printKeyValue("isa", "XCConfigurationList");
                printer.printKeyValue("buildConfigurations", std::vector<std::string>{
                    "A932DE901BFCD3CC0006E050 /* Debug */",
                    "A932DE911BFCD3CC0006E050 /* Release */",
                });
                printer.printKeyValue("defaultConfigurationIsVisible", "0");
            printer.endObject();
        printer.endKeyValue();
    printer.endSection();
}

std::string generatePbxproj()
{
    using StringHelper::format;

    std::stringstream stream;
    stream << "// !$*UTF8*$!\n";

    XcodePrinter printer(stream);
    printer.beginObject();
        printer.printKeyValue("archiveVersion", "1");
        printer.beginKeyValue("classes");
            printer.beginObject();
            printer.endObject();
        printer.endKeyValue();
        printer.printKeyValue("objectVersion", "46");
        printer.beginKeyValue("objects");
            printer.beginObject();
            printObjects(printer);
            printer.endObject();
        printer.endKeyValue();
        printer.printKeyValue("rootObject", "A932DE801BFCD3CC0006E050 /* Project object */");
    printer.endObject();

    stream << "\n";
    return stream.str();
}

} // unnamed namespace

GeneratorError Xcode::generateXcodeProject(const CompileOptions& options)
{
    namespace FileSystem = somera::FileSystem;

    const auto xcodeprojPath = options.outputPath + ".xcodeproj";

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

    {
        const auto pbxprojPath = FileSystem::join(xcodeprojPath, "project.pbxproj");
        std::ofstream stream(pbxprojPath);
        if (!stream) {
            return GeneratorError("Error: Cannot open.");
        }
        stream << generatePbxproj();
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
