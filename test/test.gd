extends Node2D

@onready var file_dialog: FileDialog = $FileDialog

@onready var theora_ogv: VideoStreamPlayer = $TheoraOGV
@onready var ffmpeg_ogv: VideoStreamPlayer = $FFmpegOGV
@onready var ffmpeg_mp4: VideoStreamPlayer = $FFmpegMP4

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_open"):
		file_dialog.popup()
	
	if event.is_action_pressed("ui_accept"):
		if theora_ogv.is_playing():
			theora_ogv.stop()
			ffmpeg_ogv.stop()
			ffmpeg_mp4.stop()
		else:
			theora_ogv.play()
			ffmpeg_ogv.play()
			ffmpeg_mp4.play()

func _on_file_dialog_file_selected(path: String) -> void:
	theora_ogv.stream = load(path.get_basename() + ".ogv")
	ffmpeg_ogv.stream = load(path.get_basename() + ".ogv") as VideoStreamFFmpeg
	ffmpeg_mp4.stream = load(path)
