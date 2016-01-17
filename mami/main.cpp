// Copyright (c) 2016 mogemimi. Distributed under the MIT license.

#include "IOService.h"
#include "CommandLineParser.h"

using namespace somera;

namespace {

void SetupCommandLineParser(CommandLineParser & parser)
{
    using somera::CommandLineArgumentType::Flag;
    using somera::CommandLineArgumentType::JoinedOrSeparate;
    parser.setUsageText("mami [options ...]");
    parser.addArgument("-h", Flag, "Display available options");
    parser.addArgument("-help", Flag, "Display available options");
    parser.addArgument("-s", Flag, "server");
    parser.addArgument("-c", Flag, "client");
    parser.addArgument("port", JoinedOrSeparate, "port number (default is 8000)");
}

void Log(const std::string& text)
{
    std::puts(text.c_str());
}

void RunServer(uint16_t port)
{
    IOService service;

    TcpServerSocket socket(service);
    socket.Bind(EndPoint::CreateFromV4("localhost", port));
    socket.Listen(5, [](Socket & client, const Error&) {
        Log(StringHelper::format("Listen: client = { fd : %d }", client.GetHandle()));
    });
    socket.Read([](Socket & client, const ArrayView<uint8_t>& view) {
        std::string text(reinterpret_cast<const char*>(view.data), view.size);
        text = StringHelper::trimRight(text, '\n');
        text = StringHelper::trimRight(text, '\r');
        text = StringHelper::trimRight(text, '\n');

        Log(StringHelper::format(
            "Read: client = { fd : %d, result = %s }",
            client.GetHandle(),
            text.c_str()));
    });

    Log("Run");
    service.Run();
}

void RunClient(uint16_t port)
{
    IOService service;

    TcpSocket socket(service);
    auto onConnected = [](Socket & client, const Error& error) {
        if (error) {
            Log(error.What());
            return;
        }
        Log("Connected.");

        std::string text = "Hello, socket!";
        client.Send(text.data(), text.size());
    };
    socket.Connect(EndPoint::CreateFromV4("localhost", port), onConnected);
    socket.Read([](Socket & client, const ArrayView<uint8_t>& view) {
        std::string text(reinterpret_cast<const char*>(view.data), view.size);
        text = StringHelper::trimRight(text, '\n');
        text = StringHelper::trimRight(text, '\r');
        text = StringHelper::trimRight(text, '\n');

        Log(StringHelper::format(
            "Read: server = { fd : %d, result = %s }",
            client.GetHandle(),
            text.c_str()));
    });

    Log("Run");
    service.Run();
}

} // unnamed namespace

int main(int argc, char *argv[])
{
    somera::CommandLineParser parser;
    SetupCommandLineParser(parser);
    parser.parse(argc, argv);

    if (parser.hasParseError()) {
        std::cerr << parser.getErrorMessage() << std::endl;
        return 1;
    }
    if (parser.exists("-h") || parser.exists("-help")) {
        std::cout << parser.getHelpText() << std::endl;
        return 0;
    }

    uint16_t port = 8000;
    if (auto p = parser.getValue("port")) {
        try {
            port = static_cast<uint16_t>(std::stoi(*p));
        }
        catch (const std::invalid_argument&) {
            return 1;
        }
    }
    if (parser.exists("-s")) {
        RunServer(port);
    }
    else if (parser.exists("-c")) {
        RunClient(port);
    }

    Log("Done");
    return 0;
}
