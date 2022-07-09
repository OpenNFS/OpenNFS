#include "Logger.h"

using namespace g3;

Logger::Logger() {
    // Set up logging framework
    static std::unique_ptr<LogWorker> gMainLogger = LogWorker::createLogWorker();
    auto defaultHandler                           = gMainLogger->addSink(std::make_unique<FileSink>("OpenNFS", LOG_FILE_PATH, ""), &FileSink::fileWrite);

    auto changeFormatting       = defaultHandler->call(&FileSink::overrideLogDetails, LogMessage::FullLogDetailsToString);
    const std::string newHeader = "\t\tLOG format: [hh:mm:ss FILE->FUNCTION:LINE]: message\n\t\t\n\n";
    auto changeHeader           = defaultHandler->call(&FileSink::overrideLogHeader, newHeader);
    auto coutSinkHandle         = gMainLogger->addSink(std::make_unique<ColorCoutSink>(), &ColorCoutSink::ReceiveLogMessage);
    auto onScreenSinkHandle     = gMainLogger->addSink(std::make_unique<OnScreenLogSink>(&onScreenLog), &OnScreenLogSink::ReceiveLogMessage);

    // logger is initialized
    initializeLogging(gMainLogger.get());
}
