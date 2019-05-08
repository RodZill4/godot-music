extends Node

var sound_font

const GENERATORS = [
	"start_addrs_offset",
	"end_addrs_offset",
	"startloop_addrs_offset",
	"endloop_addrs_offset",
	"start_addrs_coarse_offset",
	"mod_lfo_to_pitch",
	"vib_lfo_to_pitch",
	"mod_env_to_pitch",
	"initial_filter_fc",
	"initial_filter_q",
	"mod_lfo_to_filter_fc",
	"mod_env_to_filter_fc",
	"end_addrs_coarse_offset",
	"mod_lfo_to_volume",
	"unused1",
	"chorus_effects_send",
	"reverb_effects_send",
	"pan",
	"unused2",
	"unused3",
	"unused4",
	"delay_mod_lfo",
	"freq_mod_lfo",
	"delay_vib_lfo",
	"freq_vib_lfo",
	"delay_mod_env",
	"attack_mod_env",
	"hold_mod_env",
	"decay_mod_env",
	"sustain_mod_env",
	"release_mod_env",
	"keynum_to_mod_env_hold",
	"keynum_to_mod_env_decay",
	"delay_vol_env",
	"attack_vol_env",
	"hold_vol_env",
	"decay_vol_env",
	"sustain_vol_env",
	"release_vol_env",
	"keynum_to_vol_env_hold",
	"keynum_to_vol_env_decay",
	"instrument",
	"reserved1",
	"key_range",
	"vel_range",
	"startloop_addrs_coarse_offset",
	"keynum",
	"velocity",
	"initial_attenuation",
	"reserved2",
	"endloop_addrs_coarse_offset",
	"coarse_tune",
	"fine_tune",
	"sample_id",
	"sample_modes",
	"reserved3",
	"scale_tuning",
	"exclusive_class",
	"overriding_root_key",
	"unused5",
	"end_oper"
]

func _ready():
	pass

func load(file_name):
	var file = File.new()
	file.open(file_name, File.READ)
	sound_font = read_chunk(file)
	sound_font.file_name = file_name
	file.close()
	# Process phdr
	for p in sound_font.pdta.phdr:
		var bags = []
		for i in range(p.first_bag_index, p.last_bag_index):
			var bag = sound_font.pdta.pbag[i]
			var contents = {}
			for j in range(bag.first_gen_index, bag.last_gen_index):
				contents[GENERATORS[sound_font.pdta.pgen[j].generator]] = sound_font.pdta.pgen[j].amount
			if bag.first_mod_index < bag.last_mod_index:
				var mods = []
				for j in range(bag.first_mod_index, bag.last_mod_index):
					mods.append(sound_font.pdta.pmod[j])
				contents.mods = mods
			bags.append(contents)
		p.bags = bags
	sound_font.pdta.erase("pbag")
	sound_font.pdta.erase("pgen")
	sound_font.pdta.erase("pmod")
	# Process inst
	for p in sound_font.pdta.inst:
		var bags = []
		for i in range(p.first_bag_index, p.last_bag_index):
			var bag = sound_font.pdta.ibag[i]
			var contents = {}
			for j in range(bag.first_gen_index, bag.last_gen_index):
				contents[GENERATORS[sound_font.pdta.igen[j].generator]] = sound_font.pdta.igen[j].amount
			if bag.first_mod_index < bag.last_mod_index:
				var mods = []
				for j in range(bag.first_mod_index, bag.last_mod_index):
					mods.append(sound_font.pdta.imod[j])
				contents.mods = mods
			bags.append(contents)
		p.bags = bags
	sound_font.pdta.erase("ibag")
	sound_font.pdta.erase("igen")
	sound_font.pdta.erase("imod")
	# Generate JSON file for debug
	if false:
		var outfile = File.new()
		outfile.open(file_name+".json", File.WRITE)
		outfile.store_string(to_json(sound_font))
		outfile.close()

