/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <fstream>

#include "logfile.h"
#include "define.h"
#include "mesh.h"
#include "skeleton.h"
#include "mainwindow.h"

struct animation
{
    // Animation name.
    string nameAnim;
    bool   nameAnimOK;
    int    lineNr;
    float  lenAnim;
    // Animation frames.
    vector<float> vecKeyframeTime;
    vector<int>   vecKeyframeLine;    
    int sumKeyframes;
};

vector<animation*> vecAnimation;
vector<string> vecSkelXML;
vector<string>mvecAnimNames;
string mSkeletonData, mSkelFilename;


// ========================================================================
// 
// ========================================================================
void Skeleton::renameAnim()
{
    if (gpMainWin->selRenameAnim->value() > gpMainWin->selRenameAnim->size()) { return; }
    if (gpMainWin->selRenameAnim->size() == 0) { return; }
    if (!fl_ask("Do you want to write the new Animation name?")) { return; }
    mAnimNr = gpMainWin->selAnimName->value();
    vecAnimation[mAnimNr]->nameAnim = mvecAnimNames[gpMainWin->selRenameAnim->value()];
    vecAnimation[mAnimNr]->nameAnimOK = true;    
    gpMainWin->selAnimName->clear();
    for (unsigned int i=0; i < vecAnimation.size(); ++i) { gpMainWin->selAnimName->add(vecAnimation[i]->nameAnim.c_str()); }
    selAnimation(mAnimNr);
    sprintf(mBuffer, "\t<animation name=\"%s\" length=\"%f\">", vecAnimation[mAnimNr]->nameAnim.c_str(), vecAnimation[mAnimNr]->lenAnim); 
    vecSkelXML[vecAnimation[mAnimNr]->lineNr] = mBuffer;
    checkAnimNames(); 
    save(gpMainWin->SkeletonName->value());
}

// ========================================================================
// 
// ========================================================================
bool Skeleton::Init()
{
    LogFile::getSingleton().Info("Open Animation Names: '%s'..." , FILE_ANIM_NAMES);
    ifstream inAnimNames(FILE_ANIM_NAMES);
    if (!inAnimNames)
    {
        LogFile::getSingleton().Success(false);
    }
    LogFile::getSingleton().Success(true);    
    string line;
    while(getline(inAnimNames, line))
    { 
        mvecAnimNames.push_back(line);
        gpMainWin->selRenameAnim->add(line.c_str());
    }
    gpMainWin->butRenameAnim->deactivate();
    gpMainWin->selRenameAnim->deactivate();
    return true;
}

// ========================================================================
// 
// ========================================================================
void Skeleton::updateAnimLength()
{
    if (gpMainWin->selAnimName->size() == 0) { return; }
    float len = vecAnimation[mAnimNr]->lenAnim;
    sprintf(mBuffer, "%.2f", len);
    gpMainWin->outAnimLen->value(mBuffer);
    len *= gpMainWin->countAnimLength->value();
    sprintf(mBuffer, "%.2f", len);
    gpMainWin->outAnimNewLen->value(mBuffer);

}

// ========================================================================
// 
// ========================================================================
void Skeleton::changeAnimLength()
{
    if (gpMainWin->selAnimName->value() > gpMainWin->selAnimName->size()) { return; }
    if (gpMainWin->selAnimName->size() == 0) { return; }
    if (!fl_ask("Do you want to write the new Animation length?")) { return; } 
    float multiplier = gpMainWin->countAnimLength->value();
    vecAnimation[mAnimNr]->lenAnim *= multiplier;
    
    for (int i = 0; i < vecAnimation[mAnimNr]->sumKeyframes; ++i)
    {
        sprintf(mBuffer, "\t\t\t<keyframe time=\"%f\">", vecAnimation[mAnimNr]->vecKeyframeTime[i] *multiplier); 
        vecSkelXML[vecAnimation[mAnimNr]->vecKeyframeLine[i]] = mBuffer;
    }
    sprintf(mBuffer, "\t<animation name=\"%s\" length=\"%f\">", vecAnimation[mAnimNr]->nameAnim.c_str(), vecAnimation[mAnimNr]->lenAnim); 
    vecSkelXML[vecAnimation[mAnimNr]->lineNr] = mBuffer;
    updateAnimLength();
    save(gpMainWin->SkeletonName->value());
}

