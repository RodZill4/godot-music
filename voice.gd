extends AudioStreamPlayer
class_name Voice

var tween : Tween

var current_note
var current_volume
var current_sample
var key_released

signal stopped

func _ready():
	tween = Tween.new()
	add_child(tween)
	tween.connect("tween_completed", self, "on_tween_completed")

func play_note(note, volume, sample):
	print("play")
	current_note = note
	current_volume = volume
	current_sample = sample
	key_released = false
	stream = sample.sample
	pitch_scale = pow(2.0, (note-sample.pitch)/12.0)
	var peak_volume_db = linear2db(volume)
	var delay = sample.delay_vol_env if sample.has("delay_vol_env") else 0.0
	if delay < 0.1: delay = 0.0
	var attack = sample.attack_vol_env if sample.has("attack_vol_env") else 0.0
	var hold = sample.hold_vol_env if sample.has("hold_vol_env") else 0.0
	var decay = sample.decay_vol_env if sample.has("decay_vol_env") else 0.0
	var sustain = sample.sustain_vol_env if sample.has("sustain_vol_env") else 0.0
	tween.remove_all()
	if delay > 0.0:
		tween.interpolate_property(self, "volume_db", linear2db(0.0), linear2db(0.0), delay, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT)
	if attack > 0.0:
		tween.interpolate_property(self, "volume_db", linear2db(0.0), peak_volume_db, attack, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT, delay)
	if hold > 0.0:
		tween.interpolate_property(self, "volume_db", peak_volume_db, peak_volume_db, hold, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT, delay+attack)
	if decay > 0.0:
		tween.interpolate_property(self, "volume_db", peak_volume_db, peak_volume_db-sustain, decay, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT, delay+attack+hold)
	if delay+attack == 0.0:
		volume_db = peak_volume_db
	.play()
	tween.start()

func stop_note():
	tween.stop_all()
	var release = current_sample.release_vol_env if current_sample.has("release_vol_env") else 0.0
	if release > 0.0:
		key_released = true
		tween.remove_all()
		tween.interpolate_property(self, "volume_db", volume_db, linear2db(0.0), release, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT)
		tween.start()
	else:
		.stop()
		emit_signal("stopped", self)

func on_tween_completed(object, key):
	if key_released:
		.stop()
		emit_signal("stopped", self)
