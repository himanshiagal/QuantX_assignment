#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <cstring>
#include <filesystem>
#include "Logger.h"
#include "configLoader.h"


Logger logger("../log.txt");


struct ScriptQuote
{
    int32_t bid;
    int32_t ask;
    int32_t bidQty;
    int32_t askQty;
    int32_t ltp;
    std::string ScriptName; // use std::string to avoid manual memory management
    long long Time;
    int fileIndex; // still needed to identify which file this quote came from

    ScriptQuote(std::string name, long long ts, int32_t b, int32_t a, int32_t bq, int32_t aq, int32_t l, int idx)
        : ScriptName(name), Time(ts), bid(b), ask(a), bidQty(bq), askQty(aq), ltp(l), fileIndex(idx) {}

    bool operator>(const ScriptQuote &other) const
    {
        return Time > other.Time;
    }
};

long long createTimeStamp(const std::string &date, const std::string &time)
{
    int day, month, year, hour, minute, second;
    sscanf(date.c_str(), "%d/%d/%d", &day, &month, &year);  // Fixed order
    sscanf(time.c_str(), "%d:%d:%d", &hour, &minute, &second);
    return year * 10000000000LL + month * 100000000 + day * 1000000 + hour * 10000 + minute * 100 + second;
}


struct FileReader
{
    std::ifstream file;  // Input file stream to read the file
    std::string leftoverLine;  // Holds leftover part of the line that wasn't processed
    bool isFirstLine = true;  // Flag to identify the first line of the file

    FileReader(const std::string &fname) : file(fname, std::ios::in)
    {
        if (!file.is_open())
        {
            logger.log("Failed to open file: " + fname);  
            throw std::runtime_error("Failed to open file: " + fname);
        }
    }

    // Function to read the next chunk of data from the file, process it, and store it in 'quotes'
    bool readNextChunk(size_t bufferSize, std::vector<ScriptQuote> &quotes, int fileIndex)
    {
        if (!file.good())
            return false;

        try
        {

            char *buffer = new char[bufferSize];
            file.read(buffer, bufferSize);
            std::string chunk(buffer, file.gcount());
            delete[] buffer;

            if (chunk.empty())
                return false;

            // If there's leftover data from the previous chunk, append it to the current chunk
            if (!leftoverLine.empty())
            {
                chunk = leftoverLine + chunk;
                leftoverLine.clear();
            }

            std::stringstream ss(chunk);
            std::string line;

            while (std::getline(ss, line))
            {
                // Check if it's the last line and it doesn't end with a newline, save it as leftover
                if (ss.eof() && chunk.back() != '\n')
                {
                    leftoverLine = line;
                    break;
                }

                // Skip the first line (typically headers or unwanted info)
                if (isFirstLine)
                {
                    isFirstLine = false;
                    continue;
                }

                // Process the data line
                std::stringstream lineStream(line);
                std::string ticker, date, time;
                float f_ltp, f_buyPrice, f_sellPrice, ltq, f_buyQty, f_sellQty, openInterest;
                int32_t ltp, buyPrice, buyQty, sellPrice, sellQty;

                // Extract each comma-separated value from the line
                std::getline(lineStream, ticker, ',');
                std::getline(lineStream, date, ',');
                std::getline(lineStream, time, ',');

                // Read as float first
                lineStream >> f_ltp;
                lineStream.ignore(1, ',');
                lineStream >> f_buyPrice;
                lineStream.ignore(1, ',');
                lineStream >> f_buyQty;
                lineStream.ignore(1, ',');
                lineStream >> f_sellPrice;
                lineStream.ignore(1, ',');
                lineStream >> f_sellQty;
                lineStream.ignore(1, ',');
                lineStream >> ltq;
                lineStream.ignore(1, ',');
                lineStream >> openInterest;

                // Create a timestamp from the date and time
                long long ts = createTimeStamp(date, time);

               // Convert floating point values to paise (integer format)
                ltp = static_cast<int32_t>(f_ltp * 100 + 0.5);
                buyPrice = static_cast<int32_t>(f_buyPrice * 100 + 0.5);
                sellPrice = static_cast<int32_t>(f_sellPrice * 100 + 0.5);
                buyQty = static_cast<int32_t>(f_buyQty);
                sellQty = static_cast<int32_t>(f_sellQty);

                quotes.emplace_back(ticker, ts, buyPrice, sellPrice, buyQty, sellQty, ltp, fileIndex);
            }
        }
        catch (const std::exception &e)
        {
            std::string errorMessage = "Error reading file chunk: " + std::string(e.what());
            logger.log(errorMessage);  
            std::cerr << errorMessage << std::endl;
            return false;
        }

        return !quotes.empty();
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        logger.log("Error: Config file path must be provided as a command line argument!");
        std::cerr << "Error: Config file path must be provided as a command line argument!" << std::endl;
        return 1; // Exit with error code
    }

