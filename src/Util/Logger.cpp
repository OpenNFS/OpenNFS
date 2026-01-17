#include "Logger.h"

using namespace g3;

namespace OpenNFS {
    Logger::Logger() {
        // Set up logging framework
        static std::unique_ptr<LogWorker> gMainLogger{LogWorker::createLogWorker()};
        auto const defaultHandler{gMainLogger->addSink(std::make_unique<FileSink>("OpenNFS", LOG_FILE_PATH, ""), &FileSink::fileWrite)};

        auto changeFormatting{defaultHandler->call(&FileSink::overrideLogDetails, &FormatLog)};
        std::string const newHeader{"\t\tLOG format: [hh:mm:ss FILE->FUNCTION:LINE]: message\n\t\t\n\n"};
        auto changeHeader{defaultHandler->call(&FileSink::overrideLogHeader, newHeader)};
        auto coutSinkHandle{gMainLogger->addSink(std::make_unique<ColourCoutSink>(), &ColourCoutSink::ReceiveLogMessage)};
        auto onScreenSinkHandle{gMainLogger->addSink(std::make_unique<OnScreenLogSink>(&onScreenLog), &OnScreenLogSink::ReceiveLogMessage)};

        // Logger is initialized
        initializeLogging(gMainLogger.get());
    }
} // namespace OpenNFS
