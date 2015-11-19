// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "ProjectTemplate.h"
#include "FileSystem.h"
#include "StringHelper.h"
#include <fstream>
#include <utility>
#include <random>
#include <ctime>
#include <sstream>

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

std::string generatePbxproj()
{
    std::stringstream stream;
    stream <<
    "// !$*UTF8*$!\n"
    "{\n"
    "  archiveVersion = 1;\n"
    "  classes = {\n"
    "  };\n"
    "  objectVersion = 46;\n"
    "  objects = {\n"
    "\n";
    stream <<
    "/* Begin PBXBuildFile section */\n"
    "    A932DE8C1BFCD3CC0006E050 /* main.cpp in Sources */ = {isa = PBXBuildFile; fileRef = A932DE8B1BFCD3CC0006E050 /* main.cpp */; };\n"
    "/* End PBXBuildFile section */\n"
    "\n";
    stream <<
    "/* Begin PBXCopyFilesBuildPhase section */\n"
    "    A932DE861BFCD3CC0006E050 /* CopyFiles */ = {\n"
    "      isa = PBXCopyFilesBuildPhase;\n"
    "      buildActionMask = 2147483647;\n"
    "      dstPath = /usr/share/man/man1/;\n"
    "      dstSubfolderSpec = 0;\n"
    "      files = (\n"
    "      );\n"
    "      runOnlyForDeploymentPostprocessing = 1;\n"
    "    };\n"
    "/* End PBXCopyFilesBuildPhase section */\n"
    "\n";
    stream <<
    "/* Begin PBXFileReference section */\n"
    "    A932DE881BFCD3CC0006E050 /* MyHidamari */ = {isa = PBXFileReference; explicitFileType = \"compiled.mach-o.executable\"; includeInIndex = 0; path = MyHidamari; sourceTree = BUILT_PRODUCTS_DIR; };\n"
    "    A932DE8B1BFCD3CC0006E050 /* main.cpp */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.cpp.cpp; path = main.cpp; sourceTree = \"<group>\"; };\n"
    "/* End PBXFileReference section */\n"
    "\n";
    stream <<
    "/* Begin PBXFrameworksBuildPhase section */\n"
    "    A932DE851BFCD3CC0006E050 /* Frameworks */ = {\n"
    "      isa = PBXFrameworksBuildPhase;\n"
    "      buildActionMask = 2147483647;\n"
    "      files = (\n"
    "      );\n"
    "      runOnlyForDeploymentPostprocessing = 0;\n"
    "    };\n"
    "/* End PBXFrameworksBuildPhase section */\n"
    "\n";
    stream <<
    "/* Begin PBXGroup section */\n"
    "    A932DE7F1BFCD3CC0006E050 = {\n"
    "      isa = PBXGroup;\n"
    "      children = (\n"
    "        A932DE8A1BFCD3CC0006E050 /* MyHidamari */,\n"
    "        A932DE891BFCD3CC0006E050 /* Products */,\n"
    "      );\n"
    "      sourceTree = \"<group>\";\n"
    "    };\n"
    "    A932DE891BFCD3CC0006E050 /* Products */ = {\n"
    "      isa = PBXGroup;\n"
    "      children = (\n"
    "        A932DE881BFCD3CC0006E050 /* MyHidamari */,\n"
    "      );\n"
    "      name = Products;\n"
    "      sourceTree = \"<group>\";\n"
    "    };\n"
    "    A932DE8A1BFCD3CC0006E050 /* MyHidamari */ = {\n"
    "      isa = PBXGroup;\n"
    "      children = (\n"
    "        A932DE8B1BFCD3CC0006E050 /* main.cpp */,\n"
    "      );\n"
    "      path = MyHidamari;\n"
    "      sourceTree = \"<group>\";\n"
    "    };\n"
    "/* End PBXGroup section */\n"
    "\n";
    stream <<
    "/* Begin PBXNativeTarget section */\n"
    "    A932DE871BFCD3CC0006E050 /* MyHidamari */ = {\n"
    "      isa = PBXNativeTarget;\n"
    "      buildConfigurationList = A932DE8F1BFCD3CC0006E050 /* Build configuration list for PBXNativeTarget \"MyHidamari\" */;\n"
    "      buildPhases = (\n"
    "        A932DE841BFCD3CC0006E050 /* Sources */,\n"
    "        A932DE851BFCD3CC0006E050 /* Frameworks */,\n"
    "        A932DE861BFCD3CC0006E050 /* CopyFiles */,\n"
    "      );\n"
    "      buildRules = (\n"
    "      );\n"
    "      dependencies = (\n"
    "      );\n"
    "      name = MyHidamari;\n"
    "      productName = MyHidamari;\n"
    "      productReference = A932DE881BFCD3CC0006E050 /* MyHidamari */;\n"
    "      productType = \"com.apple.product-type.tool\";\n"
    "    };\n"
    "/* End PBXNativeTarget section */\n"
    "\n";
    stream <<
    "/* Begin PBXProject section */\n"
    "    A932DE801BFCD3CC0006E050 /* Project object */ = {\n"
    "      isa = PBXProject;\n"
    "      attributes = {\n"
    "        LastUpgradeCheck = 0710;\n"
    "        ORGANIZATIONNAME = mogemimi;\n"
    "        TargetAttributes = {\n"
    "          A932DE871BFCD3CC0006E050 = {\n"
    "            CreatedOnToolsVersion = 7.1.1;\n"
    "          };\n"
    "        };\n"
    "      };\n"
    "      buildConfigurationList = A932DE831BFCD3CC0006E050 /* Build configuration list for PBXProject \"MyHidamari\" */;\n"
    "      compatibilityVersion = \"Xcode 3.2\";\n"
    "      developmentRegion = English;\n"
    "      hasScannedForEncodings = 0;\n"
    "      knownRegions = (\n"
    "        en,\n"
    "      );\n"
    "      mainGroup = A932DE7F1BFCD3CC0006E050;\n"
    "      productRefGroup = A932DE891BFCD3CC0006E050 /* Products */;\n"
    "      projectDirPath = \"\";\n"
    "      projectRoot = \"\";\n"
    "      targets = (\n"
    "        A932DE871BFCD3CC0006E050 /* MyHidamari */,\n"
    "      );\n"
    "    };\n"
    "/* End PBXProject section */\n"
    "\n";
    stream <<
    "/* Begin PBXSourcesBuildPhase section */\n"
    "    A932DE841BFCD3CC0006E050 /* Sources */ = {\n"
    "      isa = PBXSourcesBuildPhase;\n"
    "      buildActionMask = 2147483647;\n"
    "      files = (\n"
    "        A932DE8C1BFCD3CC0006E050 /* main.cpp in Sources */,\n"
    "      );\n"
    "      runOnlyForDeploymentPostprocessing = 0;\n"
    "    };\n"
    "/* End PBXSourcesBuildPhase section */\n"
    "\n";
    stream <<
    "/* Begin XCBuildConfiguration section */\n"
    "    A932DE8D1BFCD3CC0006E050 /* Debug */ = {\n"
    "      isa = XCBuildConfiguration;\n"
    "      buildSettings = {\n"
    "        ALWAYS_SEARCH_USER_PATHS = NO;\n"
    "        CLANG_CXX_LANGUAGE_STANDARD = \"gnu++0x\";\n"
    "        CLANG_CXX_LIBRARY = \"libc++\";\n"
    "        CLANG_ENABLE_MODULES = YES;\n"
    "        CLANG_ENABLE_OBJC_ARC = YES;\n"
    "        CLANG_WARN_BOOL_CONVERSION = YES;\n"
    "        CLANG_WARN_CONSTANT_CONVERSION = YES;\n"
    "        CLANG_WARN_DIRECT_OBJC_ISA_USAGE = YES_ERROR;\n"
    "        CLANG_WARN_EMPTY_BODY = YES;\n"
    "        CLANG_WARN_ENUM_CONVERSION = YES;\n"
    "        CLANG_WARN_INT_CONVERSION = YES;\n"
    "        CLANG_WARN_OBJC_ROOT_CLASS = YES_ERROR;\n"
    "        CLANG_WARN_UNREACHABLE_CODE = YES;\n"
    "        CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;\n"
    "        CODE_SIGN_IDENTITY = \"-\";\n"
    "        COPY_PHASE_STRIP = NO;\n"
    "        DEBUG_INFORMATION_FORMAT = dwarf;\n"
    "        ENABLE_STRICT_OBJC_MSGSEND = YES;\n"
    "        ENABLE_TESTABILITY = YES;\n"
    "        GCC_C_LANGUAGE_STANDARD = gnu99;\n"
    "        GCC_DYNAMIC_NO_PIC = NO;\n"
    "        GCC_NO_COMMON_BLOCKS = YES;\n"
    "        GCC_OPTIMIZATION_LEVEL = 0;\n"
    "        GCC_PREPROCESSOR_DEFINITIONS = (\n"
    "          \"DEBUG=1\",\n"
    "          \"$(inherited)\",\n"
    "        );\n"
    "        GCC_WARN_64_TO_32_BIT_CONVERSION = YES;\n"
    "        GCC_WARN_ABOUT_RETURN_TYPE = YES_ERROR;\n"
    "        GCC_WARN_UNDECLARED_SELECTOR = YES;\n"
    "        GCC_WARN_UNINITIALIZED_AUTOS = YES_AGGRESSIVE;\n"
    "        GCC_WARN_UNUSED_FUNCTION = YES;\n"
    "        GCC_WARN_UNUSED_VARIABLE = YES;\n"
    "        MACOSX_DEPLOYMENT_TARGET = 10.11;\n"
    "        MTL_ENABLE_DEBUG_INFO = YES;\n"
    "        ONLY_ACTIVE_ARCH = YES;\n"
    "        SDKROOT = macosx;\n"
    "      };\n"
    "      name = Debug;\n"
    "    };\n"
    "    A932DE8E1BFCD3CC0006E050 /* Release */ = {\n"
    "      isa = XCBuildConfiguration;\n"
    "      buildSettings = {\n"
    "        ALWAYS_SEARCH_USER_PATHS = NO;\n"
    "        CLANG_CXX_LANGUAGE_STANDARD = \"gnu++0x\";\n"
    "        CLANG_CXX_LIBRARY = \"libc++\";\n"
    "        CLANG_ENABLE_MODULES = YES;\n"
    "        CLANG_ENABLE_OBJC_ARC = YES;\n"
    "        CLANG_WARN_BOOL_CONVERSION = YES;\n"
    "        CLANG_WARN_CONSTANT_CONVERSION = YES;\n"
    "        CLANG_WARN_DIRECT_OBJC_ISA_USAGE = YES_ERROR;\n"
    "        CLANG_WARN_EMPTY_BODY = YES;\n"
    "        CLANG_WARN_ENUM_CONVERSION = YES;\n"
    "        CLANG_WARN_INT_CONVERSION = YES;\n"
    "        CLANG_WARN_OBJC_ROOT_CLASS = YES_ERROR;\n"
    "        CLANG_WARN_UNREACHABLE_CODE = YES;\n"
    "        CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;\n"
    "        CODE_SIGN_IDENTITY = \"-\";\n"
    "        COPY_PHASE_STRIP = NO;\n"
    "        DEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\";\n"
    "        ENABLE_NS_ASSERTIONS = NO;\n"
    "        ENABLE_STRICT_OBJC_MSGSEND = YES;\n"
    "        GCC_C_LANGUAGE_STANDARD = gnu99;\n"
    "        GCC_NO_COMMON_BLOCKS = YES;\n"
    "        GCC_WARN_64_TO_32_BIT_CONVERSION = YES;\n"
    "        GCC_WARN_ABOUT_RETURN_TYPE = YES_ERROR;\n"
    "        GCC_WARN_UNDECLARED_SELECTOR = YES;\n"
    "        GCC_WARN_UNINITIALIZED_AUTOS = YES_AGGRESSIVE;\n"
    "        GCC_WARN_UNUSED_FUNCTION = YES;\n"
    "        GCC_WARN_UNUSED_VARIABLE = YES;\n"
    "        MACOSX_DEPLOYMENT_TARGET = 10.11;\n"
    "        MTL_ENABLE_DEBUG_INFO = NO;\n"
    "        SDKROOT = macosx;\n"
    "      };\n"
    "      name = Release;\n"
    "    };\n"
    "    A932DE901BFCD3CC0006E050 /* Debug */ = {\n"
    "      isa = XCBuildConfiguration;\n"
    "      buildSettings = {\n"
    "        PRODUCT_NAME = \"$(TARGET_NAME)\";\n"
    "      };\n"
    "      name = Debug;\n"
    "    };\n"
    "    A932DE911BFCD3CC0006E050 /* Release */ = {\n"
    "      isa = XCBuildConfiguration;\n"
    "      buildSettings = {\n"
    "        PRODUCT_NAME = \"$(TARGET_NAME)\";\n"
    "      };\n"
    "      name = Release;\n"
    "    };\n"
    "/* End XCBuildConfiguration section */\n"
    "\n";
    stream <<
    "/* Begin XCConfigurationList section */\n"
    "    A932DE831BFCD3CC0006E050 /* Build configuration list for PBXProject \"MyHidamari\" */ = {\n"
    "      isa = XCConfigurationList;\n"
    "      buildConfigurations = (\n"
    "        A932DE8D1BFCD3CC0006E050 /* Debug */,\n"
    "        A932DE8E1BFCD3CC0006E050 /* Release */,\n"
    "      );\n"
    "      defaultConfigurationIsVisible = 0;\n"
    "      defaultConfigurationName = Release;\n"
    "    };\n"
    "    A932DE8F1BFCD3CC0006E050 /* Build configuration list for PBXNativeTarget \"MyHidamari\" */ = {\n"
    "      isa = XCConfigurationList;\n"
    "      buildConfigurations = (\n"
    "        A932DE901BFCD3CC0006E050 /* Debug */,\n"
    "        A932DE911BFCD3CC0006E050 /* Release */,\n"
    "      );\n"
    "      defaultConfigurationIsVisible = 0;\n"
    "    };\n"
    "/* End XCConfigurationList section */\n";
    stream <<
    "  };\n"
    "  rootObject = A932DE801BFCD3CC0006E050 /* Project object */;\n"
    "}\n";
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
