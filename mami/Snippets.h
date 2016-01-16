// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#pragma once

#if 0

class FirstFilterHandler final : public Handler<int, double> {
public:
    double Execute(int n) override
    {
        return static_cast<double>(n) * 0.5;
    }
};

class SecondFilterHandler final : public Handler<double, std::string> {
public:
    std::string Execute(double count) override
    {
        return std::to_string(count);
    }
};

class StdOutHandler final : public InboundHandler<std::string> {
public:
    void Execute(std::string text) override
    {
        std::cout << text << std::endl;
    }
};

void TestCase_Trivials()
{
    auto pipeline = std::make_shared<Pipeline<int, std::string>>();
    pipeline->AddBack(std::make_shared<FirstFilterHandler>());
    pipeline->AddBack(std::make_shared<SecondFilterHandler>());
    pipeline->AddBack(std::make_shared<StdOutHandler>());
    pipeline->Build();

    pipeline->Write(42);
}

class SocketHandler final : public Handler<int, double> {
public:
    double Execute(int n) override
    {
        return static_cast<double>(n) * 0.5;
    }
};

#endif
