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

#ifndef SOUND_H
#define SOUND_H

////////////////////////////////////////////////////////////
/// Defines.
////////////////////////////////////////////////////////////
enum SampleName
{
  SAMPLE_BUTTON_CLICK,
  SAMPLE_PLAYER_IDLE,
  SAMPLE_SUM
};

////////////////////////////////////////////////////////////
/// Singleton class.
////////////////////////////////////////////////////////////
class Sound
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  static Sound &getSingleton()
  {
    static Sound Singleton; return Singleton;
  }
  bool Init();
  void freeRecources();
  // Streams.
  void playStream( const char *filename );
  void stopStream();
  // Songs.
  void playSong( const char *filename );
  void stopSong();
  // Samples.
  void setSamplePos3D( unsigned int channel,  float &posX, float &posY, float &posZ );
  int  loadSample( const char *filename );
  int  playSample( unsigned int id, float posX = 1.0, float posY = 1.0 , float posZ = 1.0 );
  void stopSample( unsigned int channel );
  void createSampleDummy();
  void setVolume( unsigned int channel, int volume );

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  float mWeight;
  bool  mSound3D;
  bool  mSuccess;
  int   mMusicVolume, mSampleVolume;
  int   mChannel;

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  Sound()
  {}
  ~Sound()
  {}
  Sound( const Sound& ); // disable copy-constructor.
};

#endif
