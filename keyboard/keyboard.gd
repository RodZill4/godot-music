tool
extends Control
class_name Keyboard

export(int) var keys = 8 setget set_keys
export(int) var offset = 0 setget set_offset

const WHITE_INTERVALS = [ 2, 2, 1, 2, 2, 2, 1 ]

signal note(note, volume)

# Called when the node enters the scene tree for the first time.
func _ready():
	OS.open_midi_inputs()
	print(OS.get_connected_midi_inputs())

func _unhandled_input(event):
	if event is InputEventMIDI:
		if event.message == 9:
			emit_signal("note", event.pitch, event.velocity/127.0)
		elif event.message == 8:
			emit_signal("note", event.pitch, 0)

func set_keys(k):
	keys = k
	update_keyboard()
	
func set_offset(o):
	offset = o
	update_keyboard()

func update_keyboard():
	for c in get_children():
		c.queue_free()
	var x = 0
	var interval = 0
	var key : Key
	var starting_note = 0
	var current_note
	for i in range(offset):
		starting_note += [i % WHITE_INTERVALS.size()]
	current_note = starting_note
	for i in range(keys):
		key = preload("res://keyboard/key_white.tscn").instance()
		key.rect_position = Vector2(x, 0)
		add_child(key)
		key.connect("note", self, "on_note", [ current_note ])
		x += key.rect_size.x
		current_note += WHITE_INTERVALS[(offset + i) % WHITE_INTERVALS.size()]
	rect_min_size = key.rect_position+key.rect_size
	rect_size = rect_min_size
	x = 30
	interval = 0
	current_note = starting_note+1
	for i in range(keys-1):
		var next_interval = WHITE_INTERVALS[(offset + i) % WHITE_INTERVALS.size()]
		if next_interval == 2:
			key = preload("res://keyboard/key_black.tscn").instance()
			key.rect_position = Vector2(x, 0)
			add_child(key)
			key.connect("note", self, "on_note", [ current_note ])
		x += get_child(0).rect_size.x
		current_note += WHITE_INTERVALS[(offset + i) % WHITE_INTERVALS.size()]

func on_note(volume, note):
	emit_signal("note", 48+note, volume)
