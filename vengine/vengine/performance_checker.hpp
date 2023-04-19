#pragma once 

#include <chrono>
#include <unistd.h>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>

enum class TIME_ID {
    ROOT,               // Not really used, but expresses everything
    AI_HANDLER_UPDATE,
    FSM_UPDATE,
    EVENT_UPDATE,
    EVENT_CHECK,
    BT_EXECUTE,
    BT_TASKS
};
static std::unordered_map<TIME_ID, std::string> TIME_ID_STR
{
    {TIME_ID::ROOT, "ROOT"},
    {TIME_ID::AI_HANDLER_UPDATE, "AI_HANDLER_UPDATE"},
    {TIME_ID::FSM_UPDATE, "FSM_UPDATE"},
    {TIME_ID::EVENT_UPDATE, "EVENT_UPDATE"},
    {TIME_ID::EVENT_CHECK, "EVENT_CHECK"},
    {TIME_ID::BT_EXECUTE, "BT_EXECUTE"},
    {TIME_ID::BT_TASKS, "BT_TASKS"}
};

struct TimeData 
{
    TimeData(std::chrono::duration<double, std::milli> totalTime, uint32_t nrOfAi, uint32_t totalTimePerTest_ms, uint32_t iterationCount, uint32_t nrOfFrames)
    : totalTime(totalTime)
    , nrOfAi(nrOfAi)
    , totalTimePerTest_ms(totalTimePerTest_ms)
    , iterationCount(iterationCount)
    , numberOfFrames(nrOfFrames)
    {}
    std::chrono::duration<double, std::milli> totalTime;
    uint32_t nrOfAi;
    uint32_t totalTimePerTest_ms;
    uint32_t iterationCount;
    uint32_t numberOfFrames;
};

struct TimeStruct
{
    std::chrono::duration<double, std::milli> totalTime;    // Seems to initially be 0...        
    std::chrono::system_clock::time_point currentStart;     // Seems to initially be 0...    
    std::chrono::system_clock::time_point currentEnd;       // Seems to initially be 0... 

    // Keep two coutners to make sure stop/start does not happens before their counterpart has happened
    uint32_t iterationCount_start = 0;
    uint32_t iterationCount_stop  = 0;
    uint32_t currentIterations  = 0;

    std::vector<TimeData> previousRecords;

    void calcTotalTime()
    {
        totalTime += currentEnd - currentStart;
    }
    void start()
    {
        currentStart = std::chrono::high_resolution_clock::now();
        iterationCount_start++;
        currentIterations++;
    }
    void stop()
    {
        currentEnd = std::chrono::high_resolution_clock::now();
        iterationCount_stop++;

        this->calcTotalTime();
    }
    bool isValid()
    {
        if(iterationCount_start == iterationCount_stop) {return true;}
        return false;        
    }

    uint32_t nrOfAi = 0;
    // inline const static uint32_t totalTimePerTest_ms = 30*1000;
    inline const static uint32_t totalTimePerTest_ms = 6*1000;

};

class PerfChecker 
{
    public: 

    std::unordered_map<TIME_ID, TimeStruct> times;
    std::unordered_map<TIME_ID, std::string> extra_headers;
    std::unordered_map<TIME_ID, std::string> extra_contents;
    std::unordered_map<TIME_ID, TIME_ID> parent_func;

    void startNextRun(uint32_t nrOfEntities, uint32_t nrOfFrames)
    {
        for(auto& time : times)
        {
            time.second.previousRecords.emplace_back(time.second.totalTime, nrOfEntities, time.second.totalTimePerTest_ms, time.second.currentIterations, nrOfFrames);
            time.second.totalTime = std::chrono::seconds(0);
            time.second.currentIterations = 0;
        }
    }

    void addParentFunc(TIME_ID func, TIME_ID parent)
    {
        parent_func[func] = parent;
    }

    void start(TIME_ID id)
    {
        this->times[id].start();
    }
    void stop(TIME_ID id)
    {
        this->times[id].stop();
    }

    void exportStats(const std::string& name)
    {
        try 
        {
            std::ofstream output(name +".csv");            

            if(output.is_open())
            {
                
                // Output functions relations
                output << "Name,ParentFunc" << std::endl;
                for(auto& time : times)
                {
                     
                    output << TIME_ID_STR[time.first] << "," 
                        << TIME_ID_STR[parent_func[time.first]]
                        << "\n";

                }
                
                // Output recorded times
                for(auto& time : times)
                {
                    if(time.second.isValid())
                    {
                        output << "\n";
                        output << "Nr of AI,Name,TimeSpent[ms],TotalRunTime[ms],Iterations,frames" << std::endl;
                        for(auto& record : time.second.previousRecords)
                        {
                            output 
                            << record.nrOfAi << ","
                            << TIME_ID_STR[time.first] << "," 
                            << record.totalTime.count() << "," 
                            << TimeStruct::totalTimePerTest_ms << ","
                            << record.iterationCount << ","
                            << record.numberOfFrames 
                            //<< (!extra_contents[time.first].empty() ? "," + extra_contents[time.first] : "") 
                            << "\n";
                        }
                        
                        
                    }
                    else 
                    {
                        throw("time was not valid!");
                    }
                }

            }

            output.close();

        }
        catch(std::exception e)
        {
            std::cout << "Runtime error: " << e.what() << "\n";
        }        
    }

    void basic_test()
    {
        // times 1 second 5 times, 
        // creates test_stats.csv should contain something like this: 
        /*
            Name	totalTime	NrIterations
            AI_HANDLER_UPDATE	5000.54	5
        */

        for(int j = 0; j < 5; j++)
        {
            this->start(TIME_ID::AI_HANDLER_UPDATE);
            sleep(1);
            this->stop(TIME_ID::AI_HANDLER_UPDATE);    
        }

        this->exportStats("test_stats");
    }

}; 

extern PerfChecker perfChecker;