// ========================================================================
// 
// ========================================================================
void Skeleton::save(const char *filename)
{
    ofstream outSkel(filename);
    for(unsigned int i = 0; i < vecSkelXML.size(); ++i) { outSkel << vecSkelXML[i] << endl; }
    string file  = FILE_OGRE_XML_CONV;
    file += " ";
    file += filename;
    LogFile::getSingleton().Info("Executing: '%s'\n" , file.c_str());    
    system(file.c_str());
}

// ========================================================================
// 
// ========================================================================
void Skeleton::selAnimation(int animNr)
{
    if (gpMainWin->selAnimName->size() == 0) { return; }
    mAnimNr = animNr;
    gpMainWin->selAnimName->value(mAnimNr);
    if (vecAnimation[animNr]->nameAnimOK)
    {
        gpMainWin->butRenameAnim->deactivate();
        gpMainWin->selRenameAnim->deactivate();
    }
    else
    {
        gpMainWin->butRenameAnim->activate();
        gpMainWin->selRenameAnim->activate();
    }
    updateAnimLength();
}

// ========================================================================
// 
// ========================================================================
void Skeleton::checkAnimNames()
{
    mAnimNamesOK = true;
    for (unsigned int i = 0; i < vecAnimation.size(); ++i)
    {   
        if (vecAnimation[i]->nameAnimOK == false)
        {
            mAnimNamesOK = false;
            break;
        }
    }

    if (!mAnimNamesOK)
    { 
        gpMainWin->txtAnimNameStatus->value("Error: Found not daimonin conform Animation-Name(s)."); 
        gpMainWin->txtAnimNameStatus->textcolor(FL_RED);
    }
    else
    {
        gpMainWin->txtAnimNameStatus->value("All Anmiations-Names are daimonin conform."); 
        gpMainWin->txtAnimNameStatus->textcolor(FL_BLACK);        
    }
}

// ========================================================================
// 
// ========================================================================
void Skeleton::clearAnimations()
{
    vecSkelXML.clear();
    for (unsigned int i = 0; i < vecAnimation.size(); i++) { delete vecAnimation[i]; }
    vecAnimation.clear();
}

// ========================================================================
// 
// ========================================================================
void Skeleton::load(const char *filename)
{
    mSkelFilename = filename;
    string line;
    mAnimNamesOK = true;
    clearAnimations();
    
    // Read in SkeletonFile.
    ifstream inSkeleton(mSkelFilename.c_str());
    if (!inSkeleton)
    {
        LogFile::getSingleton().Error("Could not open skeleton-file '%s'\n" , mSkelFilename.c_str());
        return;
    }
    LogFile::getSingleton().Info("Opend Skeletonfile: '%s'\n" , mSkelFilename.c_str());
    bool sectAnim = false;
    int  sumAnim  =0;
    int lineNr =-1;
    int start, stop;
    animation *anim;
    gpMainWin->selAnimName->clear();
    while(getline(inSkeleton, line))
    { 
        ++lineNr;
        vecSkelXML.push_back(line);
        if (line.find("animation>")     != string::npos) { sectAnim = false; }
        if (line.find("animation name") != string::npos)
        { 
            sectAnim = true;
            anim = new animation;
            start = line.find("=\"")+2;
            stop  = line.find("\"",start);
            anim->nameAnim = line.substr(start, stop - start);
            anim->nameAnimOK = false;
            for (unsigned int i = 0; i < mvecAnimNames.size(); ++i)
            {   
                if (anim->nameAnim == mvecAnimNames[i])
                {
                    anim->nameAnimOK = true;
                    break;
                }
            }
            start = line.find("\"", ++stop);
            stop  = line.find("\"", ++start);
            line = line.substr(start, stop-start); 
            anim->lenAnim  = atof(line.c_str());
            anim->lineNr = lineNr;
            anim->sumKeyframes =0;
            vecAnimation.push_back(anim);
            line = anim->nameAnim;
            if (!anim->nameAnimOK)
            { 
                line+= "  (ERROR: Not daimonin conform - MUST be renamed!)";
                mAnimNamesOK = false;
            }
            gpMainWin->selAnimName->add(line.c_str());
            ++sumAnim;
            continue;
        }

        if (sectAnim && line.find("keyframe time") != string::npos )
        { 
            start = line.find("\"")+1;
            line = line.substr(start, line.find("\"",start) - start);
            float time = atof(line.c_str());
            anim->vecKeyframeTime.push_back(time);
            anim->vecKeyframeLine.push_back(lineNr);
            ++anim->sumKeyframes;
        }
    }
    gpMainWin->SkeletonName->value(mSkelFilename.c_str());
    checkAnimNames();
    selAnimation(0);
}
