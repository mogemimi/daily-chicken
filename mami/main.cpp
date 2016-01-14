// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include <iostream>
#include <fstream>
#include <regex>

namespace {

class HandlerBase {
public:
    virtual ~HandlerBase() = default;
};

template <class PipeIn, class PipeOut>
class Handler : public HandlerBase {
public:
    using In = PipeIn;
    using Out = PipeOut;

    virtual ~Handler() = default;

    virtual Out Execute(In in) = 0;
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

    virtual void Write(In request) = 0;
};

template <class Out>
class OutboundLink {
public:
    virtual ~OutboundLink() = default;
};

class PipelineContext {
public:
    virtual ~PipelineContext() = default;

    virtual void SetPrevIn(PipelineContext* context) = 0;

    virtual void SetNextOut(PipelineContext* context) = 0;
};

template <class In, class Out>
class ContextImplement final
    : public InboundLink<In>
    , public OutboundLink<Out>
    , public PipelineContext {
public:
    using HandlerType = Handler<In, Out>;

    explicit ContextImplement(std::shared_ptr<HandlerType> handlerIn)
        : handler(handlerIn)
    {
    }

    void SetPrevIn(PipelineContext* context) override
    {
        if (context == nullptr) {
            nextIn = nullptr;
            return;
        }
        auto newNextIn = dynamic_cast<OutboundLink<In>*>(context);
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
        auto newNextOut = dynamic_cast<InboundLink<Out>*>(context);
        if (newNextOut) {
            nextOut = newNextOut;
        }
        else {
            throw std::invalid_argument("error");
        }
    }

    void Write(In request) override
    {
        auto result = handler->Execute(request);
        if (nextOut) {
            nextOut->Write(result);
        }
    }

private:
    std::shared_ptr<HandlerType> handler;
    OutboundLink<In>* nextIn = nullptr;
    InboundLink<Out>* nextOut = nullptr;
};

template <class In>
class InboundContextImplement final
    : public InboundLink<In>
    , public PipelineContext {
public:
    void SetPrevIn(PipelineContext* context) override
    {
        if (context == nullptr) {
            nextIn = nullptr;
            return;
        }
        auto newNextIn = dynamic_cast<OutboundLink<In>*>(context);
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

    void Write(In request) override
    {
        throw std::invalid_argument("not found");
    }

private:
    OutboundLink<In>* nextIn;
};

template <class PipeOut>
class OutboundContextImplement final
    : public OutboundLink<PipeOut>
    , public PipelineContext {
public:
    void SetPrevIn(PipelineContext* context) override
    {
        if (context == nullptr) {
            nextIn = nullptr;
            return;
        }
        auto newNextIn = dynamic_cast<InboundLink<PipeOut>*>(context);
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
    InboundLink<PipeOut>* nextIn;
};

template <class In, class Out>
class Pipeline {
public:
    template <class HandlerType>
    void AddBack(std::shared_ptr<HandlerType> handler)
    {
        static_assert(std::is_base_of<HandlerBase, HandlerType>::value, "");
        using InType = typename HandlerType::In;
        using OutType = typename HandlerType::Out;
        using Context = ContextImplement<InType, OutType>;
        auto context = std::make_shared<Context>(handler);
        contexts.push_back(std::move(context));
    }

    void Build()
    {
        front = nullptr;
        back = nullptr;
        if (contexts.empty()) {
            return;
        }
        front = dynamic_cast<InboundLink<In>*>(contexts.front().get());
        back = dynamic_cast<OutboundLink<Out>*>(contexts.back().get());
        for (size_t i = 0; i < contexts.size() - 1; i++) {
            contexts[i]->SetNextOut(contexts[i + 1].get());
            contexts[i + 1]->SetPrevIn(contexts[i].get());
        }
    }

    void Write(In input)
    {
        if (contexts.empty()) {
            return;
        }
        if (front) {
            front->Write(input);
        }
    }

private:
    std::vector<std::shared_ptr<PipelineContext>> contexts;
    InboundLink<In>* front;
    OutboundLink<Out>* back;
};

class Handler1 final : public Handler<std::string, int> {
public:
    int Execute(std::string text) override
    {
        return (int)text.size();
    }
};

class Handler2 final : public Handler<int, std::string> {
public:
    std::string Execute(int count) override
    {
        return std::to_string(count);
    }
};

class StdOutHandler final : public Handler<std::string, std::string> {
public:
    std::string Execute(std::string text) override
    {
        std::cout << text << std::endl;
        return text;
    }
};

void TestCase_Trivials()
{
    auto pipeline = std::make_shared<Pipeline<std::string, std::string>>();
    pipeline->AddBack(std::make_shared<Handler1>());
    pipeline->AddBack(std::make_shared<Handler2>());
    pipeline->AddBack(std::make_shared<StdOutHandler>());
    pipeline->Build();

    pipeline->Write("hi");
    pipeline->Write("hihi");
    pipeline->Write("hihihi");
}

} // unnamed namespace

int main(int argc, char *argv[])
{
    TestCase_Trivials();
    printf("ok\n");
    return 0;
}
