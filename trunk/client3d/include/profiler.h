#ifndef PROFILER_H
#define PROFILER_H

//#define _PROFILE

#ifdef _PROFILE
#include <OgreRoot.h>
#include <vector>
#include <stack>


#include "logger.h"


// Todo: PROFILE(n) to log more than 1 task.

/**
 ** This singleton class provides profiling facility for a SINGLE task.
 ** Logs are stored in HTML format.
 ** This class uses Ogre::Timer, so only functions within the Ogre::Root
 ** lifetime are profiled.
 *****************************************************************************/
class SourcefileNode
{
private:
    typedef struct
    {
        std::string FunctionName;
        unsigned long SumCalls;
        unsigned long StartTime;
        unsigned long TotalTime;
    }
    Node;
    std::stack<int> mActiceFunc;

public:
    std::vector<Node*> mvFunction;
    std::string mSourcefileName;

    SourcefileNode(std::string FunctionName) : mSourcefileName(FunctionName)
    {
    }

    ~SourcefileNode()
    {
        for (std::vector<Node*>::iterator i = mvFunction.begin(); i < mvFunction.end(); ++i)
            delete (*i);
        mvFunction.clear();
    }

    static unsigned long getTime()
    {
        static unsigned long time = 0;
        time = (Ogre::Root::getSingletonPtr())?Ogre::Root::getSingleton().getTimer()->getMicroseconds():time;
        return time;
    }

    void startNode(std::string &FunctionName)
    {
        // Search for the node.
        for (size_t i = mvFunction.size(); i;)
        {
            if (mvFunction[--i]->FunctionName == FunctionName)
            {
                mActiceFunc.push(i);
                mvFunction[i]->StartTime = getTime();
                return;
            }
        }
        // This is a new node.
        mActiceFunc.push(mvFunction.size());
        Node *newChild = new Node;
        newChild->FunctionName = FunctionName;
        newChild->SumCalls = 0;
        newChild->TotalTime= 0;
        newChild->StartTime= getTime();
        mvFunction.push_back(newChild);
    }

    void stopNode()
    {
        size_t i =  mActiceFunc.top();
        mvFunction[i]->SumCalls++;
        mvFunction[i]->TotalTime+= getTime() - mvFunction[i]->StartTime;
        mActiceFunc.pop();
    }
};

class DProfiler
{
public:
    static DProfiler &getSingleton()
    {
        static DProfiler getSingleton;
        return getSingleton;
    }

    void startBlock(std::string file, std::string func, int /* line */)
    {
        Ogre::String filename, path;
        Ogre::StringUtil::splitFilename(file, filename, path);
        for (size_t i = mvSourceFile.size(); i;)
        {
            if (mvSourceFile[--i]->mSourcefileName == filename)
            {
                mActualSourceFile.push(i);
                mvSourceFile[i]->startNode(func);
                return;
            }
        }
        // This is a new node.
        mActualSourceFile.push(mvSourceFile.size());
        mvSourceFile.push_back(new SourcefileNode(filename));
        mvSourceFile[mActualSourceFile.top()]->startNode(func);
    }

    void endBlock()
    {
        mvSourceFile[mActualSourceFile.top()]->stopNode();
        mActualSourceFile.pop();
    }

private:
    std::vector<SourcefileNode*> mvSourceFile;
    std::ofstream mLogfile;
    std::stack<int> mActualSourceFile;
    unsigned long mProfileStartTime;
    float mPofileTime, mProfileTotalTime;

    DProfiler()
    {
        start =0;
        mPofileTime = 0.0f;
        mProfileStartTime = SourcefileNode::getTime();
    }

