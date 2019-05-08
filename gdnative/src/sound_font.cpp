#include "sound_font.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"

using namespace godot;

void SoundFont::_register_methods() {
	register_property<SoundFont, String>("sound_font", &SoundFont::set_sound_font, &SoundFont::get_sound_font, "");
	register_property<SoundFont, float>("midi_speed", &SoundFont::mMidiSpeed, 1.0);

	register_method("get_preset_names", &SoundFont::get_preset_names);

	register_method("note_on", &SoundFont::note_on);
	register_method("note_off", &SoundFont::note_off);
	register_method("note_off_all", &SoundFont::note_off_all);

	register_method("channel_set_presetindex", &SoundFont::channel_set_presetindex);
	register_method("channel_set_presetnumber", &SoundFont::channel_set_presetnumber);
	register_method("channel_set_bank", &SoundFont::channel_set_bank);
	register_method("channel_set_bank_preset", &SoundFont::channel_set_bank_preset);
	register_method("channel_set_pan", &SoundFont::channel_set_pan);
	register_method("channel_set_volume", &SoundFont::channel_set_volume);
	register_method("channel_set_pitchwheel", &SoundFont::channel_set_pitchwheel);
	register_method("channel_set_pitchrange", &SoundFont::channel_set_pitchrange);
	register_method("channel_set_tuning", &SoundFont::channel_set_tuning);
	register_method("channel_note_on", &SoundFont::channel_note_on);
	register_method("channel_note_off", &SoundFont::channel_note_off);
	register_method("channel_note_off_all", &SoundFont::channel_note_off_all);
	register_method("channel_midi_control", &SoundFont::channel_midi_control);
	register_method("channel_get_preset_index", &SoundFont::channel_get_preset_index);
	register_method("channel_get_preset_bank", &SoundFont::channel_get_preset_bank);
	register_method("channel_get_preset_number", &SoundFont::channel_get_preset_number);
	register_method("channel_get_pan", &SoundFont::channel_get_pan);
	register_method("channel_get_volume", &SoundFont::channel_get_volume);
	register_method("channel_get_pitchwheel", &SoundFont::channel_get_pitchwheel);
	register_method("channel_get_pitchrange", &SoundFont::channel_get_pitchrange);
	register_method("channel_get_tuning", &SoundFont::channel_get_tuning);
 
 	register_method("play_midi", &SoundFont::play_midi);

	register_method("get_buffer", &SoundFont::get_buffer);

	//register_signal<SoundFont>((char *)"position_changed", "node", GODOT_VARIANT_TYPE_OBJECT, "new_pos", GODOT_VARIANT_TYPE_VECTOR2);
}

SoundFont::SoundFont()
	: mTsf(NULL)
	, mTsfFile(NULL)
	, mMidiSpeed(1.0) {
}

SoundFont::~SoundFont() {
	if (mTsf != NULL) {
		tsf_close(mTsf);
	}
	delete mTsfFile;
	if (mTsfFile->open(mSoundFontName, File::READ) == Error::OK) {
		mTsf = tsf_load(&mTsfStream);
	}
	delete mTmlFile;
}

static int readTsfFile(void* inData, void* inPtr, unsigned int inSize) {
	File                *theFile = (File*)inData;
	const PoolByteArray &theData = theFile->get_buffer(inSize);
	godot_int            theReadSize = theData.size();
	memcpy(inPtr, theData.read().ptr(), theReadSize);
	
	return theReadSize;
}
static int skipTsfFile(void* inData, unsigned int inSize) {
	File *theFile = (File*)inData;
	int64_t theNewPosition = theFile->get_position()+inSize;
	theFile->seek(theNewPosition);
	return (theFile->get_position() == theNewPosition) ? 1 : 0;
}

void SoundFont::_init() {
	mTsf = NULL;
	mTsfFile = File::_new();
	mSoundFontName = "";
	mTsfStream.data = mTsfFile;
	mTsfStream.read = readTsfFile;
	mTsfStream.skip = skipTsfFile;
	mTml = NULL;
	mTmlCurrent = NULL;
	mTmlFile = File::_new();
	mTmlStream.data = mTmlFile;
	mTmlStream.read = readTsfFile;
}

