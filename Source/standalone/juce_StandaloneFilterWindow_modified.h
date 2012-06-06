/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__
#define __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__

#include "../../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/** Somewhere in the codebase of your plugin, you need to implement this function
    and make it create an instance of the filter subclass that you're building.
*/
extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();


//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your JUCEApplication::initialise() method, and
    let it do its work. It will create your filter object using the same createFilter() function
    that the other plugin wrappers use.
 
    MODIFICATION NOTE (by sam):
    Only changed one line in the constructor, such that no audio output is generated.
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                  public ButtonListener   // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    //==============================================================================
    /** Creates a window with a given title and colour.
        The settings object can be a PropertySet that the class should use to
        store its settings - the object that is passed-in will be owned by this
        class and deleted automatically when no longer needed. (It can also be null)
    */
    StandaloneFilterWindow (const String& title,
                            const Colour& backgroundColour,
                            PropertySet* settingsToUse)
        : DocumentWindow (title, backgroundColour, DocumentWindow::minimiseButton | DocumentWindow::closeButton),
          settings (settingsToUse),
          optionsButton ("options")
    {
        setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, false);

        Component::addAndMakeVisible (&optionsButton);
        optionsButton.addListener (this);
        optionsButton.setTriggeredOnMouseDown (true);

        JUCE_TRY
        {
            filter = createPluginFilter();
        }
        JUCE_CATCH_ALL

        if (filter == nullptr)
        {
            jassertfalse    // Your filter didn't create correctly! In a standalone app that's not too great.
            JUCEApplication::quit();
        }

        filter->setPlayConfigDetails (JucePlugin_MaxNumInputChannels,
                                      JucePlugin_MaxNumOutputChannels,
                                      44100, 512);

        deviceManager = new AudioDeviceManager();
        deviceManager->addAudioCallback (&player);
        deviceManager->addMidiInputCallback (String::empty, &player);

        player.setProcessor (filter);

        ScopedPointer<XmlElement> savedState;

        if (settings != nullptr)
            savedState = settings->getXmlValue ("audioSetup");

//        deviceManager->initialise (filter->getNumInputChannels(),
//                                   filter->getNumOutputChannels(),
//                                   savedState,
//                                   true);
        // by sam: Initialise with 0 output channels.
        deviceManager->initialise (filter->getNumInputChannels(),
                                   0,
                                   savedState,
                                   true);

        if (settings != nullptr)
        {
            MemoryBlock data;

            if (data.fromBase64Encoding (settings->getValue ("filterState"))
                 && data.getSize() > 0)
            {
                filter->setStateInformation (data.getData(), data.getSize());
            }
        }

        setContentOwned (filter->createEditorIfNeeded(), true);

        if (settings != nullptr)
        {
            const int x = settings->getIntValue ("windowX", -100);
            const int y = settings->getIntValue ("windowY", -100);

            if (x != -100 && y != -100)
                setBoundsConstrained (Rectangle<int> (x, y, getWidth(), getHeight()));
            else
                centreWithSize (getWidth(), getHeight());
        }
        else
        {
            centreWithSize (getWidth(), getHeight());
        }
    }

    ~StandaloneFilterWindow()
    {
        if (settings != nullptr)
        {
            settings->setValue ("windowX", getX());
            settings->setValue ("windowY", getY());

            if (deviceManager != nullptr)
            {
                ScopedPointer<XmlElement> xml (deviceManager->createStateXml());
                settings->setValue ("audioSetup", xml);
            }
        }

        deviceManager->removeMidiInputCallback (String::empty, &player);
        deviceManager->removeAudioCallback (&player);
        deviceManager = nullptr;

        if (settings != nullptr && filter != nullptr)
        {
            MemoryBlock data;
            filter->getStateInformation (data);

            settings->setValue ("filterState", data.toBase64Encoding());
        }

        deleteFilter();
    }

    //==============================================================================
    /** Deletes and re-creates the filter and its UI. */
    void resetFilter()
    {
        deleteFilter();

        filter = createPluginFilter();

        if (filter != nullptr)
        {
            if (deviceManager != nullptr)
                player.setProcessor (filter);

            setContentOwned (filter->createEditorIfNeeded(), true);
        }

        if (settings != nullptr)
            settings->removeValue ("filterState");
    }

    /** Pops up a dialog letting the user save the filter's state to a file. */
    void saveState()
    {
        FileChooser fc (TRANS("Save current state"),
                        settings != nullptr ? File (settings->getValue ("lastStateFile"))
                                            : File::nonexistent);

        if (fc.browseForFileToSave (true))
        {
            MemoryBlock data;
            filter->getStateInformation (data);

            if (! fc.getResult().replaceWithData (data.getData(), data.getSize()))
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                             TRANS("Error whilst saving"),
                                             TRANS("Couldn't write to the specified file!"));
            }
        }
    }

    /** Pops up a dialog letting the user re-load the filter's state from a file. */
    void loadState()
    {
        FileChooser fc (TRANS("Load a saved state"),
                        settings != nullptr ? File (settings->getValue ("lastStateFile"))
                                            : File::nonexistent);

        if (fc.browseForFileToOpen())
        {
            MemoryBlock data;

            if (fc.getResult().loadFileAsData (data))
            {
                filter->setStateInformation (data.getData(), data.getSize());
            }
            else
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                             TRANS("Error whilst loading"),
                                             TRANS("Couldn't read from the specified file!"));
            }
        }
    }

    /** Shows the audio properties dialog box modally. */
    virtual void showAudioSettingsDialog()
    {
        AudioDeviceSelectorComponent selectorComp (*deviceManager,
                                                   filter->getNumInputChannels(),
                                                   filter->getNumInputChannels(),
                                                   filter->getNumOutputChannels(),
                                                   filter->getNumOutputChannels(),
                                                   true, false, true, false);

        selectorComp.setSize (500, 450);

        DialogWindow::showModalDialog (TRANS("Audio Settings"), &selectorComp, this,
                                       Colours::lightgrey, true, false, false);
    }

    //==============================================================================
    /** @internal */
    void closeButtonPressed()
    {
        JUCEApplication::quit();
    }

    /** @internal */
    void buttonClicked (Button*)
    {
        if (filter != nullptr)
        {
            PopupMenu m;
            m.addItem (1, TRANS("Audio Settings..."));
            m.addSeparator();
            m.addItem (2, TRANS("Save current state..."));
            m.addItem (3, TRANS("Load a saved state..."));
            m.addSeparator();
            m.addItem (4, TRANS("Reset to default state"));

            switch (m.showAt (&optionsButton))
            {
                case 1:  showAudioSettingsDialog(); break;
                case 2:  saveState(); break;
                case 3:  loadState(); break;
                case 4:  resetFilter(); break;
                default: break;
            }
        }
    }

    /** @internal */
    void resized()
    {
        DocumentWindow::resized();
        optionsButton.setBounds (8, 6, 60, getTitleBarHeight() - 8);
    }

private:
    //==============================================================================
    ScopedPointer<PropertySet> settings;
    ScopedPointer<AudioProcessor> filter;
    ScopedPointer<AudioDeviceManager> deviceManager;
    AudioProcessorPlayer player;
    TextButton optionsButton;

    void deleteFilter()
    {
        player.setProcessor (nullptr);

        if (filter != nullptr && getContentComponent() != nullptr)
        {
            filter->editorBeingDeleted (dynamic_cast <AudioProcessorEditor*> (getContentComponent()));
            clearContentComponent();
        }

        filter = nullptr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandaloneFilterWindow);
};

#endif   // __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__