    ~DProfiler()
    {
        mProfileTotalTime = (SourcefileNode::getTime() - mProfileStartTime) / 1000.0f;
        mLogfile.open("profile_log.html");
        if(mLogfile.is_open())
        {
            mLogfile << "<html>\n<head>\n  <title>Profiling Report</title>\
            \n  <style type=\"text/css\">\
            \n    .tealtable1,.tealtable TD1, .tealtable TH1 { background-color:lightblue;  color:white;  }\
            \n    .tealtable2,.tealtable TD2, .tealtable TH2 { background-color:lightgreen; color:white; }\
            \n  </style>\n</head>\n\
            \n<body>\n <p>Daimonin Client3d Profiling Report</p>\n <font size=\"2\">" << std::endl;
            for (size_t i = mvSourceFile.size(); i;)
                printFunctions(--i);
            mLogfile << "\n  </table><br/> " << std::endl;
            mLogfile << "\n </font>\n</body>\n</html>" << std::endl;
            mLogfile.close();
        }
        for (std::vector<SourcefileNode*>::iterator i = mvSourceFile.begin(); i < mvSourceFile.end(); ++i)
            delete (*i);
        mvSourceFile.clear();
    }

    Ogre::String floatToStr(float value)
    {
        return Ogre::StringConverter::toString(value, 3, 3, '0',std::ios::fixed);
    }

    void printHeadline(const char *file)
    {
        mLogfile << "\
        \n  <table cellspacing=\"0\" cellpadding=\"0\" border=\"1\" bordercolor = white>\
        \n   <colgroup width=\"200\" span=\"4\"></colgroup>\
        \n   <tr align=\"left\" CLASS=\"tealtable1\"><th><font color=black>" << file <<"<font color=white></th>\
        \n   <th>Func Called</th> <th>Total Time [ms]</th> <th>Avg. Time [ms] </th> <th>% Total</th></tr>" << std::endl;
    }

    void printSummary(unsigned long sumCalls, float totalTime, float percent)
    {
        mLogfile << "   <tr align=\"right\" CLASS=\"tealtable2\"><th>Summary:</th>";
        if (!sumCalls)
        {
            mLogfile
            << " <th>N/A</th>"
            << " <th>" << floatToStr(totalTime) << "</th>"
            << " <th>N/A</th>"
            << " <th>" << floatToStr(percent) << "</td></tr>";
        }
        else
        {
            mLogfile
            << " <th>" << sumCalls << "</th>"
            << " <th>" << floatToStr(totalTime) << "</th>"
            << " <th>" << floatToStr(totalTime/sumCalls) << "</th>"
            << " <th>" << floatToStr(percent) << "</td></tr>";
        }
        mLogfile << "\n  </table><br/> " << std::endl;
    }

    void printFunctions(int e)
    {
        printHeadline(mvSourceFile[e]->mSourcefileName.c_str());
        float totalTime= 0.0f;
        unsigned long sumCalls = 0;
        for (size_t i = 0; i < mvSourceFile[e]->mvFunction.size(); ++i)
        {
            float ttime = mvSourceFile[e]->mvFunction[i]->TotalTime /1000.0f;
            float atime = ttime/mvSourceFile[e]->mvFunction[i]->SumCalls;
            sumCalls+= mvSourceFile[e]->mvFunction[i]->SumCalls;
            totalTime+=ttime;
            float ptime = (ttime/(float)(mProfileTotalTime))*100.0f;
            mLogfile << "   <tr align=\"right\">"
            << " <td>" << mvSourceFile[e]->mvFunction[i]->FunctionName << "</td>"
            << " <td>" << mvSourceFile[e]->mvFunction[i]->SumCalls     << "</td>"
            << " <td>" << floatToStr(ttime) << "</td>"
            << " <td>" << floatToStr(atime) << "</td>"
            << " <td>" << floatToStr(ptime) << "</td>"
            << "</tr>"<< std::endl;
        }
        printSummary(sumCalls, totalTime, (totalTime/mProfileTotalTime)*100.0f);
        mPofileTime+= totalTime;
    }
};

class ProfileHelper
{
public:
    /**
     ** Constructor
     *****************************************************************************/
    ProfileHelper(const char *file, const char *function, int line)
    {
        DProfiler::getSingleton().startBlock(file, function, line);
    }
    ~ProfileHelper()
    {
        DProfiler::getSingleton().endBlock();
    }
};

#define PROFILE() ProfileHelper ProfileHelperGo(__FILE__,__FUNCTION__,__LINE__);
#else // Disable Profiling.
#define PROFILE()
#endif

#endif
