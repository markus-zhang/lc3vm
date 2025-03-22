#include <fluidsynth.h>
#include <unistd.h>
#include <string>

class MidiPlayerFrontend
{
public:
    MidiPlayerFrontend();
    ~MidiPlayerFrontend();

    void Init(std::string sfPath);

public:
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    fluid_audio_driver_t* audioDriver;

    int soundFontId;
};

MidiPlayerFrontend::MidiPlayerFrontend()
{
    settings = nullptr;
    synth = nullptr;
    audioDriver = nullptr;

    soundFontId = 0;
}

MidiPlayerFrontend::~MidiPlayerFrontend()
{
    delete_fluid_audio_driver(audioDriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void MidiPlayerFrontend::Init(std::string sfPath)
{
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
    audioDriver = new_fluid_audio_driver(settings, synth);

    soundFontId = fluid_synth_sfload(synth, sfPath.c_str(), 1);
}

int main()
{
    // Initialize Fluidsynth
    fluid_settings_t* settings = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(settings);
    fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);

    // Load a SoundFont (replace with your SF2 file)
    int sf_id = fluid_synth_sfload(synth, "/usr/share/sounds/sf2/default-GM.sf2", 1);

    // Set Instrument to Piano (Program Change 0) on Channel 1
    fluid_synth_program_change(synth, 0, 0);

    // Set Volume (CC 7) to 100
    fluid_synth_cc(synth, 0, 7, 100);

    // Enable Modulation (CC 1)
    fluid_synth_cc(synth, 0, 1, 127);

    // Play a note (Note E5, velocity 100)
    fluid_synth_noteon(synth, 0, 76, 100);
    sleep(1); // Hold for 1 second

    // Apply Pitch Bend (higher pitch)
    fluid_synth_pitch_bend(synth, 0, 10000);
    sleep(1);

    // Release the note
    fluid_synth_noteoff(synth, 0, 76);

    // Cleanup
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return 0;
}