func read_chunk(file):
	var chunk_type = file.get_buffer(4).get_string_from_ascii()
	var chunk_size = file.get_32()
	var begin_position = file.get_position()
	var end_position = begin_position+chunk_size
	var return_value = null
	match chunk_type:
		"RIFF":
			if file.get_buffer(4).get_string_from_ascii() != "sfbk":
				return null
			return_value = {}
			while file.get_position()+8 < end_position:
				var chunk = read_chunk(file)
				if chunk != null:
					return_value[chunk.type] = chunk.value
		"LIST":
			var type = file.get_buffer(4).get_string_from_ascii()
			var value = {}
			print("LIST "+type)
			while file.get_position()+8 < end_position:
				var chunk = read_chunk(file)
				if chunk != null:
					if value.has(chunk.type):
						print("Overwriting "+chunk.type)
					value[chunk.type] = chunk.value
			return_value = { type=type, value=value }
		"isng", "irom", "INAM", "ICRD", "IENG", "IPRD", "ICOP", "ICMT", "ISFT":
			return_value = { type=chunk_type, value=file.get_buffer(chunk_size).get_string_from_ascii() }
		"ifil":
			return_value = { type=chunk_type, value={ major=file.get_16(), minor=file.get_16() } }
		"phdr":
			var phdr = []
			while file.get_position() < end_position:
				var preset = {}
				preset.name = file.get_buffer(20).get_string_from_ascii()
				preset.preset = file.get_16()
				preset.bank = file.get_16()
				preset.first_bag_index = file.get_16()
				preset.library = file.get_32()
				preset.genre = file.get_32()
				preset.morphology = file.get_32()
				if !phdr.empty():
					phdr.back().last_bag_index = preset.first_bag_index
				phdr.append(preset)
			phdr.pop_back()
			return_value = { type="phdr", value=phdr }
		"inst":
			var inst = []
			while file.get_position() < end_position:
				var instrument_name = file.get_buffer(20).get_string_from_ascii()
				var bag_index = file.get_16()
				if !inst.empty():
					inst.back().last_bag_index = bag_index
				inst.append({ name=instrument_name, first_bag_index=bag_index })
			inst.pop_back()
			return_value = { type="inst", value=inst }
		"smpl":
			return_value = { type="smpl", value={ from=begin_position, to=end_position } }
		"pbag", "ibag":
			var bags = []
			while file.get_position() < end_position:
				var gen_index = file.get_16()
				var mod_index = file.get_16()
				if !bags.empty():
					bags.back().last_gen_index = gen_index
					bags.back().last_mod_index = mod_index
				bags.append({ first_gen_index=gen_index, first_mod_index=mod_index })
			bags.pop_back()
			return_value = { type=chunk_type, value=bags }
		"pgen", "igen":
			var gens = []
			while file.get_position() < end_position:
				var generator = file.get_16()
				var amount = file.get_16()
				gens.append({ generator=generator, amount=amount })
			return_value = { type=chunk_type, value=gens }
		"pmod", "imod":
			var mods = []
			while file.get_position() < end_position:
				var item = {}
				item.src_oper = file.get_16()
				item.dest_oper = file.get_16()
				item.amount = file.get_16()
				item.amount_oper = file.get_16()
				item.trans_oper = file.get_16()
				mods.append(item)
			return_value = { type=chunk_type, value=mods }
		"shdr":
			var shdr = []
			while file.get_position() < end_position:
				var item = {}
				item.name = file.get_buffer(20).get_string_from_ascii()
				item.start = file.get_32()
				item.end = file.get_32()
				item.start_loop = file.get_32()
				item.end_loop = file.get_32()
				item.sample_rate = file.get_32()
				item.original_pitch = file.get_8()
				item.pitch_correction = file.get_8()
				item.sample_link = file.get_16()
				item.sample_type = file.get_16()
				shdr.append(item)
			return_value = { type=chunk_type, value=shdr }
		_:
			print("Cannot read chunk of type "+chunk_type)
	file.seek(end_position)
	return return_value

func get_preset_names():
	var return_value = []
	for p in sound_font.pdta.phdr:
		return_value.append(p.name)
	return return_value

func get_instrument_names(preset_id):
	var preset = sound_font.pdta.phdr[preset_id]
	var return_value = []
	for b in preset.bags:
		if b.has("instrument"):
			return_value.append(sound_font.pdta.inst[b.instrument].name)
	return return_value

func get_instrument(preset_id, instrument_index):
	if preset_id >= sound_font.pdta.phdr.size():
		return null
	var preset = sound_font.pdta.phdr[preset_id]
	if instrument_index >= preset.bags.size():
		return null
	if !preset.bags[instrument_index].has("instrument"):
		print(preset.bags[instrument_index])
		return null
	var instrument = preset.bags[instrument_index].instrument
	if instrument >= sound_font.pdta.inst.size():
		return null
	var instrument_data = prepare_instrument(sound_font.pdta.inst[instrument])
	print(instrument_data)
	return instrument_data

func prepare_instrument(data : Dictionary):
	data = data.duplicate(true)
	for b in data.bags:
		b.key_min = 0
		b.key_max = 255
		for k in b.keys():
			match k:
				"key_min", "key_max":
					pass
				"key_range":
					b.key_min = b.key_range & 255
					b.key_max = (b.key_range >> 8) & 255
					print(str(b.key_min)+" - "+str(b.key_max))
				"sample_id":
					var sample = sound_font.pdta.shdr[b[k]]
					b.sample = AudioStreamSample.new()
					var file = File.new()
					file.open(sound_font.file_name, File.READ)
					file.seek(sound_font.sdta.smpl.from+2*sample.start)
					b.sample.data = file.get_buffer(2*(sample.end-sample.start))
					b.sample.format = AudioStreamSample.FORMAT_16_BITS
					#b.sample.mix_rate = mix_rate
					#ass.stereo = false #bag.sample.sample_type != SoundFontLoader.sample_link_mono_sample
					b.sample.loop_begin = sample.start_loop - sample.start
					b.sample.loop_end = sample.end_loop - sample.start
					print("loop "+str(b.sample.loop_begin)+"-"+str(b.sample.loop_end))
					if b.sample.loop_begin < b.sample.loop_end:
						b.sample.loop_mode = AudioStreamSample.LOOP_FORWARD
					if !b.has("pitch"):
						b.pitch = sample.original_pitch
					b.pitch_correction = sample.pitch_correction
					file.close()
				"delay_vol_env", "attack_vol_env", "hold_vol_env", "decay_vol_env", "release_vol_env":
					if b[k] >= 32768:
						b[k] -= 65536
					b[k] = pow(2.0, b[k]/1200.0)
				"sustain_vol_env":
					b[k] = 0.1*b[k]
				"overriding_root_key":
					b.pitch = b.overriding_root_key
				_:
					print(k+" is not supported")
					b.erase(k)
	return data
