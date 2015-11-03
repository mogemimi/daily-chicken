#include "editdistance.h"
#include "optional.h"
#include "spellcheck.h"
#include "wordsegmenter.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wconversion"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Version.h"
#include "clang/Format/Format.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Parse/Parser.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Signals.h"

#pragma GCC diagnostic pop

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>


using somera::NativeSpellChecker;
using somera::EditDistance;
using somera::Optional;
using somera::NullOpt;
using somera::WordSegmenter;

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

using clang::tooling::Replacements;

static cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...");

namespace {

#define UNUSED_CODE 0
#if UNUSED_CODE

void tryMatch(const std::string& a, const std::string& b)
{
    std::printf("%9s %9s", a.c_str(), b.c_str());
    std::printf(" | %2d", somera::EditDistance::levenshteinDistance(a, b));
    std::printf(" | %1.4f", somera::EditDistance::closestMatchFuzzyDistance(a, b));
    std::printf(" | %1.4f", somera::EditDistance::jaroWinklerDistance(a, b));
    std::cout << std::endl;
}

void checkSpell(const std::string& word)
{
    auto correction = somera::NativeSpellChecker::correctSpelling(word);
    auto result = somera::NativeSpellChecker::findClosestWords(word);

    std::cout << word << " => ";

    if (correction.empty()) {
        std::cout << "(is not misspelling)";
    }
    else {
        std::cout << correction;
    }

    {
        std::cout << " (";
        bool spacing = false;
        for (auto & s : result) {
            if (spacing) {
                std::cout << " ";
            }
            std::cout << s;
            spacing = true;
        }
        std::cout << ")" << std::endl;
    }
}

void test()
{
    std::printf("%9s %9s | Ls | fast-M | Jw \n", "s1", "s2");

    tryMatch("LISP", "LISP");
    tryMatch("LISP", "LISp");
    tryMatch("LISP", "LIsp");
    tryMatch("LISP", "Lisp");
    tryMatch("LISP", "lisp");
    tryMatch("LISP", "lis");
    tryMatch("LISP", "li");
    tryMatch("LISP", "l");
    tryMatch("LISP", "");
    tryMatch("abcd", "abcd");
    tryMatch("abcd", "abcD");
    tryMatch("abcd", "Abcd");
    tryMatch("abc", "abc");
    tryMatch("abc", "Abc");
    tryMatch("abc", "aBc");
    tryMatch("abc", "abC");
    tryMatch("abc", "ABc");
    tryMatch("abc", "aBC");
    tryMatch("abc", "AbC");
    tryMatch("abc", "acb");
    tryMatch("abc", "bac");
    tryMatch("abc", "bca");
    tryMatch("abc", "acB");
    tryMatch("A", "A");
    tryMatch("A", "a");
    tryMatch("A", "B");
    tryMatch("Google", "google");
    tryMatch("Typo", "Type");
    tryMatch("Hoge", "Foo");
    tryMatch("unkown", "unknown");

    std::cout << "------" << std::endl;

    checkSpell("unknown");
    checkSpell("unkown");
    checkSpell("exisiting");
    checkSpell("volid");
    tryMatch("volid", "solid");
    tryMatch("volid", "valid");
    tryMatch("volid", "void");
    tryMatch("volid", "voile");
    tryMatch("volid", "vilid");
    tryMatch("volid", "vllid");
    checkSpell("charater");
    tryMatch("charater", "charter");
    tryMatch("charater", "character");
}





void parse(int argc, char *argv[])
{
    if (argc < 2) {
        std::printf("failed");
        return;
    }

    std::string path = argv[1];

    if (path.empty()) {
        std::printf("failed");
        return;
    }

    std::fstream stream(path);

    if (!stream) {
        std::printf("cannot open file %s", path.c_str());
        return;
    }

    std::cout << "open: " << path << std::endl;

    std::set<std::string> dictionary = {"json", "impl"};

    std::string line;
    while(std::getline(stream, line)) {
        splitWords(line, [&dictionary](const std::string& word){
            auto iter = dictionary.find(word);
            if (iter != std::end(dictionary)) {
                return;
            }
            auto corrections = correctWord(word);
            if (!corrections.empty()) {
                std::printf("%10s => %10s", word.c_str(), corrections.front().c_str());
                if (corrections.size() > 1) {
                    std::printf(" (");
                    for (size_t i = 1; i < corrections.size(); ++i) {
                        if (i > 1) {
                            std::printf(" ");
                        }
                        std::printf("%s", corrections[i].c_str());
                    }
                    std::printf(")");
                }
                std::printf("\n");
            }
        });
    }
}

#endif

std::vector<std::string> correctWord(const std::string& word)
{
    auto correction = somera::NativeSpellChecker::correctSpelling(word);
    if (correction.empty()) {
        return {};
    }

    auto corrections = somera::NativeSpellChecker::findClosestWords(word);
    if (corrections.empty()) {
        return {std::move(correction)};
    }

    if (corrections.size() > 4) {
        corrections.resize(4);
    }

    std::sort(std::begin(corrections), std::end(corrections), [&](auto& a, auto& b) {
        const auto fuzzyA = EditDistance::closestMatchFuzzyDistance(word, a);
        const auto fuzzyB = EditDistance::closestMatchFuzzyDistance(word, b);
        if (fuzzyA != fuzzyB) {
            return fuzzyA > fuzzyB;
        }
        const auto jwA = EditDistance::jaroWinklerDistance(word, a);
        const auto jwB = EditDistance::jaroWinklerDistance(word, b);
        if (jwA != jwB) {
            return jwA > jwB;
        }
        const auto lsA = EditDistance::levenshteinDistance(word, a);
        const auto lsB = EditDistance::levenshteinDistance(word, b);
        return lsA <= lsB;
    });

    assert(!corrections.empty());
    return std::move(corrections);
}

class MyCommentHandler final : public clang::CommentHandler {
private:
    llvm::StringRef inputFile;
    Rewriter* rewriter = nullptr;

public:
    void setFile(llvm::StringRef fileIn) { inputFile = fileIn; }