void SoundFont::set_sound_font(String inSoundFontName) {
	mSoundFontName = inSoundFontName;
	if (mTsf != NULL) {
		tsf_close(mTsf);
		mTsf = NULL;
		mTsfFile->close();
	}
	if (mTsfFile->open(mSoundFontName, File::READ) == Error::OK) {
		mTsf = tsf_load(&mTsfStream);
	}
}

String SoundFont::get_sound_font() const {
	return mSoundFontName;
}

PoolStringArray SoundFont::get_preset_names() const {
	PoolStringArray theReturnValue;

	if (mTsf != NULL) {
		unsigned theIndex, theEnd;

		for (theIndex = 0, theEnd = tsf_get_presetcount(mTsf); theIndex < theEnd; ++theIndex) {
			theReturnValue.append(tsf_get_presetname(mTsf, theIndex));
		}
	}
	return theReturnValue;
}

godot_int SoundFont::get_presetindex(godot_int inBank, godot_int inPresetNumber) {
	if (mTsf == NULL) {
		return -1;
	}
	return tsf_get_presetindex(mTsf, inBank, inPresetNumber);
}

void SoundFont::note_on(godot_int inPresetIndex, godot_int inKey, float inVelocity) {
	if (mTsf == NULL) {
		return;
	}
	if (inVelocity > 0.0) {
		tsf_note_on(mTsf, inPresetIndex, inKey, inVelocity);
	} else {
		tsf_note_off(mTsf, inPresetIndex, inKey);
	}
}

void SoundFont::note_off(godot_int inPresetIndex, godot_int inKey) {
	tsf_note_on(mTsf, inPresetIndex, inKey, 0.0);
}

void SoundFont::note_off_all() {
	if (mTsf == NULL) {
		return;
	}
	tsf_note_off_all(mTsf);
}

void SoundFont::channel_set_presetindex(godot_int inChannel, godot_int inPresetIndex) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_set_presetindex(mTsf, inChannel, inPresetIndex);
}


godot_int SoundFont::channel_set_presetnumber(godot_int inChannel, godot_int inPresetNumber, godot_int inDrums) {
	if (mTsf == NULL) {
		return -1;
	}
	return tsf_channel_set_presetnumber(mTsf, inChannel, inPresetNumber, inDrums);
}


void SoundFont::channel_set_bank(godot_int inChannel, godot_int inBank) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_set_bank(mTsf, inChannel, inBank);
}


godot_int SoundFont::channel_set_bank_preset(godot_int inChannel, godot_int inBank, godot_int inPresetNumber) {
	if (mTsf == NULL) {
		return -1;
	}
	return tsf_channel_set_bank_preset(mTsf, inChannel, inBank, inPresetNumber);
}


void SoundFont::channel_set_pan(godot_int inChannel, float inPan) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_set_pan(mTsf, inChannel, inPan);
}


void SoundFont::channel_set_volume(godot_int inChannel, float inVolume) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_set_volume(mTsf, inChannel, inVolume);
}


void SoundFont::channel_set_pitchwheel(godot_int inChannel, godot_int inPitchWheel) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_set_pitchwheel(mTsf, inChannel, inPitchWheel);
}


void SoundFont::channel_set_pitchrange(godot_int inChannel, float inPitchRange) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_set_pitchrange(mTsf, inChannel, inPitchRange);
}


void SoundFont::channel_set_tuning(godot_int inChannel, float inTuning) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_set_tuning(mTsf, inChannel, inTuning);
}


void SoundFont::channel_note_on(godot_int inChannel, godot_int inKey, float inVelocity) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_note_on(mTsf, inChannel, inKey, inVelocity);
}


void SoundFont::channel_note_off(godot_int inChannel, godot_int inKey) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_note_off(mTsf, inChannel, inKey);
}


void SoundFont::channel_note_off_all(godot_int inChannel) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_note_off_all(mTsf, inChannel);
}

 //end with sustain and release
void SoundFont::channel_sounds_off_all(godot_int inChannel) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_sounds_off_all(mTsf, inChannel);
}

 //end immediatly
void SoundFont::channel_midi_control(godot_int inChannel, godot_int inController, godot_int inValue) {
	if (mTsf == NULL) {
		return;
	}
	tsf_channel_midi_control(mTsf, inChannel, inController, inValue);
}


