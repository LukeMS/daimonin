/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html
 
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.
 
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "textwindow.h"
#include "option.h"
#include "animate.h"
#include "logger.h"
#include "sound.h"
#include "object_manager.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
const char *StateNames[STATE_SUM]=
  {
    "Idle1", "Idle2", "Idle3",
    "Walk1", "Walk2", "Walk3",
    "Run1",  "Run2",  "Run3",
    "Attack1", "Attack2", "Attack3",
    "Block1", "Block2", "Block3",
    "Slump1", "Slump2", "Slump3",
    "Death1", "Death2", "Death3",
    "Hit1",  "Hit2",  "Hit3",
    "Cast1", "Cast2", "Cast3"
  };

const std::string sndPreFix  = "Sound_";
const std::string animPreFix = "Anim_";

//=================================================================================================
// Constructor. (Description File must be open when you call me.)
//=================================================================================================
Animate::Animate(Entity *entity)
{
  std::string animName, soundName;
  Option::getSingleton().getDescStr("Speed_Anim", mStrTemp);
  mAnimSpeed = atoi(mStrTemp.c_str())/100;

  Option::getSingleton().getDescStr("Speed_Turn", mStrTemp);
  mTurnSpeed = atoi(mStrTemp.c_str())/100;

  // fill the animation states.
  for (int state = 0; state < STATE_SUM; ++state)
  {
    animName = animPreFix + StateNames[state];
    if (Option::getSingleton().getDescStr(animName.c_str(), mStrTemp))
    {
      mAnimStates[state] = entity->getAnimationState(mStrTemp.c_str());
      if (!mAnimStates[state])
        Logger::log().error() << "Critical: No Animatin named "
        << mStrTemp
        << " found!";
    }
    else // not found animation-name in description file -> use the standard one (IDLE1).
    {
      //            LogFile::getSingleton().Info("- Animation: %s has no animation defined\n", animName.c_str());
      Option::getSingleton().getDescStr(StateNames[STATE_IDLE1], mStrTemp);
      mAnimStates[state] = entity->getAnimationState(mStrTemp.c_str());
    }
    // load the sound for this animation.
    soundName = sndPreFix + StateNames[state];
    if (Option::getSingleton().getDescStr(soundName.c_str(), mStrTemp))
    {
      mStrTemp = DIR_SAMPLES + mStrTemp;
      sound_handle[state] = Sound::getSingleton().loadSample(mStrTemp.c_str());
    }
    else
    {
      // LogFile::getSingleton().Info("- Animation: %s has no sound defined\n", soundName.c_str());
      sound_handle[state] = Sound::getSingleton().loadSample(FILE_SAMPLE_DUMMY);
    }
  }
  mAnimGroup = 0;
  mAnimType  =-1;
  mAnimState = mAnimStates[STATE_IDLE1];
  mAnimState->setEnabled(true);
  mAnimState->setLoop(false);
  mSpellTrigger = false;
}

//=================================================================================================
//
//=================================================================================================
void Animate::toggleAnimGroup()
{
  if (++mAnimGroup >2)
  {
    mAnimGroup =0;
  }
  toggleAnimation(mAnimType, true);
  char buf[80];
  sprintf(buf, "AnimGroup No %d is now active.", mAnimGroup+1);
  TextWin->Print(buf, TXT_WHITE);
}

//=================================================================================================
//
//=================================================================================================
void Animate::update(const FrameEvent& event)
{
  mAnimState = mAnimStates[mAnimType + mAnimGroup];
  mAnimState->addTime(event.timeSinceLastFrame * mAnimSpeed);
  // if an animation ends, then force the idle animation.
  if (mAnimState->getTimePosition() >= mAnimState->getLength())
  {
    toggleAnimation(STATE_IDLE1, true);
  }
  if (!mSpellTrigger && mAnimType >= STATE_CAST1)
  {
    if (mAnimState->getTimePosition() >= 1)
    {
      const int SPELL_FIREBALL =0;
      ObjectManager::getSingleton().castSpell(OBJECT_PLAYER, SPELL_FIREBALL);
      mSpellTrigger = true;
    }
  }
}

//=================================================================================================
//
//=================================================================================================
void Animate::toggleAnimation(int animationNr, bool force)
{
  if (!force && (mAnimType == animationNr || ( !isMovement() && animationNr >= STATE_ATTACK1)))
  {
    return;
  }
  mSpellTrigger = false;
  mAnimType = animationNr;
  mAnimState->setEnabled(false);
  mAnimState = mAnimStates[mAnimType + mAnimGroup];
  mAnimState->setEnabled(true);
  mAnimState->setTimePosition(0);
  mAnimState->setLoop(false);
  if (mAnimType > STATE_RUN3) Sound::getSingleton().playSample(sound_handle[animationNr -1 + mAnimGroup]);
}