    std::string configPath = argv[1];
    std::map<std::string, std::string> config;


    logger.log("Attempting to load config from: " + configPath);
    try
    {
        config = loadConfig(configPath);
        logger.log("Config loaded successfully."); 
    }
    catch (const std::exception &e)
    {
        logger.log("Error loading config: " + std::string(e.what()));
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return 1; // Exit with error code
    }

    // Lambda function to fetch a config value by key. Exits the program if the key is missing.
    auto getConfigValue = [&](const std::string &key) -> std::string
    {
        if (config.find(key) == config.end())
        {
            std::cerr << "Missing key in config: " << key << std::endl;
            logger.log("Missing key in config: " + key); // Log missing key
            std::exit(1); // Exit if a critical config value is missing
        }
        return config[key];
    };



    try
    {
        long long sessionStart = std::stoll(getConfigValue("session_start"));
        long long sessionEnd = std::stoll(getConfigValue("session_end"));
        std::string folderPath = getConfigValue("input_folder");
        std::string outputPath = getConfigValue("output_file");
        size_t bufferSize = std::stoul(getConfigValue("buffer_size"));
        std::string prefix = getConfigValue("file_prefix");


        logger.log("Session Start: " + std::to_string(sessionStart));
        logger.log("Session End: " + std::to_string(sessionEnd));
        logger.log("Input Folder: " + folderPath);
        logger.log("Output File: " + outputPath);

        std::vector<std::string> filenames;

        logger.log("Searching for files in folder: " + folderPath);

        // Iterate over the files in the input folder and find the ones with the given prefix
        for (const auto &entry : std::filesystem::directory_iterator(folderPath))
        {
            if (entry.is_regular_file())
            {
                std::string path = entry.path().string();
                std::string filename = entry.path().filename().string();
                if (filename.rfind(prefix, 0) == 0)
                { 
                    filenames.push_back(path);
                    logger.log("Found file with prefix: " + filename);
                }
            }
        }

        std::vector<FileReader> readers;
        std::priority_queue<ScriptQuote, std::vector<ScriptQuote>, std::greater<ScriptQuote>> minHeap;

        // Initialize
        for (int i = 0; i < filenames.size(); ++i)
        {
            readers.emplace_back(filenames[i]);
            std::vector<ScriptQuote> quotes;
            if (readers[i].readNextChunk(bufferSize, quotes, i))
            {
                // Add quotes within the session start and end times to the priority queue
                for (const auto &q : quotes)
                {
                    if (q.Time >= sessionStart && q.Time <= sessionEnd)
                    {
                        minHeap.push(q); // Push into heap only if within range
                    }
                }
            }
        }

        // Open the output file for writing the results
        std::ofstream out(outputPath);
        out << "ScriptName,TimeStamp,Bid,Ask,Bid_Quantity,Ask_Quntity,ltp" << std::endl;
        while (!minHeap.empty())
        {
            ScriptQuote top = minHeap.top();
            minHeap.pop();

            out << top.ScriptName << "," << top.Time << ","
                << top.bid << ","
                << top.ask << ","
                << top.bidQty << ","
                << top.askQty << "," << top.ltp << std::endl;

            std::vector<ScriptQuote> nextQuotes;

            // Read the next chunk of quotes from the corresponding file and add them to the heap if valid
            if (readers[top.fileIndex].readNextChunk(bufferSize, nextQuotes, top.fileIndex))
            {
                for (const auto &q : nextQuotes)
                {
                    if (q.Time >= sessionStart && q.Time <= sessionEnd)
                    {
                        minHeap.push(q);
                    }
                }
            }
        }
        out.close();
        logger.log("Processing complete. Output written to: " + outputPath);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        logger.log("Exception: " + std::string(e.what()));
        return 1;
    }

    return 0;
}
