#include "consolecolor.h"
#include "editdistance.h"
#include "spellcheck.h"
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

struct TypoSource {
    std::string file;
    clang::SourceRange range;
};

struct Typo {
    std::string typo;
    std::vector<std::string> corrections;
    std::vector<TypoSource> sources;
};

struct TypoSet {
private:
    std::vector<std::string> newTypoWords;
public:
    std::map<std::string, Typo> typos;

    bool exists(const std::string& word)
    {
        return typos.find(word) != std::end(typos);
    }

    somera::Optional<Typo*> findTypo(const std::string& word)
    {
        auto iter = typos.find(word);
        if (iter != std::end(typos)) {
            return &iter->second;
        }
        return somera::NullOpt;
    }

    void addTypoSource(const std::string& word, TypoSource && s)
    {
        auto iter = typos.find(word);
        assert(iter != std::end(typos));
        iter->second.sources.push_back(std::move(s));
    }

    void addTypo(Typo && typo)
    {
        auto word = typo.typo;
        newTypoWords.push_back(word);
        typos.emplace(word, std::move(typo));
    }

    void clearNewTypoWords()
    {
        newTypoWords.clear();
    }

    const std::vector<std::string>& getNewTypoWords() const
    {
        return newTypoWords;
    }
};

class MyCommentHandler final : public clang::CommentHandler {
private:
    llvm::StringRef inputFile;
    Rewriter* rewriter = nullptr;
    TypoSet* typos = nullptr;

public:
    void setFile(llvm::StringRef fileIn) { inputFile = fileIn; }

    void setRewriter(Rewriter* rewriterIn) { rewriter = rewriterIn; }

    void setTypoSet(TypoSet* typosIn) { typos = typosIn; }

    bool HandleComment(clang::Preprocessor &pp, clang::SourceRange range) override
    {
        clang::SourceManager &sm = pp.getSourceManager();
        if (sm.getFilename(range.getBegin()) != inputFile) {
            return false;
        }

        assert(typos != nullptr);
        assert(rewriter != nullptr);
        assert(sm.getFilename(range.getBegin()) == inputFile);

        const auto startLoc = sm.getDecomposedLoc(range.getBegin());
        const auto endLoc = sm.getDecomposedLoc(range.getEnd());
        const auto fileData = sm.getBufferData(startLoc.first);

        auto sourceString = fileData.substr(startLoc.second, endLoc.second - startLoc.second).str();

        std::set<std::string> dictionary = {
//            ".jpg",
//            ".png",
//            ".mp3",
//            ".ogg",
//            "//!", // doxygen
            "json",
            "impl",
            "NONINFRINGEMENT", // for MIT License
            "overrided" // => overriden
        };

        WordSegmenter segmenter;
        segmenter.setDictionary(dictionary);

        segmenter.parse(sourceString, [&](const somera::PartOfSpeech& pos) {
            const auto word = pos.text;
            if (pos.tag != somera::PartOfSpeechTag::Raw and
                pos.tag != somera::PartOfSpeechTag::EnglishWord) {
                return;
            }

            if (typos->exists(word)) {
                TypoSource source;
                source.file = inputFile;
                source.range = range;
                typos->addTypoSource(word, std::move(source));
                return;
            }

            auto corrections = correctWord(word);
            if (!corrections.empty()) {
                Typo typo;
                typo.typo = word;
                typo.corrections = std::move(corrections);
                typos->addTypo(std::move(typo));
            }
        });

//        std::transform(sourceString.begin(), sourceString.end(), sourceString.begin(), toupper);
//        rewriter->ReplaceText(range, sourceString);

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
//        if (const IfStmt *ifStatement =
//                result.Nodes.getNodeAs<clang::IfStmt>("ifStmt")) {
//            const Stmt *Then = ifStatement->getThen();
//            rewriter.InsertText(Then->getLocStart(), "// if\n", true, true);
//
//            if (const Stmt *Else = ifStatement->getElse()) {
//                rewriter.InsertText(Else->getLocStart(), "// else\n", true, true);
//            }
//        }
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
//        if (const DeclStmt *declStatement =
//            result.Nodes.getNodeAs<clang::DeclStmt>("declStmt")) {
//
//            const auto range = declStatement->getSourceRange();
//            const auto& sm = *result.SourceManager;
//            const auto startLoc = sm.getDecomposedLoc(range.getBegin());
//            const auto endLoc = sm.getDecomposedLoc(range.getEnd());
//            const auto fileData = sm.getBufferData(startLoc.first);
//
//            auto sourceString = fileData.substr(
//                startLoc.second, endLoc.second - startLoc.second).str();
//
//            std::cout << "ok: " <<  sourceString  << std::endl;
//
//            rewriter.InsertText(declStatement->getLocStart(), "// decl\n", true, true);
//        }
    }

private:
    Rewriter &rewriter;
};

class MyASTConsumer final : public ASTConsumer {
public:
    MyASTConsumer(Rewriter &rewriter, TypoSet & typosIn)
        : handlerForIf(rewriter)
        , handlerForDeclStmt(rewriter)
        , typos(typosIn)
    {
        matcher.addMatcher(ifStmt().bind("ifStmt"), &handlerForIf);
        matcher.addMatcher(declStmt().bind("declStmt"), &handlerForDeclStmt);
        matcher.addMatcher(declStmt().bind("declStmt"), &handlerForDeclStmt);
    }

    void HandleTranslationUnit(ASTContext &context) override
    {
        matcher.matchAST(context);
    }

