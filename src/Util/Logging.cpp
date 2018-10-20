//
// Created by SYSTEM on 20/10/2018.
//

#include "Logging.h"

using namespace g3;

void Logging::InitialiseLogging() {
    // Set up logging framework
    auto worker = LogWorker::createLogWorker();
    auto defaultHandler = worker->addDefaultLogger("OpenNFS", LOG_FILE_PATH, "");
    auto changeFormatting = defaultHandler->call(&FileSink::overrideLogDetails, LogMessage::FullLogDetailsToString);
    const std::string newHeader = "\t\tLOG format: [hh:mm:ss FILE->FUNCTION:LINE]: message\n\t\t\n\n";
    auto changeHeader = defaultHandler->call(&FileSink::overrideLogHeader, newHeader);
    auto sinkHandle = worker->addSink(std::make_unique<ColorCoutSink>(), &ColorCoutSink::ReceiveLogMessage);
    //auto onScreenSinkHandle = worker->addSink(std::make_unique<OnScreenLogSink>(), &OnScreenLogSink::ReceiveLogMessage);

    // logger is initialized
    initializeLogging(worker.get());
}