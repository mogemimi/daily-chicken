// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include "../daily/CommandLineParser.h"
#include "../daily/StringHelper.h"
#include <iostream>
#include <fstream>
#include <regex>

using namespace somera;

namespace {

class HandlerWrapperBase {
public:
    virtual ~HandlerWrapperBase() = default;
};

template <class In>
class InboundHandlerContext {
public:
};

template <class In, class Out>
class BothHandlerContext {
public:
};

template <class Out>
class OutboundHandlerContext {
public:
};

template <class In>
class InboundLink {
public:
    virtual ~InboundLink() = default;
};

template <class Out>
class OutboundLink {
public:
    virtual ~OutboundLink() = default;
};

class PipelineContext {
public:
    virtual ~PipelineContext() = default;

    virtual void SetNextIn(PipelineContext* context) = 0;

    virtual void SetNextOut(PipelineContext* context) = 0;
};

template <class In, class Out>
class ContextImplement final
    : public InboundLink<In>
    , public OutboundLink<Out>
    , public PipelineContext {
public:
    void SetNextIn(PipelineContext* context) override
    {
        if (context == nullptr) {
            nextIn = nullptr;
            return;
        }
        auto newNextIn = dynamic_cast<InboundLink<In>*>(context);
        if (newNextIn) {
            nextIn = newNextIn;
        }
        else {
            throw std::invalid_argument("error");
        }
    }

    void SetNextOut(PipelineContext* context) override
    {
        if (context == nullptr) {
            nextOut = nullptr;
            return;
        }
        auto newNextOut = dynamic_cast<OutboundLink<Out>*>(context);
        if (newNextOut) {
            nextOut = newNextOut;
        }
        else {
            throw std::invalid_argument("error");
        }
    }

private:
    InboundLink<In>* nextIn = nullptr;
    OutboundLink<Out>* nextOut = nullptr;
};

template <class Out>
class InboundContextImplement final
    : public InboundLink<Out>
    , public PipelineContext {
public:
    void SetNextIn(PipelineContext* context) override
    {
    }

    void SetNextOut(PipelineContext* context) override
    {
        if (context == nullptr) {
            nextOut = nullptr;
            return;
        }
        auto newNextOut = dynamic_cast<OutboundLink<Out>*>(context);
        if (newNextOut) {
            nextOut = newNextOut;
        }
        else {
            throw std::invalid_argument("error");
        }
    }

private:
    OutboundLink<Out>* nextOut;
};

template <class In>
class OutboundContextImplement final
    : public OutboundLink<In>
    , public PipelineContext {
public:
    void SetNextIn(PipelineContext* context) override
    {
        if (context == nullptr) {
            nextIn = nullptr;
            return;
        }
        auto newNextIn = dynamic_cast<InboundLink<In>*>(context);
        if (newNextIn) {
            nextIn = newNextIn;
        }
        else {
            throw std::invalid_argument("error");
        }
    }

    void SetNextOut(PipelineContext* context) override
    {
    }

private:
    InboundLink<In>* nextIn;
};

template <class Handler>
class HandlerWrapper final : public HandlerWrapperBase {
public:
    using Context = typename Handler::Context;
    Context context;
    std::shared_ptr<Handler> handler;

    explicit HandlerWrapper(std::shared_ptr<Handler> handlerIn)
    {
        handler = handlerIn;
    }

    void Execute()
    {

    }
};

template <typename In>
class Pipeline {
public:
    template <class Handler>
    void AddBack(std::shared_ptr<Handler> handler)
    {
        auto wrapper = std::make_unique<HandlerWrapper<Handler>>(handler);
        handlers.push_back(std::move(wrapper));
    }

    void Close()
    {

    }

    void Execute(In input)
    {
        if (handlers.empty()) {
            return;
        }
    }

private:
    std::vector<std::unique_ptr<HandlerWrapperBase>> handlers;
};

//struct FirstHandler {
//    typedef InboundHandlerContext<std::string, std::string> Context;
//
//    std::string Execute(std::string text)
//    {
//        return text + text;
//    }
//};
//
//struct SecondHandler {
//    typedef BothHandlerContext<std::string, int> Context;
//
//    int Execute(std::string s)
//    {
//        return static_cast<int>(s.size());
//    }
//};
//
//struct ThirdHandler {
//    typedef OutboundHandlerContext<int, std::string> Context;
//
//    std::string Execute(int count)
//    {
//        std::cout << std::to_string(count) << std::endl;
//        return std::to_string(count);
//    }
//};
//
//void TestCase()
//{
//    auto pipeline = std::make_shared<Pipeline<std::string>>();
//    pipeline->AddBack(std::make_shared<FirstHandler>());
//    pipeline->AddBack(std::make_shared<SecondHandler>());
//    pipeline->AddBack(std::make_shared<ThirdHandler>());
//    pipeline->Close();
//
//    pipeline->Execute("miyako");
//}

void pipeContext(PipelineContext* in, PipelineContext* out)
{
    in->SetNextOut(out);
    out->SetNextIn(in);
}

void TestCase_Pipe()
{
    auto context1 = std::make_shared<InboundContextImplement<std::string>>();
    auto context2 = std::make_shared<ContextImplement<std::string, int>>();
    auto context3 = std::make_shared<OutboundContextImplement<int>>();

    pipeContext(context1.get(), context2.get());
    pipeContext(context2.get(), context3.get());
}

//template <class T>
//class ProcessorContext {
//public:
//    void wrtie(T)
//    {
//    }
//};

class EchoClientHandler {
public:
    void read(std::string text)
    {
    }
};

class SimpleClient {
public:
    void SetPipeline(std::shared_ptr<Pipeline<std::string>>)
    {
    }

    std::shared_ptr<Pipeline<std::string>> GetPipeline()
    {
        return {};
    }
};

void TestCase_Trivials()
{
    SimpleClient client;
    client.SetPipeline([]() -> std::shared_ptr<Pipeline<std::string>> {
        auto pipeline = std::make_shared<Pipeline<std::string>>();
        pipeline->AddBack(std::make_shared<EchoClientHandler>());
        pipeline->Close();
        return std::move(pipeline);
    }());

    auto pipeline = client.GetPipeline();
    pipeline->Write("hi");
}

} // unnamed namespace

int main(int argc, char *argv[])
{
    TestCase_Pipe();
//    TestCase();
    return 0;
}