    bool HandleTopLevelDecl(clang::DeclGroupRef d) override
    {
        for (auto it = d.begin(); it != d.end(); it++)
        {
//            {
//                clang::VarDecl *vd = llvm::dyn_cast<clang::VarDecl>(*it);
//                if (vd && vd->isFileVarDecl() && !vd->hasExternalStorage()) {
//                    std::cerr << "Read top-level variable decl: '";
//                    std::cerr << vd->getDeclName().getAsString();
//                    std::cerr << std::endl;
//                }
//            }
//            {
//                auto fd = llvm::dyn_cast<clang::NamedDecl>(*it);
//                if (fd) {
//                    std::cerr << "Read top-level variable decl: '";
//                    std::cerr << fd->getDeclName().getAsString();
//                    std::cerr << std::endl;
//                }
//            }
//            {
//                auto fd = llvm::dyn_cast<clang::NamespaceDecl>(*it);
//                if (fd) {
//                    std::cerr << "Read top-level variable decl: '";
//                    std::cerr << fd->getDeclName().getAsString();
//                    std::cerr << std::endl;
//                }
//            }
//            {
//                clang::VarDecl *vd = llvm::dyn_cast<clang::VarDecl>(*it);
//                if (vd) {
//                    std::cerr << "Read top-level variable decl: '";
//                    std::cerr << vd->getDeclName().getAsString();
//                    std::cerr << std::endl;
//                }
//            }
//            {
//                auto fd = llvm::dyn_cast<clang::FunctionDecl>(*it);
//                if (fd) {
//                    std::cerr << "Read top-level variable decl: '";
//                    std::cerr << fd->getDeclName().getAsString();
//                    std::cerr << std::endl;
//                }
//            }

            auto fd = llvm::dyn_cast<clang::NamedDecl>(*it);
            if (fd) {
                std::string identifier = fd->getDeclName().getAsString();
                auto words = somera::IdentifierWordSegmenter::parse(identifier);

                for (auto & word : words) {
                    if (word.size() <= 3) {
                        continue;
                    }
                    if (typos.exists(word)) {
                        continue;
                    }

                    auto corrections = correctWord(word);
                    if (!corrections.empty()) {
                        Typo typo;
                        typo.typo = word;
                        typo.corrections = std::move(corrections);
                        typos.addTypo(std::move(typo));
                    }
                }
            }
        }
        return true;
    }

private:
    IfStmtHandler handlerForIf;
    DeclStmtHandler handlerForDeclStmt;
    MatchFinder matcher;
    TypoSet & typos;
};

class MyPPCallbacks : public clang::PPCallbacks {
public:
//    {
//        std::cout << "======" << std::endl;
////        auto & ci = getCompilerInstance();
//        clang::Token token;
//        do {
//            assert(ci.hasPreprocessor());
//            auto & preprocessor = ci.getPreprocessor();
//            auto & diagnosticsEngine = ci.getDiagnostics();
////            auto & preprocessor = pp;
////            auto & diagnosticsEngine = pp.getDiagnostics();
//            if (diagnosticsEngine.hasErrorOccurred()) {
//                break;
//            }
//            if (!preprocessor.isInPrimaryFile()) {
//                break;
//            }
//            //preprocessor.LexNonComment(token);
//            preprocessor.Lex(token);
//            if (token.getKind() == tok::comment) {
//                // is comment
//            }
//            //tok::unknown
//            //identifier
//            //raw_identifier
//            //string_literal
//            //wide_string_literal
//            //angle_string_literal
//            //utf8_string_literal
//            //utf16_string_literal
//            //utf32_string_literal
//            preprocessor.DumpToken(token);
//            std::cerr << std::endl;
//        } while (token.isNot(clang::tok::eof));
//    }
};

class MyFrontendAction final : public ASTFrontendAction {
public:
    MyFrontendAction(TypoSet & typosIn) : typos(typosIn) {}

    void EndSourceFileAction() override
    {
        for (const auto& word : typos.getNewTypoWords()) {
            assert(typos.exists(word));
            const auto& typo = **typos.findTypo(word);
            const auto& corrections = typo.corrections;

            if (corrections.empty()) {
                continue;
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
        }
        typos.clearNewTypoWords();

//        rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID())
//            .write(llvm::outs());
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci,
                                                   StringRef file) override
    {
        commentHandler.setFile(file);
        commentHandler.setRewriter(&rewriter);
        commentHandler.setTypoSet(&typos);

        ci.getPreprocessor().addCommentHandler(&commentHandler);
        ci.getPreprocessor().addPPCallbacks(std::make_unique<MyPPCallbacks>());

        rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
        return std::make_unique<MyASTConsumer>(rewriter, typos);
    }

private:
    Rewriter rewriter;
    TypoSet & typos;
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
    TypoSet & typos;
public:
    MyFrontendActionFactory(TypoSet & typosIn) : typos(typosIn) {}

    clang::FrontendAction* create() override {
        return new MyFrontendAction(typos);
    }
};

} // unnamed namespace

int main(int argc, const char **argv)
{
    CommonOptionsParser options(argc, argv, MyToolCategory);
    ClangTool tool(options.getCompilations(), options.getSourcePathList());

    TypoSet typos;
    MyDiagnosticConsumer diagnosticConsumer;
    tool.setDiagnosticConsumer(&diagnosticConsumer);

    return tool.run(std::make_unique<MyFrontendActionFactory>(typos).get());
}