    void setRewriter(Rewriter* rewriterIn) { rewriter = rewriterIn; }

    bool HandleComment(clang::Preprocessor &pp, clang::SourceRange range) override
    {
        clang::SourceManager &sm = pp.getSourceManager();
        if (sm.getFilename(range.getBegin()) != inputFile) {
            return false;
        }

        assert(rewriter != nullptr);
        assert(sm.getFilename(range.getBegin()) == inputFile);

        const auto startLoc = sm.getDecomposedLoc(range.getBegin());
        const auto endLoc = sm.getDecomposedLoc(range.getEnd());
        const auto fileData = sm.getBufferData(startLoc.first);

        auto sourceString = fileData.substr(startLoc.second, endLoc.second - startLoc.second).str();

        std::set<std::string> dictionary = {
            "json",
            "impl",
            "NONINFRINGEMENT", // for MIT License
            "overrided" // => overriden
        };

        WordSegmenter segmenter;
        segmenter.setDictionary(dictionary);

        segmenter.parse(sourceString, [](const somera::PartOfSpeech& pos) {
            const auto word = pos.text;
            if (pos.tag != somera::PartOfSpeechTag::Raw and
                pos.tag != somera::PartOfSpeechTag::EnglishWord) {
                return;
            }

            auto corrections = correctWord(word);
            if (!corrections.empty()) {
                std::printf("%20s => %20s", word.c_str(), corrections.front().c_str());
                if (corrections.size() > 1) {
                    std::printf(" (");
                    for (size_t i = 1; i < corrections.size(); ++i) {
                        if (i > 1) {
                            std::printf(" ");
                        }
                        std::printf("%s", corrections[i].c_str());
                    }
                    std::printf(")");
                }
                std::printf("\n");
            }
        });

        std::transform(sourceString.begin(), sourceString.end(), sourceString.begin(), toupper);

        rewriter->ReplaceText(range, sourceString);

        return false;
    }
};

class IfStmtHandler final : public MatchFinder::MatchCallback {
public:
    IfStmtHandler(Rewriter &rewriterIn)
        : rewriter(rewriterIn)
    {
    }

    void run(const MatchFinder::MatchResult &result) override
    {
        if (const IfStmt *ifStatement =
                result.Nodes.getNodeAs<clang::IfStmt>("ifStmt")) {
            const Stmt *Then = ifStatement->getThen();
            rewriter.InsertText(Then->getLocStart(), "// the 'if' part\n", true,
                               true);

            if (const Stmt *Else = ifStatement->getElse()) {
                rewriter.InsertText(Else->getLocStart(), "// the 'else' part\n",
                                   true, true);
            }
        }
    }

private:
    Rewriter &rewriter;
};

class DeclStmtHandler final : public MatchFinder::MatchCallback {
public:
    DeclStmtHandler(Rewriter &rewriterIn)
        : rewriter(rewriterIn)
    {
    }

    void run(const MatchFinder::MatchResult &result) override
    {
        if (const DeclStmt *declStatement =
                result.Nodes.getNodeAs<clang::DeclStmt>("declStmt")) {

            rewriter.InsertText(declStatement->getLocStart(),
                               "// the 'decl start' part\n", true, true);
        }
    }

private:
    Rewriter &rewriter;
};

class MyASTConsumer final : public ASTConsumer {
public:
    MyASTConsumer(Rewriter &rewriter)
        : handlerForIf(rewriter)
        , handlerForDeclStmt(rewriter)
    {
        matcher.addMatcher(ifStmt().bind("ifStmt"), &handlerForIf);
        matcher.addMatcher(declStmt().bind("declStmt"), &handlerForDeclStmt);
    }

    void HandleTranslationUnit(ASTContext &context) override
    {
        matcher.matchAST(context);
    }

private:
    IfStmtHandler handlerForIf;
    DeclStmtHandler handlerForDeclStmt;
    MatchFinder matcher;
};

class MyFrontendAction final : public ASTFrontendAction {
public:
    MyFrontendAction() {}

    void EndSourceFileAction() override
    {
//        rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID())
//            .write(llvm::outs());
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci,
                                                   StringRef file) override
    {
        commentHandler.setFile(file);
        commentHandler.setRewriter(&rewriter);

        ci.getPreprocessor().addCommentHandler(&commentHandler);

        rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
        return llvm::make_unique<MyASTConsumer>(rewriter);
    }

private:
    Rewriter rewriter;
    MyCommentHandler commentHandler;
};

class MyDiagnosticConsumer final : public clang::DiagnosticConsumer {
public:
    bool IncludeInDiagnosticCounts() const override
    {
        return false;
    }
};

} // unnamed namespace

int main(int argc, const char **argv)
{
    CommonOptionsParser options(argc, argv, MyToolCategory);
    ClangTool tool(options.getCompilations(), options.getSourcePathList());

    MyDiagnosticConsumer diagnosticConsumer;
    tool.setDiagnosticConsumer(&diagnosticConsumer);

    return tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
