extends Control

enum VIEW{ALL, GD_OGV, FF_OGV, FF_MP4}

@onready var views: Node2D = $Views
@onready var file_dialog: FileDialog = $FileDialog

@onready var theora_ogv: VideoStreamPlayer = %TheoraOGV
@onready var ffmpeg_ogv: VideoStreamPlayer = %FFmpegOGV
@onready var ffmpeg_mp4: VideoStreamPlayer = %FFmpegMP4

var view_tween: Tween
var view_positions: Array
var view_order: Array
var selected_view: VideoStreamPlayer

func _ready() -> void:
	view_order = views.get_children()
	view_positions.append(theora_ogv.position.y)
	view_positions.append(ffmpeg_ogv.position.y)
	view_positions.append(ffmpeg_mp4.position.y)

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_open"):
		file_dialog.popup()
	elif event.is_action_pressed("ui_cancel") and selected_view:
		select_view(VIEW.ALL)
	elif event.is_action_pressed("ui_reload"):
		get_tree().reload_current_scene()

func _on_file_dialog_file_selected(path: String) -> void:
	theora_ogv.stream = null; ffmpeg_ogv.stream = null; ffmpeg_mp4.stream = null
	
	if FileAccess.file_exists(path.get_basename() + ".ogv"):
		theora_ogv.stream = load(path.get_basename() + ".ogv")
		var stream_ogv: VideoStreamFFmpeg = VideoStreamFFmpeg.new()
		stream_ogv.file = path.get_basename() + ".ogv"
		ffmpeg_ogv.stream = stream_ogv
	
	if path.get_extension() == "ogv":
		var stream_mp4: VideoStreamFFmpeg = VideoStreamFFmpeg.new()
		stream_mp4.file = path
		ffmpeg_mp4.stream = stream_mp4
	else:
		ffmpeg_mp4.stream = load(path)
	
	theora_ogv.play(); ffmpeg_ogv.play(); ffmpeg_mp4.play()

func _on_theora_ogv_gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.is_pressed():
		select_view(VIEW.GD_OGV)

func _on_ffmpeg_ogv_gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.is_pressed():
		select_view(VIEW.FF_OGV)

func _on_ffmpeg_mp4_gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.is_pressed():
		select_view(VIEW.FF_MP4)

func select_view(id: VIEW) -> void:
	var video_stream_player: VideoStreamPlayer
	match id:
		VIEW.GD_OGV: video_stream_player = theora_ogv
		VIEW.FF_OGV: video_stream_player = ffmpeg_ogv
		VIEW.FF_MP4: video_stream_player = ffmpeg_mp4
	
	if selected_view == video_stream_player: return
	if view_tween: view_tween.kill()
	view_tween = create_tween().set_parallel().set_trans(Tween.TRANS_EXPO).set_ease(Tween.EASE_OUT)
	
	var og_size: float = get_viewport().size.x
	var og_pos: float = get_viewport().position.x
	if video_stream_player == null:
		view_tween.tween_property(get_window(), "content_scale_size:x", get_window().content_scale_size.x / 3.0, 0.2)
		view_tween.tween_property(file_dialog, "size:x", file_dialog.size.x / 3.0, 0.2)
		
		if DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_WINDOWED:
			view_tween.tween_property(get_window(), "size:x", 640, 0.2)
			view_tween.tween_property(get_window(), "position:x", og_pos + og_size / 3, 0.2)
		
		view_tween.tween_property(selected_view, "scale", Vector2(0.5, 0.5), 0.2)
		view_tween.tween_property(selected_view, "size", Vector2(1280, 720), 0.2)
		view_tween.tween_property(selected_view, "position:y", view_positions[view_order.find(selected_view)], 0.2)
		
		for child in selected_view.get_children():
			view_tween.tween_property(child, "position", child.position / Vector2(1.5, 1.5), 0.2)
			if child is Label:
				view_tween.tween_property(child, "size:x", child.size.x / 1.5, 0.2)
				view_tween.tween_property(child, "modulate:a", 1, 0.2)
		for view in views.get_children():
			if view == selected_view: continue
			var tween: Tween = create_tween()
			tween.tween_property(view, "modulate:a", 1, 0.1)
			tween.tween_property(view, "volume", 1, 0.1)
	else:
		view_tween.tween_property(get_window(), "content_scale_size:x", get_window().content_scale_size.x * 3, 0.2)
		view_tween.tween_property(file_dialog, "size:x", file_dialog.size.x * 3, 0.2)
		
		if DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_WINDOWED:
			view_tween.tween_property(get_window(), "size:x", 1920, 0.2)
			view_tween.tween_property(get_window(), "position:x", og_pos - og_size, 0.2)
		
		video_stream_player.move_to_front()
		view_tween.tween_property(video_stream_player, "scale", Vector2(1, 1), 0.2)
		view_tween.tween_property(video_stream_player, "size", Vector2(1920, 1080), 0.2)
		view_tween.tween_property(video_stream_player, "position:y", 0, 0.2)
		
		for child in video_stream_player.get_children():
			view_tween.tween_property(child, "position", child.position * Vector2(1.5, 1.5), 0.2)
			if child is Label:
				view_tween.tween_property(child, "size:x", child.size.x * 1.5, 0.2)
				view_tween.tween_property(child, "modulate:a", 0, 0.2)
		for view in views.get_children():
			if view == video_stream_player: continue
			var tween: Tween = create_tween()
			tween.tween_property(view, "modulate:a", 0, 0.1)
			tween.tween_property(view, "volume", 0, 0.1)
	
	selected_view = video_stream_player
