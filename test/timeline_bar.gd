extends Panel

@onready var time_left: Label = $VBoxContainer/HBoxContainer/TimeLeft
@onready var time_length: Label = $VBoxContainer/HBoxContainer/TimeLength
@onready var h_scroll_bar: HScrollBar = $VBoxContainer/HBoxContainer/HScrollBar

@onready var play_button: Button = $Play
@onready var volume_visual: ProgressBar = $Volume/Visual

@onready var hover_pos: float = position.y

var hover_tween: Tween
var timeline_up: bool = true
var timeline_delay: float

func _process(delta: float) -> void:
	h_scroll_bar.set_value_no_signal(%FFmpegMP4.stream_position)
	if %FFmpegMP4.stream:
		time_left.text = Time.get_time_string_from_unix_time(%FFmpegMP4.stream_position)
	else:
		time_left.text = "00:00:00"
	
	if %TheoraOGV.is_playing() or %FFmpegOGV.is_playing() or %FFmpegMP4.is_playing() and not (%TheoraOGV.paused or %FFmpegOGV.paused or %FFmpegMP4.paused):
		if not timeline_up and get_window().get_mouse_position().y >= hover_pos and get_window().get_mouse_position().x < get_window().size.x and get_window().has_focus():
			pull_up_timeline()
		elif timeline_up and (get_window().get_mouse_position().y < hover_pos or get_window().get_mouse_position().x >= get_window().size.x) and get_window().has_focus():
			timeline_delay += delta
			if timeline_delay >= 3:
				timeline_up = false
				_on_timeline_bar_trigger_mouse_exited()
	else:
		pull_up_timeline()

func pull_up_timeline():
	timeline_up = true
	timeline_delay = 0
	_on_timeline_bar_trigger_mouse_entered()

func _on_file_dialog_file_selected(_path: String) -> void:
	await get_tree().process_frame
	time_length.text = Time.get_time_string_from_unix_time(%FFmpegMP4.get_stream_length())
	h_scroll_bar.max_value = %FFmpegMP4.get_stream_length()
	play_button.icon = preload("res://textures/Pause.svg")
	%TheoraOGV.paused = false; %FFmpegOGV.paused = false; %FFmpegMP4.paused = false

func _on_h_scroll_bar_value_changed(value: float) -> void:
	%TheoraOGV.stream_position = value
	%FFmpegOGV.stream_position = value
	%FFmpegMP4.stream_position = value

func _on_play_button_up() -> void:
	if %TheoraOGV.is_playing() or %FFmpegOGV.is_playing() or %FFmpegMP4.is_playing():
		%TheoraOGV.paused = !%TheoraOGV.paused
		%FFmpegOGV.paused = !%FFmpegOGV.paused
		%FFmpegMP4.paused = !%FFmpegMP4.paused

		if %TheoraOGV.paused:
			play_button.icon = preload("res://textures/Play.svg")
		else:
			play_button.icon = preload("res://textures/Pause.svg")
	else:
		%TheoraOGV.play()
		%FFmpegOGV.play()
		%FFmpegMP4.play()

func _on_stop_button_up() -> void:
	play_button.icon = preload("res://textures/Play.svg")
	%TheoraOGV.stop(); %TheoraOGV.paused = false
	%FFmpegOGV.stop(); %FFmpegOGV.paused = false
	%FFmpegMP4.stop(); %FFmpegMP4.paused = false

func _on_loop_toggled(toggled_on: bool) -> void:
	%TheoraOGV.loop = toggled_on
	%FFmpegOGV.loop = toggled_on
	%FFmpegMP4.loop = toggled_on

func _on_fullscreen_toggled(toggled_on: bool) -> void:
	if toggled_on:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_EXCLUSIVE_FULLSCREEN)
	else:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)

func _on_volume_value_changed(value: float) -> void:
	AudioServer.set_bus_volume_linear(0, value)
	volume_visual.value = value

func _on_timeline_bar_trigger_mouse_entered() -> void:
	if hover_tween: hover_tween.kill()
	hover_tween = create_tween()
	hover_tween.tween_property(self, "position:y", hover_pos, 0.5).set_trans(Tween.TRANS_EXPO).set_ease(Tween.EASE_OUT)

func _on_timeline_bar_trigger_mouse_exited() -> void:
	if hover_tween: hover_tween.kill()
	hover_tween = create_tween()
	hover_tween.tween_property(self, "position:y", hover_pos + size.y, 0.5).set_trans(Tween.TRANS_EXPO).set_ease(Tween.EASE_OUT)