godot_int SoundFont::channel_get_preset_index(godot_int inChannel) {
	if (mTsf == NULL) {
		return -1;
	}
	return tsf_channel_get_preset_index(mTsf, inChannel);
}


godot_int SoundFont::channel_get_preset_bank(godot_int inChannel) {
	if (mTsf == NULL) {
		return -1;
	}
	return tsf_channel_get_preset_bank(mTsf, inChannel);
}


godot_int SoundFont::channel_get_preset_number(godot_int inChannel) {
	if (mTsf == NULL) {
		return -1;
	}
	return tsf_channel_get_preset_number(mTsf, inChannel);
}


float SoundFont::channel_get_pan(godot_int inChannel) {
	if (mTsf == NULL) {
		return 0.0;
	}
	return tsf_channel_get_pan(mTsf, inChannel);
}


float SoundFont::channel_get_volume(godot_int inChannel) {
	if (mTsf == NULL) {
		return 0.0;
	}
	return tsf_channel_get_volume(mTsf, inChannel);
}


godot_int SoundFont::channel_get_pitchwheel(godot_int inChannel) {
	if (mTsf == NULL) {
		return 0;
	}
	return tsf_channel_get_pitchwheel(mTsf, inChannel);
}

float SoundFont::channel_get_pitchrange(godot_int inChannel) {
	if (mTsf == NULL) {
		return 0.0;
	}
	return tsf_channel_get_pitchrange(mTsf, inChannel);
}

float SoundFont::channel_get_tuning(godot_int inChannel) {
	if (mTsf == NULL) {
		return 0.0;
	}
	return tsf_channel_get_tuning(mTsf, inChannel);
}

void SoundFont::play_midi(String inMidiFileName) {
	if (mTml != NULL) {
		tml_free(mTml);
		mTml = NULL;
		mTmlCurrent = NULL;
		mTmlFile->close();
	}
	if (mTmlFile->open(inMidiFileName, File::READ) == Error::OK) {
		mTml = tml_load(&mTmlStream);
		mTmlCurrent = mTml;
		mTmlTime = 0.0;
	}
}

PoolVector2Array SoundFont::get_buffer(godot_int inSize) {
	PoolVector2Array theBuffer;
	
	if (inSize > 0 && mTsf != NULL) {
		theBuffer.resize(inSize);
		Vector2 *theData = theBuffer.write().ptr();
		if (mTmlCurrent != NULL) {
			int SampleBlock;
			int SampleCount = inSize;
			for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, theData += SampleBlock) {
				//We progress the MIDI playback and then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
				if (SampleBlock > SampleCount) SampleBlock = SampleCount;

				//Loop through all MIDI messages which need to be played up until the current playback time
				for (mTmlTime += SampleBlock * mMidiSpeed * (1000.0 / 44100.0); mTmlCurrent && mTmlTime >= mTmlCurrent->time; mTmlCurrent = mTmlCurrent->next) {
					switch (mTmlCurrent->type) {
						case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
							tsf_channel_set_presetnumber(mTsf, mTmlCurrent->channel, mTmlCurrent->program, (mTmlCurrent->channel == 9));
							break;
						case TML_NOTE_ON: //play a note
							tsf_channel_note_on(mTsf, mTmlCurrent->channel, mTmlCurrent->key, mTmlCurrent->velocity / 127.0f);
							break;
						case TML_NOTE_OFF: //stop a note
							tsf_channel_note_off(mTsf, mTmlCurrent->channel, mTmlCurrent->key);
							break;
						case TML_PITCH_BEND: //pitch wheel modification
							tsf_channel_set_pitchwheel(mTsf, mTmlCurrent->channel, mTmlCurrent->pitch_bend);
							break;
						case TML_CONTROL_CHANGE: //MIDI controller messages
							tsf_channel_midi_control(mTsf, mTmlCurrent->channel, mTmlCurrent->control, mTmlCurrent->control_value);
							break;
					}
				}
				// Render the block of audio samples in float format
				tsf_render_float(mTsf, (float*)theData, SampleBlock, 0);
			}
		} else {
			tsf_render_float(mTsf, (float*)theData, inSize, 0);
		}
	}
	return theBuffer;
}

void set_playback(AudioStreamPlayback *inAudioStreamPlayback) {

}
