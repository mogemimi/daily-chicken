#include "consolecolor.h"
#include "editdistance.h"
#include "spellcheck.h"
#include "typo.h"
#include "worddiff.h"
#include "wordsegmenter.h"
#include "daily/Optional.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wconversion"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Version.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Parse/Parser.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Signals.h"

#pragma GCC diagnostic pop

#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

using somera::NativeSpellChecker;
using somera::EditDistance;
using somera::Optional;
using somera::NullOpt;
using somera::WordSegmenter;
using somera::TypoMan;

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...");

namespace {

class MyCommentHandler final : public clang::CommentHandler {
private:
    llvm::StringRef inputFile;
    TypoMan* typos = nullptr;

public:
    void setFile(llvm::StringRef fileIn) { inputFile = fileIn; }

    void setTypoSet(TypoMan* typosIn) { typos = typosIn; }

    bool HandleComment(clang::Preprocessor& pp, clang::SourceRange range) override
    {
        clang::SourceManager& sm = pp.getSourceManager();
        if (sm.getFilename(range.getBegin()) != inputFile) {
            return false;
        }
        assert(typos != nullptr);
        assert(sm.getFilename(range.getBegin()) == inputFile);

        const auto startLoc = sm.getDecomposedLoc(range.getBegin());
        const auto endLoc = sm.getDecomposedLoc(range.getEnd());
        const auto fileData = sm.getBufferData(startLoc.first);

        auto sourceString = fileData.substr(startLoc.second, endLoc.second - startLoc.second).str();
        somera::TypoSource source;
        source.file = inputFile;
        typos->computeFromSentence(sourceString, source);

        return false;
    }
};

class MyASTConsumer final : public ASTConsumer {
public:
    explicit MyASTConsumer(TypoMan & typosIn)
        : typos(typosIn)
    {
    }

    void HandleTranslationUnit(ASTContext& context) override
    {
        matcher.matchAST(context);
    }

    bool HandleTopLevelDecl(clang::DeclGroupRef d) override
    {
        for (auto it = d.begin(); it != d.end(); it++) {
            auto fd = llvm::dyn_cast<clang::NamedDecl>(*it);
            if (fd) {
                std::string identifier = fd->getDeclName().getAsString();
                typos.computeFromIdentifier(identifier);
            }
        }
        return true;
    }

private:
    MatchFinder matcher;
    TypoMan & typos;
};

class MyFrontendAction final : public ASTFrontendAction {
public:
    explicit MyFrontendAction(TypoMan & typosIn) : typos(typosIn) {}

    std::unique_ptr<ASTConsumer>
    CreateASTConsumer(CompilerInstance &ci, StringRef file) override
    {
        commentHandler.setFile(file);
        commentHandler.setTypoSet(&typos);
        ci.getPreprocessor().addCommentHandler(&commentHandler);
        return std::make_unique<MyASTConsumer>(typos);
    }

private:
    TypoMan & typos;
    MyCommentHandler commentHandler;
};

class MyDiagnosticConsumer final : public clang::DiagnosticConsumer {
public:
    bool IncludeInDiagnosticCounts() const override
    {
        return false;
    }
};

class MyFrontendActionFactory final : public FrontendActionFactory {
    TypoMan & typos;
public:
    explicit MyFrontendActionFactory(TypoMan & typosIn) : typos(typosIn) {}

    clang::FrontendAction* create() override {
        return new MyFrontendAction(typos);
    }
};

} // unnamed namespace

int main(int argc, const char **argv)
{
    CommonOptionsParser options(argc, argv, MyToolCategory);
    ClangTool tool(options.getCompilations(), options.getSourcePathList());

    MyDiagnosticConsumer diagnosticConsumer;
    tool.setDiagnosticConsumer(&diagnosticConsumer);

    TypoMan typos;
    typos.setStrictWhiteSpace(false);
    typos.setStrictHyphen(false);
    typos.setStrictLetterCase(false);
    typos.setIgnoreBritishEnglish(true);
    typos.setMinimumWordSize(3);
    typos.setMaxCorrectWordCount(4);
    typos.setFoundCallback([](const somera::Typo& typo) -> void
    {
        const auto& word = typo.typo;
        const auto& corrections = typo.corrections;
        if (corrections.empty()) {
            return;
        }

        using somera::DiffOperation;
        using somera::TerminalColor;
        {
            auto & correction = corrections.front();
            auto hunks = somera::computeDiff(word, correction);
            constexpr int indentSpaces = 18;
            std::stringstream fromStream;
            for (auto & hunk : hunks) {
                if (hunk.operation == DiffOperation::Equality) {
                    fromStream << hunk.text;
                }
                else if (hunk.operation == DiffOperation::Deletion) {
                    fromStream << somera::changeTerminalTextColor(
                        hunk.text,
                        TerminalColor::Black,
                        TerminalColor::Red);
                }
            }
            for (int i = indentSpaces - static_cast<int>(word.size()); i > 0; --i) {
                fromStream << " ";
            }
            std::stringstream toStream;
            for (auto & hunk : hunks) {
                if (hunk.operation == DiffOperation::Equality) {
                    toStream << hunk.text;
                }
                else if (hunk.operation == DiffOperation::Insertion) {
                    toStream << somera::changeTerminalTextColor(
                        hunk.text,
                        TerminalColor::White,
                        TerminalColor::Green);
                }
            }
            for (int i = indentSpaces - static_cast<int>(correction.size()); i > 0; --i) {
                toStream << " ";
            }
            std::printf("%s => %s", fromStream.str().c_str(), toStream.str().c_str());
        }

        if (corrections.size() > 1) {
            std::printf(" (");
            for (size_t i = 1; i < corrections.size(); ++i) {
                if (i > 1) {
                    std::printf(" ");
                }
                auto & correction = corrections[i];
                auto hunks = somera::computeDiff(word, correction);
                std::stringstream toStream;
                for (auto & hunk : hunks) {
                    if (hunk.operation == DiffOperation::Equality) {
                        toStream << hunk.text;
                    }
                    else if (hunk.operation == DiffOperation::Insertion) {
                        toStream << somera::changeTerminalTextColor(
                            hunk.text,
                            TerminalColor::Black,
                            TerminalColor::Blue);
                    }
                }
                std::printf("%s", toStream.str().c_str());
            }
            std::printf(")");
        }
        std::printf("\n");
    });

    return tool.run(std::make_unique<MyFrontendActionFactory>(typos).get());
}
