#ifndef SoundFont_H
#define SoundFont_H

#include <Godot.hpp>
#include <Node.hpp>
#include <File.hpp>
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

	void play_note(godot_int preset_index, godot_int key, float vel);

	void play_midi(String inMidiFileName);

	PoolVector2Array get_buffer(godot_int inSize);
};

}

#endif
