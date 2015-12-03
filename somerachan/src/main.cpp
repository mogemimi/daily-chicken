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
using somera::TypoMan;

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

class MyCommentHandler final : public clang::CommentHandler {
private:
    llvm::StringRef inputFile;
    Rewriter* rewriter = nullptr;
    TypoMan* typos = nullptr;

public:
    void setFile(llvm::StringRef fileIn) { inputFile = fileIn; }

    void setRewriter(Rewriter* rewriterIn) { rewriter = rewriterIn; }

    void setTypoSet(TypoMan* typosIn) { typos = typosIn; }

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
        somera::TypoSource source;
        source.file = inputFile;
        typos->computeFromSentence(sourceString, source);

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
    MyASTConsumer(Rewriter &rewriter, TypoMan & typosIn)
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
                typos.computeFromIdentifier(identifier);
            }
        }
        return true;
    }

private:
    IfStmtHandler handlerForIf;
    DeclStmtHandler handlerForDeclStmt;
    MatchFinder matcher;
    TypoMan & typos;
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
    MyFrontendAction(TypoMan & typosIn) : typos(typosIn) {}

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
        commentHandler.setTypoSet(&typos);

        ci.getPreprocessor().addCommentHandler(&commentHandler);
        ci.getPreprocessor().addPPCallbacks(std::make_unique<MyPPCallbacks>());

        rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
        return std::make_unique<MyASTConsumer>(rewriter, typos);
    }

private:
    Rewriter rewriter;
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
    MyFrontendActionFactory(TypoMan & typosIn) : typos(typosIn) {}

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
    typos.setMinimumWordSize(4);
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
