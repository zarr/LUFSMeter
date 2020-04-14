/*
 ===============================================================================
 
 BackgroundGrid.h
  
 
 This file is part of the LUFS Meter audio measurement plugin.
 Copyright 2011-14 by Klangfreund, Samuel Gaehwiler.
 
 -------------------------------------------------------------------------------
 
 The LUFS Meter can be redistributed and/or modified under the terms of the GNU 
 General Public License Version 2, as published by the Free Software Foundation.
 A copy of the license is included with these source files. It can also be found
 at www.gnu.org/licenses.
 
 The LUFS Meter is distributed WITHOUT ANY WARRANTY.
 See the GNU General Public License for more details.
 
 -------------------------------------------------------------------------------
 
 To release a closed-source product which uses the LUFS Meter or parts of it,
 a commercial license is available. Visit www.klangfreund.com/lufsmeter for more
 information.
 
 ===============================================================================
 */

#ifndef __BACKGROUND_GRID__
#define __BACKGROUND_GRID__

#include "../MacrosAndJuceHeaders.h"



//==============================================================================
/**
*/
class BackgroundGrid  : public Component,
                        public Value::Listener
{
public:
    BackgroundGrid (const Value & minLoudnessToReferTo,
                    const Value & maxLoudnessToReferTo);
    
    ~BackgroundGrid ();

    void paint (Graphics& g) override;
    
private:
    void valueChanged (Value & value) override;

    Value minLoudness;
    Value maxLoudness;
};


#endif  // __BACKGROUND_GRID__
