#ifndef SoundFont_H
#define SoundFont_H

#include <Godot.hpp>
#include <Node.hpp>
#include <File.hpp>
#include <AudioStreamPlayback.hpp>
#include "tsf.h"
#include "tml.h"

namespace godot {

class SoundFont : public Node {
	GODOT_CLASS(SoundFont, Node)

private:
	String       mSoundFontName;
	tsf         *mTsf;
	File        *mTsfFile;
	tsf_stream   mTsfStream;
	tml_message *mTml;
	tml_message *mTmlCurrent;
	double       mTmlTime;
	float        mMidiSpeed;
	File        *mTmlFile;
	tml_stream   mTmlStream;

public:
	static void _register_methods();

	SoundFont();
	~SoundFont();

	void _init(); // our initializer called by Godot

	void set_sound_font(String inSoundFontName);
	String get_sound_font() const;

	PoolStringArray get_preset_names() const;
	godot_int get_presetindex(godot_int inBank, godot_int inPresetNumber);

	void set_output(godot_int inSampleRate, float inGainDb);
	
	void note_on(godot_int inPresetIndex, godot_int inKey, float inVelocity);
	void note_off(godot_int inPresetIndex, godot_int inKey);
	void note_off_all();

    void channel_set_presetindex(godot_int inChannel, godot_int inPresetIndex);
    godot_int channel_set_presetnumber(godot_int inChannel, godot_int inPresetNumber, godot_int inDrums);
    void channel_set_bank(godot_int inChannel, godot_int inBank);
    godot_int channel_set_bank_preset(godot_int inChannel, godot_int inBank, godot_int inPresetNumber);
    void channel_set_pan(godot_int inChannel, float inPan);
    void channel_set_volume(godot_int inChannel, float inVolume);
    void channel_set_pitchwheel(godot_int inChannel, godot_int inPitchWheel);
    void channel_set_pitchrange(godot_int inChannel, float inPitchRange);
    void channel_set_tuning(godot_int inChannel, float inTuning);
    void channel_note_on(godot_int inChannel, godot_int inKey, float inVelocity);
    void channel_note_off(godot_int inChannel, godot_int inKey);
    void channel_note_off_all(godot_int inChannel); //end with sustain and release
    void channel_sounds_off_all(godot_int inChannel); //end immediatly
    void channel_midi_control(godot_int inChannel, godot_int inController, godot_int inValue);
    godot_int channel_get_preset_index(godot_int inChannel);
    godot_int channel_get_preset_bank(godot_int inChannel);
    godot_int channel_get_preset_number(godot_int inChannel);
    float channel_get_pan(godot_int inChannel);
    float channel_get_volume(godot_int inChannel);
    godot_int channel_get_pitchwheel(godot_int inChannel);
    float channel_get_pitchrange(godot_int inChannel);
    float channel_get_tuning(godot_int inChannel);

	void play_midi(String inMidiFileName);

	PoolVector2Array get_buffer(godot_int inSize);
};

}

#endif
