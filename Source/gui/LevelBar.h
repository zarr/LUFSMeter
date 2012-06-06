/*
 ===============================================================================
 
 LevelBar.h
 
 
 This file is part of the LUFS Meter audio measurement plugin.
 Copyright 2011-12 by Klangfreund, Samuel Gaehwiler.
 
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


#ifndef __LEVEL_BAR__
#define __LEVEL_BAR__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Macros.h"



//==============================================================================
/**
 A vertical level meter.
 
 Usage: Create a Value object v to which this level bar can "listen" to.
 Every time v is changed it will update itself. You don't have to add the
 level bar directly to the listeners of v (v.addListener(&LevelBar)).
 But you need to ensure that its internal value referes to the same
 value source as your value object v:
 
 -  LevelBar.getLevelValueObject().referTo(v);
*/
class LevelBar  : public Component,
                  public Value::Listener
{
public:
    LevelBar (const Value & levelValueToReferTo);
    
    ~LevelBar ();
    
    Value & getLevelValueObject ();
    
    void valueChanged (Value & value);

    void paint (Graphics& g);
    
private:
    float maximumLevel;
    float minimumLevel;
    float stretch;
    float offset;
    
    Value levelValue;
    
    /** Defines one border of the region which needs to be repainted. */
    float currentLevel;
    /** Defines one border of the region which needs to be repainted. */
    float previousLevel;
};


#endif  // __LEVEL_BAR__
